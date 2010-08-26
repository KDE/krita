/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com> 
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

#include <QRect>

#include <kis_image.h>
#include <kis_vec.h>
#include <kis_debug.h>

#include <KoColorTransformation.h>
#include <KoColor.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_brush_based_paintop_settings.h>
#include <kis_color_source.h>

KisBrushOp::KisBrushOp(const KisBrushBasedPaintOpSettings *settings, KisPainter *painter, KisImageWSP image)
        : KisBrushBasedPaintOp(settings, painter), m_hsvTransfo(0)
{
    Q_UNUSED(image);
    Q_ASSERT(settings);
    Q_ASSERT(painter);

    m_colorSource = new KisPlainColorSource(painter->backgroundColor(), painter->paintColor());
    
    m_hsvOptions.append(KisPressureHSVOption::createHueOption());
    m_hsvOptions.append(KisPressureHSVOption::createSaturationOption());
    m_hsvOptions.append(KisPressureHSVOption::createValueOption());
    
    foreach(KisPressureHSVOption* option, m_hsvOptions)
    {
        option->readOptionSetting(settings);
        option->sensor()->reset();
        if(option->isChecked() && !m_hsvTransfo)
        {
            m_hsvTransfo = painter->backgroundColor().colorSpace()->createColorTransformation("hsv_adjustment", QHash<QString, QVariant>());
        }
    }
    
    m_sizeOption.readOptionSetting(settings);
    m_mirrorOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_darkenOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_mixOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);
    
    m_sizeOption.sensor()->reset();
    m_mirrorOption.sensor()->reset();
    m_opacityOption.sensor()->reset();
    m_darkenOption.sensor()->reset();
    m_rotationOption.sensor()->reset();
    m_scatterOption.sensor()->reset();
}

KisBrushOp::~KisBrushOp()
{
    qDeleteAll(m_hsvOptions);
    delete m_colorSource;
    delete m_hsvTransfo;
}

double KisBrushOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()->device()) return 1.0;

    KisBrushSP brush = m_brush;
    Q_ASSERT(brush);
    if (!brush)
        return 1.0;
    
    if (!brush->canPaintFor(info))
        return 1.0;

    double scale = KisPaintOp::scaleForPressure(m_sizeOption.apply(info));
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return spacing(scale);

    KisPaintDeviceSP device = painter()->device();

    double rotation = m_rotationOption.apply(info);
    
    QPointF hotSpot = brush->hotSpot(scale, scale, rotation);
    // return info.pos() if sensor is not enabled
    QPointF pos = m_scatterOption.apply(info, qMax(brush->width(), brush->height()) * scale); 
    QPointF pt = pos - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    quint8 origOpacity = m_opacityOption.apply(painter(), info);
    m_colorSource->selectColor(m_mixOption.apply(info) );
    KoColor origColor = painter()->paintColor();
    m_darkenOption.apply(m_colorSource, info);

    if(m_hsvTransfo)
    {
        foreach(KisPressureHSVOption* option, m_hsvOptions)
        {
            option->apply(m_hsvTransfo, info);
        }
        m_colorSource->applyColorTransformation(m_hsvTransfo);
    }

    painter()->setPaintColor( m_colorSource->uniformColor() );

    KisFixedPaintDeviceSP dab = cachedDab(device->colorSpace());
    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->paintDevice(device->colorSpace(), scale, rotation, info, xFraction, yFraction);
    } else {
        KoColor color = painter()->paintColor();
        color.convertTo(dab->colorSpace());
        brush->mask(dab, color, scale, scale, rotation, info, xFraction, yFraction);
    }
    
    MirrorProperties mirrors = m_mirrorOption.apply(info);
    dab->mirror(mirrors.horizontalMirror, mirrors.verticalMirror);
    
    painter()->bltFixed(QPoint(x, y), dab, dab->bounds());
    painter()->setOpacity(origOpacity);
    painter()->setPaintColor(origColor);

    return spacing(scale);
}
