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

#ifndef _BRUSH_H_
#define _BRUSH_H_

#include <QVector>
#include <QList>
#include <QTransform>

#include <KoColor.h>

#include "bristle.h"
#include "brush_shape.h"

#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_random_accessor.h>

class KisSumiProperties{
public:
    quint16 radius;
    quint16 inkAmount;
    qreal sigma;
    QList<float> inkDepletionCurve;
    bool isbrushDimension1D;
    bool useMousePressure;
    bool useSaturation;
    bool useOpacity;
    bool useWeights;

    quint8 pressureWeight;
    quint8 bristleLengthWeight;
    quint8 bristleInkAmountWeight;
    quint8 inkDepletionWeight;

    qreal shearFactor;
    qreal randomFactor;
    qreal scaleFactor;
};

class Brush
{

public:
    Brush();
    ~Brush();
    void paintLine(KisPaintDeviceSP dev, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2);

    void repositionBristles(double angle, double slope);
    void rotateBristles(double angle);
    double computeMousePressure(double distance);
    double getAngleDelta(const KisPaintInformation& info);

    void setInkColor(const KoColor &color);
    void setBrushShape(BrushShape brushShape);
    void setProperties(KisSumiProperties * properties){ m_properties = properties; }
    
    /// paints single bristle
    void putBristle(Bristle *bristle, float wx, float wy, const KoColor &color);
    void mixCMY(double x, double y, int cyan, int magenta, int yellow, double weight);
    void addBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color);
    void oldAddBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color);

private:
    const KisSumiProperties * m_properties;
    
    QVector<Bristle> m_bristles;
    QTransform m_transform;

    BrushShape m_initialShape;
    KoColor m_inkColor;

    // temporary device
    KisPaintDeviceSP m_dev;
    KisRandomAccessor * m_dabAccessor;
    quint32 m_pixelSize;

    // painter()->device()
    KisPaintDeviceSP m_layer;
    KisRandomAccessor * m_layerAccessor;
    quint32 m_layerPixelSize;

    int m_counter;

    double m_lastAngle;
    double m_lastSlope;

    double m_angle;
    double m_oldPressure;
};

#endif
