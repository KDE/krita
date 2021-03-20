/*
 *  SPDX-FileCopyrightText: 2008-2010, 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _SPRAY_BRUSH_H_
#define _SPRAY_BRUSH_H_

#include <KoColor.h>

#include "kis_types.h"
#include "kis_painter.h"

#include <brushengine/kis_random_source.h>
#include "kis_color_option.h"
#include "kis_spray_shape_option.h"
#include "kis_spray_shape_dynamics.h"
#include "kis_sprayop_option.h"


#include <QImage>
#include <kis_brush.h>

class KisPaintInformation;

class SprayBrush
{

public:
    SprayBrush();
    ~SprayBrush();

    void paint(KisPaintDeviceSP dab, KisPaintDeviceSP source,  const KisPaintInformation& info, qreal rotation, qreal scale, qreal additionalScale, const KoColor &color, const KoColor &bgColor);
    void setProperties(KisSprayOptionProperties * properties,
                       KisColorProperties * colorProperties,
                       KisShapeProperties * shapeProperties,
                       KisShapeDynamicsProperties * shapeDynamicsProperties,
                       KisBrushSP brush);

    void setFixedDab(KisFixedPaintDeviceSP dab);

private:
    int m_dabSeqNo = 0;
    KoColor m_inkColor;
    qreal m_radius;
    quint32 m_particlesCount;
    quint8 m_dabPixelSize;

    KisPainter * m_painter;
    KisPaintDeviceSP m_imageDevice;
    QImage m_brushQImage;
    QImage m_transformed;

    KoColorTransformation* m_transfo;

    const KisSprayOptionProperties * m_properties;
    const KisColorProperties * m_colorProperties;
    const KisShapeProperties * m_shapeProperties;
    const KisShapeDynamicsProperties * m_shapeDynamicsProperties;

    KisBrushSP m_brush;
    KisFixedPaintDeviceSP m_fixedDab;

private:
    /// rotation in radians according the settings (gauss distribution, uniform distribution or fixed angle)
    qreal rotationAngle(KisRandomSourceSP randomSource);
    /// Paints Wu Particle
    void paintParticle(KisRandomAccessorSP &writeAccessor, const KoColor &color, qreal rx, qreal ry);
    void paintCircle(KisPainter * painter, qreal x, qreal y, qreal radius);
    void paintEllipse(KisPainter * painter, qreal x, qreal y, qreal a, qreal b, qreal angle);
    void paintRectangle(KisPainter * painter, qreal x, qreal y, qreal width, qreal height, qreal angle);

    void paintOutline(KisPaintDeviceSP dev, const KoColor& painterColor, qreal posX, qreal posY, qreal radius);

    /// mix a with b.b mix with weight and a with 1.0 - weight
    inline qreal linearInterpolation(qreal a, qreal b, qreal weight) const {
        return (1.0 - weight) * a + weight * b;
    }

    // TODO: move this somewhere where I can reuse it
    /// convert radians to degrees
    inline qreal rad2deg(qreal rad) const {
        return rad * (180.0 / M_PI);
    }

    /// convert degrees to radians
    inline qreal deg2rad(quint16 deg) const {
        return deg * (M_PI / 180.0);
    }
};

#endif
