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


#ifndef _PARTICLE_BRUSH_H_
#define _PARTICLE_BRUSH_H_

#include "kis_paint_device.h"
#include "kis_debug.h"
#include <QPointF>


class KisParticleBrushProperties {
public:
    quint16 particleCount;
    quint16 iterations;
    qreal weight;
    qreal gravity;
    QPointF scale;
};

class KisRandomAccessor;
class KoColorSpace;
class KoColor;

class ParticleBrush{

public:

    ParticleBrush();
    ~ParticleBrush();
    void initParticles();
    void draw(KisPaintDeviceSP dab,const KoColor& color,QPointF pos);

    void setInitialPosition(QPointF pos);
    void setProperties(KisParticleBrushProperties * properties){        m_properties = properties;    }
    
private:
    /// paints wu particle, similar to spray version but you can turn on respecting opacity of the tool and add weight to opacity
    /// also the particle respects opacity in the destination pixel buffer
    void paintParticle(KisRandomAccessor& writeAccessor,KoColorSpace * cs, QPointF pos, const KoColor& color, qreal weight, bool respectOpacity);

    QVector<QPointF> m_particlePos;
    QVector<QPointF> m_particleNextPos;
    QVector<qreal> m_accelaration;
    
    KisParticleBrushProperties * m_properties;
};

#endif
