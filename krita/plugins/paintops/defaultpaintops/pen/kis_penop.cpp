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
#include <kis_brush_based_paintop_settings.h>

KisPenOp::KisPenOp(const KisBrushBasedPaintOpSettings *settings, KisPainter *painter, KisImageWSP image)
        : KisBrushBasedPaintOp(settings, painter)
{
    Q_UNUSED(image);
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_darkenOption.readOptionSetting(settings);
    m_sizeOption.sensor()->reset();
    m_opacityOption.sensor()->reset();
    m_darkenOption.sensor()->reset();
}

KisPenOp::~KisPenOp()
{
}

double KisPenOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()->device()) return 1.0;

    KisBrushSP brush = m_brush;
    if (!m_brush)
        return 1.0;

    if (! brush->canPaintFor(info))
        return 1.0;

    double scale = KisPaintOp::scaleForPressure(m_sizeOption.apply(info));
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return spacing(scale);

    KisPaintDeviceSP device = painter()->device();
    QPointF hotSpot = brush->hotSpot(scale, scale);
    QPointF pt = info.pos() - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x = qRound(pt.x());
    qint32 y = qRound(pt.y());

    quint8 origOpacity = m_opacityOption.apply(painter(), info);
    KoColor origColor = m_darkenOption.apply(painter(), info);

    KisFixedPaintDeviceSP dab = cachedDab();

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->paintDevice(device->colorSpace(), scale, 0.0, info);
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

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return 1.0;

    const KoColorSpace * cs = dab->colorSpace();

    // Set all alpha > opaque/2 to opaque, the rest to transparent.
    // XXX: Using 4/10 as the 1x1 circle brush paints nothing with 0.5.
    quint8* dabPointer = dab->data();
    QRect rc = dab->bounds();
    int pixelSize = dab->pixelSize();
    for (int i = 0; i < rc.width() * rc.height(); i++) {
        quint8 alpha = cs->opacityU8(dabPointer);

        if (alpha < (4 * OPACITY_OPAQUE_U8) / 10) {
            cs->setOpacity(dabPointer, OPACITY_TRANSPARENT_U8, 1);
        } else {
            cs->setOpacity(dabPointer, OPACITY_OPAQUE_U8, 1);
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
    
    return spacing(scale);
}

KisDistanceInformation KisPenOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisDistanceInformation& savedDist)
{
    if(m_brush && m_brush->width() == 1 && m_brush->height() == 1) {
        
        if (!m_dab) {
            m_dab = new KisPaintDevice(painter()->device()->colorSpace());
        } else {
            m_dab->clear();
        }
        
        KisPainter p(m_dab);
        p.setPaintColor(painter()->paintColor());
        p.drawDDALine(pi1.pos(), pi2.pos());

        QRect rc = m_dab->extent();  
        painter()->bitBlt(rc.x(), rc.y(), m_dab, rc.x(), rc.y(), rc.width(), rc.height());
        
        return KisDistanceInformation(0.0, 0.0);
    }
    return KisPaintOp::paintLine(pi1, pi2, savedDist);
}

