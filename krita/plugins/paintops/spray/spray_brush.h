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
#include "kis_painter.h"

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
    void paintCircle(KisPainter &painter,qreal x, qreal y, int radius, int steps);
    void paintEllipse(KisPainter &painter,qreal x, qreal y, int a, int b, qreal angle, int steps);
    void paintRectangle(KisPainter &painter,qreal x, qreal y, int width, int height, qreal angle, int steps);

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

    void setAmount(qreal amount){
        m_amount = amount;
    }

    void setScale(qreal scale){
        m_scale = scale;
    }

    void setCurveData(const QList<qreal>& curveData);

    void setObject(int object){
        m_object = object;
    }

    void setShape(int shape){
        m_shape = shape;
    }

    void setJitterShapeSize(bool jitterShapeSize){
        m_jitterShapeSize = jitterShapeSize;
    }

    void setObjectDimenstion(int width, int height){
        m_width = width;
        m_height = height;
    }

    

private:
    KoColor m_inkColor;
    int m_counter;
    qreal m_radius;
    int m_pixelSize;
    int m_diameter;
    qreal m_scale;

    // jitter
    bool m_jitterSize;
    bool m_jitterMovement;

    qreal m_coverage;
    // amount of jitter for movement
    qreal m_amount;

    int m_object;
    int m_shape;
    // object shape
    int m_width;
    int m_height;
    bool m_jitterShapeSize;
};

#endif
