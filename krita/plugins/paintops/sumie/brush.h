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

class Brush
{

public:
    Brush(const BrushShape &initialShape, const KoColor &inkColor);
    Brush();
    ~Brush();
    void paint(KisPaintDeviceSP dev, const KisPaintInformation &info);
    void paintLine(KisPaintDeviceSP dev, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2);
    void setInkDepletion(const QList<float>& curveData);
    void setInkColor(const KoColor &color);

    void repositionBristles(double angle, double slope);
    void rotateBristles(double angle);

    double getAngleDelta(const KisPaintInformation& info);

    void setRadius(int radius);
    void setSigma(double sigma);
    void setBrushShape(BrushShape brushShape);
    double computeMousePressure(double distance);
    void enableMousePressure(bool enable);

    void setShear(double shearFactor) {
        m_shearFactor = shearFactor;
    }
    void setRandom(double randomFactor) {
        m_randomFactor = randomFactor;
    }
    void setScale(double scaleFactor) {
        m_scaleFactor = scaleFactor;
    }

    void enableWeights(bool useWeights) {
        m_useWeights = useWeights;
    }
    void enableSaturation(bool useSaturation) {
        m_useSaturation = useSaturation;
    }
    void enableOpacity(bool useOpacity) {
        m_useOpacity = useOpacity;
    }


    void setPressureWeight(double pressureWeight) {
        m_pressureWeight = pressureWeight;
    }
    void setBristleLengthWeight(double bristleLengthWeight) {
        m_bristleLengthWeight = bristleLengthWeight;
    }
    void setBristleInkAmountWeight(double bristleInkAmountWeight) {
        m_bristleInkAmountWeight = bristleInkAmountWeight;
    }
    void setInkDepletionWeight(double inkDepletionWeight) {
        m_inkDepletionWeight = inkDepletionWeight;
    }

    /// paints single bristle
    void putBristle(Bristle *bristle, float wx, float wy, const KoColor &color);
    void mixCMY(double x, double y, int cyan, int magenta, int yellow, double weight);
    void addBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color);
    void oldAddBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color);
    void initDefaultValues();

private:
    QVector<Bristle> m_bristles;
    QList<float> m_inkDepletion; // array
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

    int m_radius;
    double m_sigma;

    double m_lastAngle;
    double m_lastSlope;

    double m_angle;
    double m_oldPressure;

    bool m_mousePressureEnabled;

    double m_scaleFactor;
    double m_randomFactor;
    double m_shearFactor;

    double m_pressureWeight;
    double m_bristleLengthWeight;
    double m_bristleInkAmountWeight;
    double m_inkDepletionWeight;

    bool m_useWeights;
    bool m_useSaturation;
    bool m_useOpacity;
};

#endif
