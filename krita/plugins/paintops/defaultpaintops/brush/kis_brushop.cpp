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
#include <kis_pressure_rotation_option.h>


KisBrushOp::KisBrushOp(const KisBrushOpSettings *settings, KisPainter *painter)
        : KisBrushBasedPaintOp(painter)
        , settings(settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    if (settings && settings->m_options) {
        Q_ASSERT(settings->m_options->m_brushOption);
        m_brush = settings->m_options->m_brushOption->brush();
        settings->m_options->m_sizeOption->sensor()->reset();
        settings->m_options->m_opacityOption->sensor()->reset();
        settings->m_options->m_darkenOption->sensor()->reset();
        settings->m_options->m_rotationOption->sensor()->reset();
    }
}

KisBrushOp::~KisBrushOp()
{
}

void KisBrushOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()->device()) return;
    if (!settings->m_options) return;

    KisBrushSP brush = m_brush;
    Q_ASSERT(brush);
    if (!brush) {
        if (settings->m_options) {
            m_brush = settings->m_options->m_brushOption->brush();
            brush = m_brush;
        } else {
            return;
        }
    }

    if (!brush->canPaintFor(info))
        return;

    double scale = KisPaintOp::scaleForPressure(settings->m_options->m_sizeOption->apply(info));
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return;

    KisPaintDeviceSP device = painter()->device();

    double rotation = settings->m_options->m_rotationOption->apply(info);
    
    QPointF hotSpot = brush->hotSpot(scale, scale, rotation);
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

    quint8 origOpacity = settings->m_options->m_opacityOption->apply(painter(), info);
    KoColor origColor = settings->m_options->m_darkenOption->apply(painter(), info);

    KisFixedPaintDeviceSP dab = cachedDab(device->colorSpace());
    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), scale, rotation, info, xFraction, yFraction);
    } else {
        KoColor color = painter()->paintColor();
        color.convertTo(dab->colorSpace());
        brush->mask(dab, color, scale, scale, rotation, info, xFraction, yFraction);
    }

    painter()->bltFixed(QPoint(x, y), dab, dab->bounds());
    painter()->setOpacity(origOpacity);
    painter()->setPaintColor(origColor);

}
