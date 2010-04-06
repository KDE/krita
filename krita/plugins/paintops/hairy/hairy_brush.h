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

#ifndef _HAIRY_BRUSH_H_
#define _HAIRY_BRUSH_H_

#include <QVector>
#include <QList>
#include <QTransform>

#include <KoColor.h>

#include "trajectory.h"
#include "bristle.h"
#include "brush_shape.h"

#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_random_accessor.h>


class KoColorSpace;

class KisHairyProperties{
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

    bool useSoakInk;
    bool connectedPath;
    
    quint8 pressureWeight;
    quint8 bristleLengthWeight;
    quint8 bristleInkAmountWeight;
    quint8 inkDepletionWeight;

    qreal shearFactor;
    qreal randomFactor;
    qreal scaleFactor;
    qreal threshold;
    
};

class HairyBrush
{

public:
    HairyBrush();
    ~HairyBrush();
    void paintLine(KisPaintDeviceSP dev, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2, qreal scale);

    void repositionBristles(double angle, double slope);
    void rotateBristles(double angle);
    double computeMousePressure(double distance);

    void setInkColor(const KoColor &color);
    void setBrushShape(BrushShape brushShape);
    void setProperties(KisHairyProperties * properties){ m_properties = properties; }
    
    /// paints single bristle
    void putBristle(Bristle *bristle, float wx, float wy, const KoColor &color);
    void addBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color);
    void oldAddBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color);

    /// similar to sample input color in spray
    void colorifyBristles(KisRandomConstAccessor& acc, KoColorSpace * cs, QPointF point);
    
private:
    const KisHairyProperties * m_properties;
    
    QVector<Bristle*> m_bristles;
    QTransform m_transform;

    BrushShape m_initialShape;
   
    // used for interpolation the path of bristles
    Trajectory m_trajectory;
    QHash<QString, QVariant> m_params;    
    // temporary device
    KisPaintDeviceSP m_dev;
    KisRandomAccessor * m_dabAccessor;
    quint32 m_pixelSize;

    int m_counter;

    double m_lastAngle;
    double m_lastSlope;

    double m_angle;
    double m_oldPressure;
    
    // I use internal counter to count the calls of paint, the counter is 1 when the first call occurs
    inline bool firstStroke(){
        return (m_counter == 1);
    }
};

#endif
