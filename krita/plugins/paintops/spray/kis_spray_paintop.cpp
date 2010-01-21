/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <kis_sprayop_option.h>
#include <kis_spray_shape_option.h>
#include <kis_color_option.h>

#ifdef BENCHMARK
#include <QTime>
#endif


KisSprayPaintOp::KisSprayPaintOp(const KisSprayPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
        , m_settings(settings)
        , m_image(image)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);

    m_rotationOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_rotationOption.sensor()->reset();
    m_opacityOption.sensor()->reset();
    m_sizeOption.sensor()->reset();

    loadSettings(settings);
    m_sprayBrush.setProperties( &m_properties, &m_colorProperties);
    
    // spacing
    if ((m_properties.diameter * 0.5) > 1) {
        m_ySpacing = m_xSpacing = m_properties.diameter * 0.5 * m_properties.spacing;
    } else {
        m_ySpacing = m_xSpacing = 1.0;
    }
    m_spacing = m_xSpacing;
    
#ifdef BENCHMARK
    m_count = m_total = 0;
#endif
}

void KisSprayPaintOp::loadSettings(const KisSprayPaintOpSettings* settings)
{
    m_colorProperties.fillProperties(settings);
    
    // read the properties into primitive datatypes (just once)
    // spray
    m_properties.diameter = settings->getInt(SPRAY_DIAMETER);
    m_properties.radius =  qRound(0.5 * m_properties.diameter);
    m_properties.aspect = settings->getDouble(SPRAY_ASPECT);
    m_properties.particleCount = settings->getDouble(SPRAY_PARTICLE_COUNT);
    m_properties.coverage = (settings->getDouble(SPRAY_COVERAGE) / 100.0);
    m_properties.amount = settings->getDouble(SPRAY_JITTER_MOVE_AMOUNT);
    m_properties.spacing = settings->getDouble(SPRAY_SPACING);
    m_properties.scale = settings->getDouble(SPRAY_SCALE);
    m_properties.brushRotation = settings->getDouble(SPRAY_ROTATION);
    m_properties.jitterMovement = settings->getBool(SPRAY_JITTER_MOVEMENT);
    m_properties.useDensity = settings->getBool(SPRAY_USE_DENSITY);
    m_properties.gaussian = settings->getBool(SPRAY_GAUSS_DISTRIBUTION);

    // sprayshape
    m_properties.proportional = settings->getBool(SPRAYSHAPE_PROPORTIONAL);
    m_properties.width = settings->getInt(SPRAYSHAPE_WIDTH);
    m_properties.height = settings->getInt(SPRAYSHAPE_HEIGHT);
    if (m_properties.proportional)
    {
        m_properties.width = (m_properties.width / 100.0) * m_properties.diameter * m_properties.scale;
        m_properties.height = (m_properties.height / 100.0) * m_properties.diameter * m_properties.aspect * m_properties.scale;
    }
    
    // particle type size
    m_properties.shape = settings->getInt(SPRAYSHAPE_SHAPE);
    m_properties.randomSize = settings->getBool(SPRAYSHAPE_RANDOM_SIZE);
    
    // rotation
    m_properties.fixedRotation = settings->getBool(SPRAYSHAPE_FIXED_ROTATION);
    m_properties.randomRotation = settings->getBool(SPRAYSHAPE_RANDOM_ROTATION);
    m_properties.followCursor = settings->getBool(SPRAYSHAPE_FOLLOW_CURSOR);
    m_properties.fixedAngle = settings->getInt(SPRAYSHAPE_FIXED_ANGEL);
    m_properties.randomRotationWeight = settings->getDouble(SPRAYSHAPE_RANDOM_ROTATION_WEIGHT);
    m_properties.followCursorWeigth = settings->getDouble(SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT);
    m_properties.image = settings->image();
}


KisSprayPaintOp::~KisSprayPaintOp()
{
}

double KisSprayPaintOp::spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const
{
    Q_UNUSED(pressure1);
    Q_UNUSED(pressure2);
    xSpacing = m_xSpacing;
    ySpacing = m_ySpacing;

    return m_spacing;
}


void KisSprayPaintOp::paintAt(const KisPaintInformation& info)
{
#ifdef BENCHMARK
    QTime time;
    time.start();
#endif

    if (!painter()) return;

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    } else {
        m_dab->clear();
    }

    double rotation = m_rotationOption.apply(info);
    quint8 origOpacity = m_opacityOption.apply(painter(), info);
    double scale = KisPaintOp::scaleForPressure(m_sizeOption.apply(info));

    m_sprayBrush.paint( m_dab,
                        m_settings->node()->paintDevice(), 
                        info, 
                        rotation,
                        scale,
                        painter()->paintColor(), 
                        painter()->backgroundColor());

    QRect rc = m_dab->extent();
    painter()->bitBlt(rc.topLeft(), m_dab, rc);
    painter()->setOpacity(origOpacity);

#ifdef BENCHMARK
    int msec = time.elapsed();
    kDebug() << msec << " ms/dab " << "[average: " << m_total / (qreal)m_count << "]";
    m_total += msec;
    m_count++;
#endif
}

