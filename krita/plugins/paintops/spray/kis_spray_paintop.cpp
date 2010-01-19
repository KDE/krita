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
    m_sprayBrush.setProperties(&m_properties);
    
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
    // read the properties into primitive datatypes (just once)
    m_properties.diameter = settings->diameter();
    m_properties.radius =  qRound(0.5 * settings->diameter());
    m_properties.particleCount = settings->particleCount(); 
  
    m_properties.aspect = settings->aspect();
    m_properties.coverage = (settings->coverage() / 100.0);
    m_properties.amount = settings->amount(); 
    m_properties.spacing = settings->spacing();
    m_properties.proportional = settings->proportional();
    
    m_properties.scale = settings->scale(); 
    m_properties.brushRotation = settings->brushRotation();
    m_properties.jitterMovement = settings->jitterMovement();
    m_properties.jitterSize = settings->randomSize();
    m_properties.useDensity = settings->useDensity();
    
    m_properties.useRandomOpacity = settings->useRandomOpacity();    
    m_properties.useRandomHSV = settings->useRandomHSV();
    m_properties.hue = settings->hue();
    m_properties.saturation = settings->saturation();
    m_properties.value = settings->value();

    m_properties.colorPerParticle = settings->colorPerParticle();
    m_properties.fillBackground = settings->fillBackground();
    m_properties.mixBgColor = settings->mixBgColor();
    m_properties.sampleInput = settings->sampleInput();
    
    if (m_properties.proportional)
    {
        m_properties.width = (settings->width()  / 100.0) * m_properties.diameter * m_properties.scale;
        m_properties.height = (settings->height() / 100.0) * m_properties.diameter * m_properties.aspect * m_properties.scale;
    }else
    {
        m_properties.width = settings->width();
        m_properties.height = settings->height();
    }
    // particle type size
    m_properties.shape = settings->shape();
    m_properties.randomSize = settings->randomSize(); 
    
    // distribution
    m_properties.gaussian = settings->gaussian();
    // rotation
    m_properties.fixedRotation = settings->fixedRotation();
    m_properties.randomRotation = settings->randomRotation();
    m_properties.followCursor = settings->followCursor();
    m_properties.fixedAngle = settings->fixedAngle();
    m_properties.randomRotationWeight = settings->randomRotationWeight();
    m_properties.followCursorWeigth = settings->followCursorWeigth();
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

