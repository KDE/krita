/*
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

#include "kis_particle_paintop.h"
#include "kis_particle_paintop_settings.h"

#include <cmath>

#include "kis_vec.h"

#include <KoCompositeOp.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_paint_information.h>

#include "kis_particleop_option.h"

#include "particle_brush.h"

KisParticlePaintOp::KisParticlePaintOp(const KisParticlePaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
{
    if(!image) return;
    
    m_properties.particleCount = settings->getInt(PARTICLE_COUNT);
    m_properties.iterations = settings->getInt(PARTICLE_ITERATIONS);
    m_properties.gravity = settings->getDouble(PARTICLE_GRAVITY);
    m_properties.weight = settings->getDouble(PARTICLE_WEIGHT);
    m_properties.scale = QPointF(settings->getDouble(PARTICLE_SCALE_X),settings->getDouble(PARTICLE_SCALE_Y));;

    m_particleBrush.setProperties( &m_properties );
    m_particleBrush.initParticles();

    m_first = true;
}

KisParticlePaintOp::~KisParticlePaintOp()
{
}

void KisParticlePaintOp::paintAt(const KisPaintInformation& info)
{
    paintLine(info, info);
}


double KisParticlePaintOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, double savedDist)
{
    Q_UNUSED(savedDist);
    if (!painter()) return -1;

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    }
    else {
        m_dab->clear();
    }
    

    if (m_first){
        m_particleBrush.setInitialPosition(pi1.pos());
        m_first = false;
    }

    m_particleBrush.draw(m_dab, painter()->paintColor(), pi2.pos());
    QRect rc = m_dab->extent();
    
    painter()->bitBlt(rc.x(), rc.y(), m_dab, rc.x(), rc.y(), rc.width(), rc.height());

    QPointF diff = pi2.pos() - pi1.pos();
    return sqrt( diff.x()*diff.x() + diff.y()*diff.y() );
}
