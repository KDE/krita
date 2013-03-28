/*
 *  Copyright (c) 2008-2012 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_spray_paintop.h"
#include "kis_spray_paintop_settings.h"

#include <cmath>

#include <QRect>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>

#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_fixed_paint_device.h>
#include <kis_brush_option.h>
#include <kis_sprayop_option.h>
#include <kis_spray_shape_option.h>
#include <kis_color_option.h>

KisSprayPaintOp::KisSprayPaintOp(const KisSprayPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
        , m_settings(settings)
        , m_isPresetValid(true)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    Q_UNUSED(image);

    m_rotationOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_rotationOption.sensor()->reset();
    m_opacityOption.sensor()->reset();
    m_sizeOption.sensor()->reset();

    m_brushOption.readOptionSetting(settings);

    m_colorProperties.fillProperties(settings);
    m_properties.loadSettings(settings);
    // first load tip properties as shape properties are dependent on diameter/scale/aspect
    m_shapeProperties.loadSettings(settings,m_properties.diameter * m_properties.scale, m_properties.diameter * m_properties.aspect * m_properties.scale );

    // TODO: what to do with proportional sizes?
    m_shapeDynamicsProperties.loadSettings(settings);

    if (!m_shapeProperties.enabled && !m_brushOption.brush()) {
        // in case the preset does not contain the definition for KisBrush
        m_isPresetValid = false;
        kWarning() << "Preset is not valid. Painting is not possible. Use the preset editor to fix current brush engine preset.";
    }

    m_sprayBrush.setProperties( &m_properties,&m_colorProperties,
                                &m_shapeProperties, &m_shapeDynamicsProperties, m_brushOption.brush());

    m_sprayBrush.setFixedDab( cachedDab() );

    // spacing
    if ((m_properties.diameter * 0.5) > 1) {
        m_ySpacing = m_xSpacing = m_properties.diameter * 0.5 * m_properties.spacing;
    } else {
        m_ySpacing = m_xSpacing = 1.0;
    }
    m_spacing = m_xSpacing;
}

KisSprayPaintOp::~KisSprayPaintOp()
{
}

qreal KisSprayPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter() || !m_isPresetValid) {
        return m_spacing;
    }

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->preferredDabColorSpace());
    } else {
        m_dab->clear();
    }

    qreal rotation = m_rotationOption.apply(info);
    quint8 origOpacity = m_opacityOption.apply(painter(), info);
    // Spray Brush is capable of working with zero scale,
    // so no additional checks for 'zero'ness are needed
    qreal scale = m_sizeOption.apply(info);

    setCurrentRotation(rotation);
    setCurrentScale(scale);

    m_sprayBrush.paint( m_dab,
                        m_settings->node()->paintDevice(),
                        info,
                        rotation,
                        scale,
                        painter()->paintColor(),
                        painter()->backgroundColor());

    QRect rc = m_dab->extent();
    painter()->bitBlt(rc.topLeft(), m_dab, rc);
    painter()->renderMirrorMask(rc, m_dab);
    painter()->setOpacity(origOpacity);

    return m_spacing;
}
