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

#include "kis_brushop.h"

#include <string.h>

#include <QRect>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDomElement>
#include <QHBoxLayout>
#include <qtoolbutton.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoInputDevice.h>

#include <kis_brush.h>
#include <kis_datamanager.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_properties_configuration.h>
#include <kis_selection.h>
#include <kis_brush_option.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>

#include <kis_brushop_settings.h>
#include <kis_brushop_settings_widget.h>


KisBrushOp::KisBrushOp(const KisBrushOpSettings *settings, KisPainter *painter)
    : KisBrushBasedPaintOp(painter)
    , settings(settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    Q_ASSERT(settings->m_optionsWidget->m_brushOption);
    m_brush = settings->m_optionsWidget->m_brushOption->brush();
}

KisBrushOp::~KisBrushOp()
{
}

void KisBrushOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()->device()) return;

    KisBrushSP brush = m_brush;
    Q_ASSERT(brush);
    if (!brush) return;

    KisPaintInformation adjustedInfo = settings->m_optionsWidget->m_sizeOption->apply(info);
    if (!brush->canPaintFor(adjustedInfo))
        return;

    KisPaintDeviceSP device = painter()->device();

    double pScale = KisPaintOp::scaleForPressure(adjustedInfo.pressure());   // TODO: why is there scale and pScale that seems to contains the same things ?
    QPointF hotSpot = brush->hotSpot(pScale, pScale);
    QPointF pt = info.pos() - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    quint8 origOpacity = settings->m_optionsWidget->m_opacityOption->apply(painter(), info.pressure());
    KoColor origColor = settings->m_optionsWidget->m_darkenOption->apply(painter(), info.pressure());

    double scale = KisPaintOp::scaleForPressure(adjustedInfo.pressure());

    QRect dabRect = QRect(0, 0, brush->maskWidth(scale, 0.0), brush->maskHeight(scale, 0.0));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());


    if (painter()->bounds().isValid()) {
        dstRect &= painter()->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    KisPaintDeviceSP dab = KisPaintDeviceSP(0);

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), scale, 0.0, adjustedInfo, xFraction, yFraction);
    } else {
        dab = cachedDab();
        dab->clear();
        KoColor color = painter()->paintColor();
        color.convertTo(dab->colorSpace());
        brush->mask(dab, color, scale, scale, 0.0, info, xFraction, yFraction);
    }

    painter()->bitBlt(dstRect.x(), dstRect.y(), dab, sx, sy, sw, sh);

    painter()->setOpacity(origOpacity);
    painter()->setPaintColor(origColor);

}
