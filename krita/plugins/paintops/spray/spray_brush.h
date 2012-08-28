
class KisBrush;
class KisBrushOption;/*
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
#include "kis_spray_shape_option.h"
#include "kis_spray_shape_dynamics.h"
#include "kis_sprayop_option.h"


#include "random_gauss.h"

#include <QImage>
#include <kis_brush.h>
class QRect;

class SprayBrush
{

public:
    SprayBrush();
    ~SprayBrush();

    void paint(KisPaintDeviceSP dab, KisPaintDeviceSP source,  const KisPaintInformation& info, qreal rotation, qreal scale, const KoColor &color, const KoColor &bgColor);
    void setProperties(KisSprayProperties * properties, 
                       KisColorProperties * colorProperties, 
                       KisShapeProperties * shapeProperties, 
                       KisShapeDynamicsProperties * shapeDynamicsProperties,
                       KisBrushSP brush){
        m_properties = properties;
        m_colorProperties = colorProperties;
        m_shapeProperties = shapeProperties;
        m_shapeDynamicsProperties = shapeDynamicsProperties;
        m_brush = brush;
    }
   
    void setFixedDab(KisFixedPaintDeviceSP dab){
        m_fixedDab = dab;
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

    KoColorTransformation* m_transfo;
    
    const KisSprayProperties * m_properties;
    const KisColorProperties * m_colorProperties;
    const KisShapeProperties * m_shapeProperties;
    const KisShapeDynamicsProperties * m_shapeDynamicsProperties;
    
    const KisBrushSP m_brush;
    KisFixedPaintDeviceSP m_fixedDab;
    
private:
    /// rotation in radians according the settings (gauss distribution, uniform distribution or fixed angle)
    qreal rotationAngle();
    /// Paints Wu Particle
    void paintParticle(KisRandomAccessorSP &writeAccessor, const KoColor &color, qreal rx, qreal ry);
    void paintCircle(KisPainter * painter, qreal x, qreal y, int radius, int steps);
    void paintEllipse(KisPainter * painter, qreal x, qreal y, int a, int b, qreal angle, int steps);
    void paintRectangle(KisPainter * painter, qreal x, qreal y, int width, int height, qreal angle, int steps);

    void paintOutline(KisPaintDeviceSP dev, const KoColor& painterColor, qreal posX, qreal posY, qreal radius);

    /// mix a with b.b mix with weight and a with 1.0 - weight
    inline qreal linearInterpolation(qreal a, qreal b, qreal weight) const {
        return (1.0 - weight) * a + weight * b;
    }

    // TODO: move this somewhere where I can reuse it
    /// convert radians to degrees
    inline qreal rad2deg(qreal rad) const {
        return rad * (180.0/M_PI);
    }

    /// convert degrees to radians
    inline qreal deg2rad(quint16 deg) const {
        return deg * (M_PI/180.0);
    }
};

#endif
