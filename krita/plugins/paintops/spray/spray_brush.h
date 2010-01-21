/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <KoColor.h>

#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_paint_information.h"

#include "kis_color_option.h"

#include "random_gauss.h"


#include <QImage>


class QRect;

class KisSprayProperties{
public:
    quint16 diameter;
    quint16 radius;
    quint16 particleCount;
    qreal aspect;
    qreal coverage;
    qreal amount;
    qreal spacing;
    qreal scale;
    qreal brushRotation;
    bool jitterMovement;
    bool useDensity;
    // particle type size
    quint8 shape;
    quint16 width;
    quint16 height;
    bool randomSize;
    bool proportional;
    // distribution
    bool gaussian;
    // rotation
    bool fixedRotation;
    bool randomRotation;
    bool followCursor;
    quint16 fixedAngle;
    qreal randomRotationWeight;
    qreal followCursorWeigth;
    QImage image;
};


class SprayBrush
{

public:
    SprayBrush();
    ~SprayBrush();

    void paint(KisPaintDeviceSP dab, KisPaintDeviceSP source,  const KisPaintInformation& info, qreal rotation, qreal scale, const KoColor &color, const KoColor &bgColor);
    void setProperties(KisSprayProperties * properties, KisColorProperties * colorProperties){
        m_properties = properties;
        m_colorProperties = colorProperties;
    }
    
private:
    KoColor m_inkColor;
    qreal m_radius;
    quint32 m_particlesCount; 
    quint8 m_pixelSize;

    RandomGauss * m_rand;
    KisPainter * m_painter;
    KisPaintDeviceSP m_imageDevice;
    QImage m_brushQImage;
    QImage m_transformed;

    const KisSprayProperties * m_properties;
    const KisColorProperties * m_colorProperties;

private:
    /// rotation in radians according the settings (gauss distribution, uniform distribution or fixed angle)
    qreal rotationAngle();
    /// Paints Wu Particle
    void paintParticle(KisRandomAccessor &writeAccessor, const KoColor &color, qreal rx, qreal ry);
    void paintCircle(KisPainter * painter, qreal x, qreal y, int radius, int steps);
    void paintEllipse(KisPainter * painter, qreal x, qreal y, int a, int b, qreal angle, int steps);
    void paintRectangle(KisPainter * painter, qreal x, qreal y, int width, int height, qreal angle, int steps);

    void paintOutline(KisPaintDeviceSP dev, const KoColor& painterColor, qreal posX, qreal posY, qreal radius);

    /// mix a with b.b mix with weight and a with 1.0 - weight 
    inline qreal linearInterpolation(qreal a, qreal b, qreal weight){
        return (1.0 - weight) * a + weight * b;
    }

    // TODO: move this somewhere where I can reuse it 
    /// convert radians to degrees
    inline qreal rad2deg(qreal rad){
        return rad * (180.0/M_PI);
    }

    /// convert degrees to radians
    inline qreal deg2rad(quint16 deg){
        return deg * (M_PI/180.0);
    }
};

#endif
