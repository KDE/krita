/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#include <stdlib.h>
#include <string.h>
#include <cfloat>

#include "qbrush.h"
#include "qcolor.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpen.h"
#include "qregion.h"
#include "qmatrix.h"
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <q3pointarray.h>
#include <QRect>
#include <QString>

#include <kdebug.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_debug_areas.h"
#include "kis_gradient.h"
#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "kis_colorspace.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_gradient_painter.h"
#include "kis_meta_registry.h"
#include "kis_colorspace_factory_registry.h"

namespace {

    class GradientShapeStrategy {
    public:
        GradientShapeStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);
        virtual ~GradientShapeStrategy() {}

        virtual double valueAt(double x, double y) const = 0;

    protected:
        KisPoint m_gradientVectorStart;
        KisPoint m_gradientVectorEnd;
    };

    GradientShapeStrategy::GradientShapeStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
        : m_gradientVectorStart(gradientVectorStart), m_gradientVectorEnd(gradientVectorEnd)
    {
    }


    class LinearGradientStrategy : public GradientShapeStrategy {
        typedef GradientShapeStrategy super;
    public:
        LinearGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

        virtual double valueAt(double x, double y) const;

    protected:
        double m_normalisedVectorX;
        double m_normalisedVectorY;
        double m_vectorLength;
    };

    LinearGradientStrategy::LinearGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
        : super(gradientVectorStart, gradientVectorEnd)
    {
        double dx = gradientVectorEnd.x() - gradientVectorStart.x();
        double dy = gradientVectorEnd.y() - gradientVectorStart.y();

        m_vectorLength = sqrt((dx * dx) + (dy * dy));

        if (m_vectorLength < DBL_EPSILON) {
            m_normalisedVectorX = 0;
            m_normalisedVectorY = 0;
        }
        else {
            m_normalisedVectorX = dx / m_vectorLength;
            m_normalisedVectorY = dy / m_vectorLength;
        }
    }

    double LinearGradientStrategy::valueAt(double x, double y) const
    {
        double vx = x - m_gradientVectorStart.x();
        double vy = y - m_gradientVectorStart.y();

        // Project the vector onto the normalised gradient vector.
        double t = vx * m_normalisedVectorX + vy * m_normalisedVectorY;

        if (m_vectorLength < DBL_EPSILON) {
            t = 0;
        }
        else {
            // Scale to 0 to 1 over the gradient vector length.
            t /= m_vectorLength;
        }

        return t;
    }


    class BiLinearGradientStrategy : public LinearGradientStrategy {
        typedef LinearGradientStrategy super;
    public:
        BiLinearGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

        virtual double valueAt(double x, double y) const;
    };

    BiLinearGradientStrategy::BiLinearGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
        : super(gradientVectorStart, gradientVectorEnd)
    {
    }

    double BiLinearGradientStrategy::valueAt(double x, double y) const
    {
        double t = super::valueAt(x, y);

        // Reflect
        if (t < -DBL_EPSILON) {
            t = -t;
        }

        return t;
    }


    class RadialGradientStrategy : public GradientShapeStrategy {
        typedef GradientShapeStrategy super;
    public:
        RadialGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

        virtual double valueAt(double x, double y) const;

    protected:
        double m_radius;
    };

    RadialGradientStrategy::RadialGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
        : super(gradientVectorStart, gradientVectorEnd)
    {
        double dx = gradientVectorEnd.x() - gradientVectorStart.x();
        double dy = gradientVectorEnd.y() - gradientVectorStart.y();

        m_radius = sqrt((dx * dx) + (dy * dy));
    }

    double RadialGradientStrategy::valueAt(double x, double y) const
    {
        double dx = x - m_gradientVectorStart.x();
        double dy = y - m_gradientVectorStart.y();

        double distance = sqrt((dx * dx) + (dy * dy));

        double t;

        if (m_radius < DBL_EPSILON) {
            t = 0;
        }
        else {
            t = distance / m_radius;
        }

        return t;
    }


    class SquareGradientStrategy : public GradientShapeStrategy {
        typedef GradientShapeStrategy super;
    public:
        SquareGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

        virtual double valueAt(double x, double y) const;

    protected:
        double m_normalisedVectorX;
        double m_normalisedVectorY;
        double m_vectorLength;
    };

    SquareGradientStrategy::SquareGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
        : super(gradientVectorStart, gradientVectorEnd)
    {
        double dx = gradientVectorEnd.x() - gradientVectorStart.x();
        double dy = gradientVectorEnd.y() - gradientVectorStart.y();

        m_vectorLength = sqrt((dx * dx) + (dy * dy));

        if (m_vectorLength < DBL_EPSILON) {
            m_normalisedVectorX = 0;
            m_normalisedVectorY = 0;
        }
        else {
            m_normalisedVectorX = dx / m_vectorLength;
            m_normalisedVectorY = dy / m_vectorLength;
        }
    }

    double SquareGradientStrategy::valueAt(double x, double y) const
    {
        double px = x - m_gradientVectorStart.x();
        double py = y - m_gradientVectorStart.y();

        double distance1 = 0;
        double distance2 = 0;

        if (m_vectorLength > DBL_EPSILON) {

            // Point to line distance is:
            // distance = ((l0.y() - l1.y()) * p.x() + (l1.x() - l0.x()) * p.y() + l0.x() * l1.y() - l1.x() * l0.y()) / m_vectorLength;
            //
            // Here l0 = (0, 0) and |l1 - l0| = 1

            distance1 = -m_normalisedVectorY * px + m_normalisedVectorX * py;
            distance1 = fabs(distance1);

            // Rotate point by 90 degrees and get the distance to the perpendicular
            distance2 = -m_normalisedVectorY * -py + m_normalisedVectorX * px;
            distance2 = fabs(distance2);
        }

        double t = qMax(distance1, distance2) / m_vectorLength;

        return t;
    }


    class ConicalGradientStrategy : public GradientShapeStrategy {
        typedef GradientShapeStrategy super;
    public:
        ConicalGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

        virtual double valueAt(double x, double y) const;

    protected:
        double m_vectorAngle;
    };

    ConicalGradientStrategy::ConicalGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
        : super(gradientVectorStart, gradientVectorEnd)
    {
        double dx = gradientVectorEnd.x() - gradientVectorStart.x();
        double dy = gradientVectorEnd.y() - gradientVectorStart.y();

        // Get angle from 0 to 2 PI.
        m_vectorAngle = atan2(dy, dx) + M_PI;
    }

    double ConicalGradientStrategy::valueAt(double x, double y) const
    {
        double px = x - m_gradientVectorStart.x();
        double py = y - m_gradientVectorStart.y();

        double angle = atan2(py, px) + M_PI;

        angle -= m_vectorAngle;

        if (angle < 0) {
            angle += 2 * M_PI;
        }

        double t = angle / (2 * M_PI);

        return t;
    }


    class ConicalSymetricGradientStrategy : public GradientShapeStrategy {
        typedef GradientShapeStrategy super;
    public:
        ConicalSymetricGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd);

        virtual double valueAt(double x, double y) const;

    protected:
        double m_vectorAngle;
    };

    ConicalSymetricGradientStrategy::ConicalSymetricGradientStrategy(const KisPoint& gradientVectorStart, const KisPoint& gradientVectorEnd)
        : super(gradientVectorStart, gradientVectorEnd)
    {
        double dx = gradientVectorEnd.x() - gradientVectorStart.x();
        double dy = gradientVectorEnd.y() - gradientVectorStart.y();

        // Get angle from 0 to 2 PI.
        m_vectorAngle = atan2(dy, dx) + M_PI;
    }

    double ConicalSymetricGradientStrategy::valueAt(double x, double y) const
    {
        double px = x - m_gradientVectorStart.x();
        double py = y - m_gradientVectorStart.y();

        double angle = atan2(py, px) + M_PI;

        angle -= m_vectorAngle;

        if (angle < 0) {
            angle += 2 * M_PI;
        }

        double t;

        if (angle < M_PI) {
            t = angle / M_PI;
        }
        else {
            t = 1 - ((angle - M_PI) / M_PI);
        }

        return t;
    }


    class GradientRepeatStrategy {
    public:
        GradientRepeatStrategy() {}
        virtual ~GradientRepeatStrategy() {}

        virtual double valueAt(double t) const = 0;
    };


    class GradientRepeatNoneStrategy : public GradientRepeatStrategy {
    public:
        static GradientRepeatNoneStrategy *instance();

        virtual double valueAt(double t) const;

    private:
        GradientRepeatNoneStrategy() {}

        static GradientRepeatNoneStrategy *m_instance;
    };

    GradientRepeatNoneStrategy *GradientRepeatNoneStrategy::m_instance = 0;

    GradientRepeatNoneStrategy *GradientRepeatNoneStrategy::instance()
    {
        if (m_instance == 0) {
            m_instance = new GradientRepeatNoneStrategy();
            Q_CHECK_PTR(m_instance);
        }

        return m_instance;
    }

    // Output is clamped to 0 to 1.
    double GradientRepeatNoneStrategy::valueAt(double t) const
    {
        double value = t;

        if (t < DBL_EPSILON) {
            value = 0;
        }
        else
            if (t > 1 - DBL_EPSILON) {
                value = 1;
            }

        return value;
    }


    class GradientRepeatForwardsStrategy : public GradientRepeatStrategy {
    public:
        static GradientRepeatForwardsStrategy *instance();

        virtual double valueAt(double t) const;

    private:
        GradientRepeatForwardsStrategy() {}

        static GradientRepeatForwardsStrategy *m_instance;
    };

    GradientRepeatForwardsStrategy *GradientRepeatForwardsStrategy::m_instance = 0;

    GradientRepeatForwardsStrategy *GradientRepeatForwardsStrategy::instance()
    {
        if (m_instance == 0) {
            m_instance = new GradientRepeatForwardsStrategy();
            Q_CHECK_PTR(m_instance);
        }

        return m_instance;
    }

    // Output is 0 to 1, 0 to 1, 0 to 1...
    double GradientRepeatForwardsStrategy::valueAt(double t) const
    {
        int i = static_cast<int>(t);

        if (t < DBL_EPSILON) {
            i--;
        }

        double value = t - i;

        return value;
    }


    class GradientRepeatAlternateStrategy : public GradientRepeatStrategy {
    public:
        static GradientRepeatAlternateStrategy *instance();

        virtual double valueAt(double t) const;

    private:
        GradientRepeatAlternateStrategy() {}

        static GradientRepeatAlternateStrategy *m_instance;
    };

    GradientRepeatAlternateStrategy *GradientRepeatAlternateStrategy::m_instance = 0;

    GradientRepeatAlternateStrategy *GradientRepeatAlternateStrategy::instance()
    {
        if (m_instance == 0) {
            m_instance = new GradientRepeatAlternateStrategy();
            Q_CHECK_PTR(m_instance);
        }

        return m_instance;
    }

    // Output is 0 to 1, 1 to 0, 0 to 1, 1 to 0...
    double GradientRepeatAlternateStrategy::valueAt(double t) const
    {
        if (t < 0) {
            t = -t;
        }

        int i = static_cast<int>(t);

                double value = t - i;

        if (i % 2 == 1) {
            value = 1 - value;
        }

        return value;
    }
}

