/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_penop.h"

#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include <KoColor.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_brush_option.h>

#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>

#include <kis_penop_settings.h>

KisPenOp::KisPenOp(const KisPenOpSettings *settings, KisPainter *painter)
        : KisBrushBasedPaintOp(painter)
        , settings(settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    if (settings && settings->m_options) {
        Q_ASSERT(settings->m_options->m_brushOption);
        m_brush = settings->m_options->m_brushOption->brush();
        Q_ASSERT(m_brush);
        settings->m_options->m_sizeOption->sensor()->reset();
        settings->m_options->m_opacityOption->sensor()->reset();
        settings->m_options->m_darkenOption->sensor()->reset();
    }
}

KisPenOp::~KisPenOp()
{
}

void KisPenOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()->device()) return;

    KisBrushSP brush = m_brush;
    if (!m_brush) {
        if (settings->m_options) {
            m_brush = settings->m_options->m_brushOption->brush();
            brush = m_brush;
        } else {
            return;
        }
    }


    if (! brush->canPaintFor(info))
        return;

    double scale = KisPaintOp::scaleForPressure(settings->m_options->m_sizeOption->apply(info));
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return;

    KisPaintDeviceSP device = painter()->device();
    QPointF hotSpot = brush->hotSpot(scale, scale);
    QPointF pt = info.pos() - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x = qRound(pt.x());
    qint32 y = qRound(pt.y());

    quint8 origOpacity = settings->m_options->m_opacityOption->apply(painter(), info);
    KoColor origColor = settings->m_options->m_darkenOption->apply(painter(), info);

    KisFixedPaintDeviceSP dab = cachedDab();

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), scale, 0.0, info);
    } else {
        KoColor color = painter()->paintColor();
        color.convertTo(dab->colorSpace());
        brush->mask(dab, color, scale, scale, 0.0, info);
    }

    QRect dabRect = QRect(0, 0, brush->maskWidth(scale, 0.0), brush->maskHeight(scale, 0.0));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    if (painter()->bounds().isValid()) {
        dstRect &= painter()->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    const KoColorSpace * cs = dab->colorSpace();

    // Set all alpha > opaque/2 to opaque, the rest to transparent.
    // XXX: Using 4/10 as the 1x1 circle brush paints nothing with 0.5.
    quint8* dabPointer = dab->data();
    QRect rc = dab->bounds();
    int pixelSize = dab->pixelSize();
    for (int i = 0; i < rc.width() * rc.height(); i++) {
        quint8 alpha = cs->alpha(dabPointer);

        if (alpha < (4 * OPACITY_OPAQUE) / 10) {
            cs->setAlpha(dabPointer, OPACITY_TRANSPARENT, 1);
        } else {
            cs->setAlpha(dabPointer, OPACITY_OPAQUE, 1);
        }

        dabPointer += pixelSize;
    }

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    painter()->bltFixed(dstRect.x(), dstRect.y(), dab, sx, sy, sw, sh);
    painter()->setOpacity(origOpacity);
    painter()->setPaintColor(origColor);
}
