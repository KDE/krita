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
#include <qrect.h>

#include <KoColor.h>

#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_paint_information.h"

#include "kis_spray_paintop_settings.h"

#include "random_gauss.h"

class QRect;

class SprayBrush
{

public:
    SprayBrush();
    ~SprayBrush();

    void paint(KisPaintDeviceSP dab, KisPaintDeviceSP source,  const KisPaintInformation& info, const KoColor &color, const KoColor &bgColor);

    /// Paints Wu Particle
    void paintParticle(KisRandomAccessor &writeAccessor, const KoColor &color, qreal rx, qreal ry);
    void paintCircle(KisPainter * painter, qreal x, qreal y, int radius, int steps);
    void paintEllipse(KisPainter * painter, qreal x, qreal y, int a, int b, qreal angle, int steps);
    void paintRectangle(KisPainter * painter, qreal x, qreal y, int width, int height, qreal angle, int steps);

    void paintOutline(KisPaintDeviceSP dev, const KoColor& painterColor, qreal posX, qreal posY, qreal radius);

    void setDiameter(int diameter) {
        m_diameter = diameter;
        m_radius = diameter * 0.5;
    }

    void setCoverity(qreal coverage) {
        m_coverage = coverage;
    }

    // set the amount of the jittering brush position
    void setAmount(qreal amount) {
        m_amount = amount;
    }

    void setScale(qreal scale) {
        m_scale = scale;
    }

    void setJitterShapeSize(bool jitterShapeSize) {
        m_jitterShapeSize = jitterShapeSize;
    }

    void setObjectDimension(int width, int height) {
        m_width = width;
        m_height = height;
    }

    // setters

    void setUseDensity(bool useDensity) {
        m_useDensity = useDensity;
    }

    void setParticleCount(int particleCount) {
        m_particlesCount = particleCount;
    }

    // set true if the particles should have random opacity
    void setUseRandomOpacity(bool isRandom) {
        m_randomOpacity = isRandom;
    }

    void setSettingsObject(const KisSprayPaintOpSettings* settings) {
        m_settings = settings;
    }

    void init();

private:
    KoColor m_inkColor;
    qreal m_radius;
    int m_pixelSize;
    int m_diameter;
    qreal m_scale;

    qreal m_coverage;

    // amount of jitter for movement
    qreal m_amount;

    // object shape
    int m_width;
    int m_height;
    bool m_jitterShapeSize;

    // particles
    int m_particlesCount;
    bool m_useDensity;

    // color options
    bool m_randomOpacity;

    RandomGauss * m_rand;
    KisPainter * m_painter;
    KisPaintDeviceSP m_imageDevice;
    QImage m_brushQImage;
    QImage m_transformed;

    const KisSprayPaintOpSettings* m_settings;
    
private:
    /// rotation in radians according the settings (gauss distribution, uniform distribution or fixed angle)
    qreal rotationAngle();
    /// mix a with b.b mix with weight and a with 1.0 - weight 
    inline qreal linearInterpolation(qreal a, qreal b, qreal weight){
        return (1.0 - weight) * a + weight * b;
    }

    /// particle dimension
    inline qreal objectWidth() {
        return m_width;
    }
    inline qreal objectHeight() {
        return m_height;
    }

};

#endif
