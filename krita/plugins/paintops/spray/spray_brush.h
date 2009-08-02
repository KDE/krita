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

class QRect;

class SprayBrush
{

public:
    SprayBrush(const KoColor &inkColor);
    SprayBrush();
    ~SprayBrush();
    SprayBrush(KoColor inkColor);
    void paint(KisPaintDeviceSP dev, const KisPaintInformation& info, const KoColor &color);
    
    /// Paints Wu Particle
    void paintParticle(KisRandomAccessor &writeAccessor,const KoColor &color,qreal rx, qreal ry);
    void paintCircle(KisPainter &painter,qreal x, qreal y, int radius, int steps);
    void paintEllipse(KisPainter &painter,qreal x, qreal y, int a, int b, qreal angle, int steps);
    void paintRectangle(KisPainter &painter,qreal x, qreal y, int width, int height, qreal angle, int steps);

    void paintMetaballs(KisPaintDeviceSP dev, const KisPaintInformation &info, const KoColor &painterColor);
    void paintDistanceMap(KisPaintDeviceSP dev, const KisPaintInformation &info, const KoColor &painterColor);

    void paintOutline(KisPaintDeviceSP dev, const KoColor& painterColor, qreal posX, qreal posY, qreal radius);

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

    // set the amount of the jittering brush position
    void setAmount(qreal amount){
        m_amount = amount;
    }

    void setScale(qreal scale){
        m_scale = scale;
    }

    void setObject(int object){
        m_object = object;
    }

    void setShape(int shape){
        m_shape = shape;
    }

    void setJitterShapeSize(bool jitterShapeSize){
        m_jitterShapeSize = jitterShapeSize;
    }

    void setObjectDimension(int width, int height){
        m_width = width;
        m_height = height;
    }

    // getters
    qreal radius(){
        return m_radius;
    }

    qreal objectWidth(){
        return m_width;
    }

    qreal objectHeight(){
        return m_height;
    }

    // setters

    void setUseDensity(bool useDensity){
        m_useDensity = useDensity;
    }

    void setParticleCount(int particleCount){
        m_particlesCount = particleCount;
    }

    void setMaxTreshold(qreal tresh){
        m_maxtresh = tresh;
    }

    void setMinTreshold(qreal tresh){
        m_mintresh = tresh;
    }

    void setRendering(bool highQuality){
        m_highQuality = highQuality;
    }

    void setRadius(qreal radiusX, qreal radiusY){
        m_radiusX = qRound(radiusX);
        m_radiusY = qRound(radiusY);
    }

    void setComputeArea(QRect area){
        m_computeArea = area;
    }

    // set true if the particles should have random opacity
    void setUseRandomOpacity(bool isRandom){
        m_randomOpacity = isRandom;
    }

    void setSettingsObject(const KisSprayPaintOpSettings* settings){
        m_settings = settings;
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

    // particles
    int m_particlesCount;
    bool m_useDensity;

    // metaballs
    qreal m_maxtresh;
    qreal m_mintresh;

    // color options
    bool m_randomOpacity;

    int m_radiusX;
    int m_radiusY;

    bool m_highQuality;

    QRect m_computeArea;

private:
    const KisSprayPaintOpSettings* m_settings;
    
};

#endif