KisGradientPainter::KisGradientPainter()
    : super()
{
    m_gradient = 0;
}

KisGradientPainter::KisGradientPainter(KisPaintDeviceSP device) : super(device), m_gradient(0)
{
}

bool KisGradientPainter::paintGradient(const KisPoint& gradientVectorStart,
                       const KisPoint& gradientVectorEnd,
                       enumGradientShape shape,
                       enumGradientRepeat repeat,
                       double antiAliasThreshold,
                       bool reverseGradient,
                       qint32 startx,
                       qint32 starty,
                       qint32 width,
                       qint32 height)
{
    m_cancelRequested = false;

    if (!m_gradient) return false;

    GradientShapeStrategy *shapeStrategy = 0;

    switch (shape) {
    case GradientShapeLinear:
        shapeStrategy = new LinearGradientStrategy(gradientVectorStart, gradientVectorEnd);
        break;
    case GradientShapeBiLinear:
        shapeStrategy = new BiLinearGradientStrategy(gradientVectorStart, gradientVectorEnd);
        break;
    case GradientShapeRadial:
        shapeStrategy = new RadialGradientStrategy(gradientVectorStart, gradientVectorEnd);
        break;
    case GradientShapeSquare:
        shapeStrategy = new SquareGradientStrategy(gradientVectorStart, gradientVectorEnd);
        break;
    case GradientShapeConical:
        shapeStrategy = new ConicalGradientStrategy(gradientVectorStart, gradientVectorEnd);
        break;
    case GradientShapeConicalSymetric:
        shapeStrategy = new ConicalSymetricGradientStrategy(gradientVectorStart, gradientVectorEnd);
        break;
    }
    Q_CHECK_PTR(shapeStrategy);

    GradientRepeatStrategy *repeatStrategy = 0;

    switch (repeat) {
    case GradientRepeatNone:
        repeatStrategy = GradientRepeatNoneStrategy::instance();
        break;
    case GradientRepeatForwards:
        repeatStrategy = GradientRepeatForwardsStrategy::instance();
        break;
    case GradientRepeatAlternate:
        repeatStrategy = GradientRepeatAlternateStrategy::instance();
        break;
    }
    Q_ASSERT(repeatStrategy != 0);


    //If the device has a selection only iterate over that selection
    QRect r;
    if( m_device->hasSelection() ) {
        r = m_device->selection()->selectedExactRect();
        startx = r.x();
        starty = r.y();
        width = r.width();
        height = r.height();
    }

    qint32 endx = startx + width - 1;
    qint32 endy = starty + height - 1;

    QImage layer(width, height, QImage::Format_ARGB32);

    int pixelsProcessed = 0;
    int lastProgressPercent = 0;

    emit notifyProgressStage(i18n("Rendering gradient..."), 0);

    int totalPixels = width * height;

    if (antiAliasThreshold < 1 - DBL_EPSILON) {
        totalPixels *= 2;
    }

    for (int y = starty; y <= endy; y++) {
        for (int x = startx; x <= endx; x++) {

            double t = shapeStrategy->valueAt( x, y);
            t = repeatStrategy->valueAt(t);

            if (reverseGradient) {
                t = 1 - t;
            }

            QColor color;
            quint8 opacity;

            m_gradient->colorAt(t, &color, &opacity);

            layer.setPixel(x - startx, y - starty,
                           qRgba(color.red(), color.green(), color.blue(), opacity));

            pixelsProcessed++;

            int progressPercent = (pixelsProcessed * 100) / totalPixels;

            if (progressPercent > lastProgressPercent) {
                emit notifyProgress(progressPercent);
                lastProgressPercent = progressPercent;

                if (m_cancelRequested) {
                    break;
                }
            }
            if (m_cancelRequested) {
                break;
            }
        }
    }

    if (!m_cancelRequested && antiAliasThreshold < 1 - DBL_EPSILON) {

        emit notifyProgressStage(i18n("Anti-aliasing gradient..."), lastProgressPercent);
        quint8 * layerPointer = layer.bits();
        for (int y = starty; y <= endy; y++) {
            for (int x = startx; x <= endx; x++) {

                double maxDistance = 0;

                QColor thisPixel(layerPointer[2], layerPointer[1], layerPointer[0]);
                quint8 thisPixelOpacity = layerPointer[3];

                for (int yOffset = -1; yOffset < 2; yOffset++) {
                    for (int xOffset = -1; xOffset < 2; xOffset++) {

                        if (xOffset != 0 || yOffset != 0) {
                            int sampleX = x + xOffset;
                            int sampleY = y + yOffset;

                            if (sampleX >= startx && sampleX <= endx && sampleY >= starty && sampleY <= endy) {
                                uint x = sampleX - startx;
                                uint y = sampleY - starty;
                                quint8 * pixelPos = layer.bits() + (y * width * 4) + (x * 4);
                                QColor color(*(pixelPos +2), *(pixelPos + 1), *pixelPos);
                                quint8 opacity = *(pixelPos + 3);

                                double dRed = (color.red() * opacity - thisPixel.red() * thisPixelOpacity) / 65535.0;
                                double dGreen = (color.green() * opacity - thisPixel.green() * thisPixelOpacity) / 65535.0;
                                double dBlue = (color.blue() * opacity - thisPixel.blue() * thisPixelOpacity) / 65535.0;

                                #define SQRT_3 1.7320508

                                double distance = sqrt(dRed * dRed + dGreen * dGreen + dBlue * dBlue) / SQRT_3;

                                if (distance > maxDistance) {
                                    maxDistance = distance;
                                }
                            }
                        }
                    }
                }

                if (maxDistance > antiAliasThreshold) {
                    const int numSamples = 4;

                    int totalRed = 0;
                    int totalGreen = 0;
                    int totalBlue = 0;
                    int totalOpacity = 0;

                    for (int ySample = 0; ySample < numSamples; ySample++) {
                        for (int xSample = 0; xSample < numSamples; xSample++) {

                            double sampleWidth = 1.0 / numSamples;

                            double sampleX = x - 0.5 + (sampleWidth / 2) + xSample * sampleWidth;
                            double sampleY = y - 0.5 + (sampleWidth / 2) + ySample * sampleWidth;

                            double t = shapeStrategy->valueAt(sampleX, sampleY);
                            t = repeatStrategy->valueAt(t);

                            if (reverseGradient) {
                                t = 1 - t;
                            }

                            QColor color;
                            quint8 opacity;

                            m_gradient->colorAt(t, &color, &opacity);

                            totalRed += color.red();
                            totalGreen += color.green();
                            totalBlue += color.blue();
                            totalOpacity += opacity;
                        }
                    }

                    int red = totalRed / (numSamples * numSamples);
                    int green = totalGreen / (numSamples * numSamples);
                    int blue = totalBlue / (numSamples * numSamples);
                    int opacity = totalOpacity / (numSamples * numSamples);

                    layer.setPixel(x - startx, y - starty, qRgba(red, green, blue, opacity));
                }

                pixelsProcessed++;

                int progressPercent = (pixelsProcessed * 100) / totalPixels;

                if (progressPercent > lastProgressPercent) {
                    emit notifyProgress(progressPercent);
                    lastProgressPercent = progressPercent;

                    if (m_cancelRequested) {
                        break;
                    }
                }
                layerPointer += 4;
            }

            if (m_cancelRequested) {
                break;
            }
        }
    }

    if (!m_cancelRequested) {
        kDebug() << "Have we got a selection? " << m_device->hasSelection() << endl;
        KisPaintDeviceSP dev = KisPaintDeviceSP(new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getRGB8(), "temporary device for gradient"));
        dev->writeBytes(layer.bits(), startx, starty, width, height);
        bltSelection(startx, starty, m_compositeOp, dev, m_opacity, startx, starty, width, height);
    }
    delete shapeStrategy;

    emit notifyProgressDone();

    return !m_cancelRequested;
}
