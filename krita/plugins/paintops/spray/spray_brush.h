/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef _SPRAY_BRUSH_H_
#define _SPRAY_BRUSH_H_

#include <QVector>

#include <KoColor.h>

#include "kis_paint_device.h"

class SprayBrush
{

public:
    SprayBrush(const KoColor &inkColor);
    SprayBrush();
    ~SprayBrush();
    SprayBrush(KoColor inkColor);
    void paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color);

    /// Paints Wu Particle
    void paintParticle(KisRandomAccessor &writeAccessor,const KoColor &color,qreal rx, qreal ry);

    void setDiameter(int diameter){
        m_diameter = diameter;
        m_radius = diameter * 0.5;
    }

    void setCoverity(qreal coverage){
        m_coverage = coverage;
    }

    void setJitterSize(bool jitterSize){
        m_jitterSize = jitterSize;
    }

    void setJitterMovement(bool jitterMovement){
        m_jitterMovement = jitterMovement;
    }

    void setUseParticles(bool useParticles){
        m_useParticles = useParticles;
    }

    void setAmount(qreal amount){
        m_amount = amount;
    }

    void setCurveData(const QList<qreal>& curveData);
private:
    KoColor m_inkColor;
    int m_counter;
    qreal m_radius;
    int m_pixelSize;
    int m_diameter;


    // jitter
    bool m_jitterSize;
    bool m_jitterMovement;
    bool m_useParticles;
    qreal m_coverage;
    // amount of jitter for movement
    qreal m_amount;
};

#endif
