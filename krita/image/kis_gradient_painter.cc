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

#include "kis_gradient_painter.h"

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
#include <QRect>
#include <QString>

#include <klocale.h>

#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoAbstractGradient.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_pattern.h"
#include "kis_types.h"
#include "kis_random_accessor.h"
#include "kis_selection.h"


#include "KoColorSpaceRegistry.h"

namespace
{

class GradientShapeStrategy
{
public:
    GradientShapeStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);
    virtual ~GradientShapeStrategy() {}

    virtual double valueAt(double x, double y) const = 0;

protected:
    QPointF m_gradientVectorStart;
    QPointF m_gradientVectorEnd;
};

GradientShapeStrategy::GradientShapeStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : m_gradientVectorStart(gradientVectorStart), m_gradientVectorEnd(gradientVectorEnd)
{
}


class LinearGradientStrategy : public GradientShapeStrategy
{

public:
    LinearGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    virtual double valueAt(double x, double y) const;

protected:
    double m_normalisedVectorX;
    double m_normalisedVectorY;
    double m_vectorLength;
};

LinearGradientStrategy::LinearGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : GradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
{
    double dx = gradientVectorEnd.x() - gradientVectorStart.x();
    double dy = gradientVectorEnd.y() - gradientVectorStart.y();

    m_vectorLength = sqrt((dx * dx) + (dy * dy));

    if (m_vectorLength < DBL_EPSILON) {
        m_normalisedVectorX = 0;
        m_normalisedVectorY = 0;
    } else {
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
    } else {
        // Scale to 0 to 1 over the gradient vector length.
        t /= m_vectorLength;
    }

    return t;
}


class BiLinearGradientStrategy : public LinearGradientStrategy
{

public:
    BiLinearGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    virtual double valueAt(double x, double y) const;
};

BiLinearGradientStrategy::BiLinearGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : LinearGradientStrategy(gradientVectorStart, gradientVectorEnd)
{
}

double BiLinearGradientStrategy::valueAt(double x, double y) const
{
    double t = LinearGradientStrategy::valueAt(x, y);

    // Reflect
    if (t < -DBL_EPSILON) {
        t = -t;
    }

    return t;
}


class RadialGradientStrategy : public GradientShapeStrategy
{

public:
    RadialGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    virtual double valueAt(double x, double y) const;

protected:
    double m_radius;
};

RadialGradientStrategy::RadialGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : GradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
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
    } else {
        t = distance / m_radius;
    }

    return t;
}


class SquareGradientStrategy : public GradientShapeStrategy
{

public:
    SquareGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    virtual double valueAt(double x, double y) const;

protected:
    double m_normalisedVectorX;
    double m_normalisedVectorY;
    double m_vectorLength;
};

SquareGradientStrategy::SquareGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : GradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
{
    double dx = gradientVectorEnd.x() - gradientVectorStart.x();
    double dy = gradientVectorEnd.y() - gradientVectorStart.y();

    m_vectorLength = sqrt((dx * dx) + (dy * dy));

    if (m_vectorLength < DBL_EPSILON) {
        m_normalisedVectorX = 0;
        m_normalisedVectorY = 0;
    } else {
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


class ConicalGradientStrategy : public GradientShapeStrategy
{

public:
    ConicalGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    virtual double valueAt(double x, double y) const;

protected:
    double m_vectorAngle;
};

ConicalGradientStrategy::ConicalGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : GradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
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


class ConicalSymetricGradientStrategy : public GradientShapeStrategy
{
public:
    ConicalSymetricGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    virtual double valueAt(double x, double y) const;

protected:
    double m_vectorAngle;
};

ConicalSymetricGradientStrategy::ConicalSymetricGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : GradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
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
    } else {
        t = 1 - ((angle - M_PI) / M_PI);
    }

    return t;
}


class GradientRepeatStrategy
{
public:
    GradientRepeatStrategy() {}
    virtual ~GradientRepeatStrategy() {}

    virtual double valueAt(double t) const = 0;
};


class GradientRepeatNoneStrategy : public GradientRepeatStrategy
{
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
    } else if (t > 1 - DBL_EPSILON) {
        value = 1;
    }

    return value;
}


class GradientRepeatForwardsStrategy : public GradientRepeatStrategy
{
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


class GradientRepeatAlternateStrategy : public GradientRepeatStrategy
{
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
        : KisPainter()
{
}

KisGradientPainter::KisGradientPainter(KisPaintDeviceSP device)
        : KisPainter(device)
{
}

KisGradientPainter::KisGradientPainter(KisPaintDeviceSP device, KisSelectionSP selection)
        : KisPainter(device, selection)
{
}

bool KisGradientPainter::paintGradient(const QPointF& gradientVectorStart,
                                       const QPointF& gradientVectorEnd,
                                       enumGradientShape shape,
                                       enumGradientRepeat repeat,
                                       double antiAliasThreshold,
                                       bool reverseGradient,
                                       qint32 startx,
                                       qint32 starty,
                                       qint32 width,
                                       qint32 height)
{
    if (!gradient()) return false;

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


    //If the device has a selection only iterate over that selection united with our area of interest
    QRect r;
    QRect r2(startx, starty, width, height);
    if (selection()) {
        r = selection()->selectedExactRect();
        r2 &= r;
    }
    startx = r2.x();
    starty = r2.y();
    width = r2.width();
    height = r2.height();

    qint32 endx = startx + width - 1;
    qint32 endy = starty + height - 1;

    int linesProcessed = 0;
    int lastProgressPercent = 0;

    if (progressUpdater()) progressUpdater()->setProgress(0);

    int totalPixels = width * height;
    if (antiAliasThreshold < 1 - DBL_EPSILON) {
        totalPixels *= 2;
    }

    KisPaintDeviceSP dev = KisPaintDeviceSP(new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8()));

    int pixelsProcessed = 0;

    const KoColorSpace * colorSpace = dev->colorSpace();
    KoColor color(colorSpace);
    qint32 pixelSize = colorSpace->pixelSize();
    KisHLineIteratorPixel hit = dev->createHLineIterator(startx, starty, width);
    for (int y = starty; y <= endy; y++) {

        for (int x = startx; x <= endx; x++) {

            double t = shapeStrategy->valueAt(x, y);
            t = repeatStrategy->valueAt(t);

            if (reverseGradient) {
                t = 1 - t;
            }

            gradient()->colorAt(color, t);
            memcpy(hit.rawData(), color.data(), pixelSize);

            ++hit;
        }
        hit.nextRow();

        linesProcessed++;

        int progressPercent = (linesProcessed * 100) / height;

        if (progressPercent > lastProgressPercent) {
            if (progressUpdater()) progressUpdater()->setProgress(progressPercent);
            lastProgressPercent = progressPercent;

            if (progressUpdater() && progressUpdater()->interrupted()) {
                break;
            }
        }

        if (progressUpdater() && progressUpdater()->interrupted()) {
            break;
        }
    }

    if (!progressUpdater() || (progressUpdater() && !progressUpdater()->interrupted())) {
        // XXX: FIX ANTIALISED DRAWING OF GRADIENTS
        if (false && antiAliasThreshold < 1 - DBL_EPSILON) {

            QList<KoChannelInfo *> channels = colorSpace->channels();
            KisHLineIterator iter = dev->createHLineIterator(0, 0, 0);
            KisRandomAccessor accessor = dev->createRandomAccessor(0, 0, 0);
            double squareRootNumColorChannels = sqrt(static_cast<double>(colorSpace->colorChannelCount()));

            for (int y = starty; y <= endy; y++) {
                for (int x = startx; x <= endx; x++) {

                    double maxDistance = 0;
                    quint8* thisPixel = colorSpace->allocPixelBuffer(1);
                    memcpy(thisPixel, iter.rawData(), pixelSize);
                    quint8 thisPixelOpacity = colorSpace->alpha(thisPixel);

                    // XXX: Move the distance computation into the colorspace when it works.
                    for (int yOffset = -1; yOffset < 2; yOffset++) {
                        for (int xOffset = -1; xOffset < 2; xOffset++) {

                            if (xOffset != 0 || yOffset != 0) {
                                int sampleX = x + xOffset;
                                int sampleY = y + yOffset;

                                if (sampleX >= startx && sampleX <= endx && sampleY >= starty && sampleY <= endy) {
                                    uint x = sampleX - startx;
                                    uint y = sampleY - starty;

                                    accessor.moveTo(x, y);
                                    quint8 * pixel = accessor.rawData();
                                    quint8 opacity = colorSpace->alpha(pixel);

                                    double totalDistance = 0;

                                    foreach(KoChannelInfo * channel, channels) {
                                        double d = 0;
                                        int pos = channel->pos();
                                        if (channel->channelType() == KoChannelInfo::COLOR) {
                                            if (channel->channelValueType() == KoChannelInfo::UINT8) {
                                                d = ((quint8) * (pixel + pos) * opacity - (quint8) * (thisPixel + pos) * thisPixelOpacity) / 65535.0;
                                            } else if (channel->channelValueType() ==  KoChannelInfo::UINT16) {
                                                d = ((quint16) * (pixel + pos) * opacity - (quint16) * (thisPixel + pos) * thisPixelOpacity) / 65535.0;
                                            } else if (channel->channelValueType() == KoChannelInfo::UINT32) {
                                                d = ((quint32) * (pixel + pos) * opacity - (quint32) * (thisPixel + pos) * thisPixelOpacity) / 65535.0;
#if 0 // Check how to use the half datatype
                                            } else if (channel->channelValueType() == KoChannelInfo::FLOAT16) {
                                                d = ((quint8) * (pixel + pos) * opacity - (quint8) * (thisPixel + pos) * thisPixelOpacity) / 65535.0;
#endif
                                            } else if (channel->channelValueType() == KoChannelInfo::FLOAT32) {
                                                d = ((float) * (pixel + pos) * opacity - (float) * (thisPixel + pos) * thisPixelOpacity) / 65535.0;
                                            } else if (channel->channelValueType() == KoChannelInfo::FLOAT64) {
                                                d = ((double) * (pixel + pos) * opacity - (double) * (thisPixel + pos) * thisPixelOpacity) / 65535.0;
                                            } else if (channel->channelValueType() == KoChannelInfo::INT8) {
                                                d = ((qint8) * (pixel + pos) * opacity - (qint8) * (thisPixel + pos) * thisPixelOpacity) / 65535.0;
                                            } else if (channel->channelValueType() == KoChannelInfo::INT16) {
                                                d = ((qint16) * (pixel + pos) * opacity - (qint16) * (thisPixel + pos) * thisPixelOpacity) / 65535.0;
                                            }

                                        }
                                        totalDistance += d * d;
                                    }
                                    double distance = sqrt(totalDistance) / squareRootNumColorChannels;

                                    if (distance > maxDistance) {
                                        maxDistance = distance;
                                    }
                                }
                            }
                        }
                    }

                    // XXX: Move this averaging code into the colorspace
                    if (maxDistance > 3. * antiAliasThreshold * antiAliasThreshold) {
                        const int numSamples = 4;

                        QVector<quint32> channelTotals(colorSpace->channelCount());
                        for (uint i = 0; i < colorSpace->channelCount(); ++i) {
                            channelTotals[i] = 0;
                        }
                        quint32 totalOpacity = 0;

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

                                KoColor color;
                                gradient()->colorAt(color, t);

                                foreach(KoChannelInfo * channel, channels) {

                                    int pos = channel->pos();
                                    if (channel->channelType() == KoChannelInfo::COLOR) {
                                        if (channel->channelValueType() == KoChannelInfo::UINT8) {
                                            channelTotals[pos] += (quint8) * (color.data() + pos);
                                        } else if (channel->channelValueType() ==  KoChannelInfo::UINT16) {
                                            channelTotals[pos] += (quint16) * (color.data() + pos);
                                        } else if (channel->channelValueType() == KoChannelInfo::UINT32) {
                                            channelTotals[pos] += (quint32) * (color.data() + pos);
#if 0 // Check how to use the half datatype
                                        } else if (channel->channelValueType() == KoChannelInfo::FLOAT16) {
                                            channelTotals[pos] += (quint8)(color.data() + pos);
#endif
                                        } else if (channel->channelValueType() == KoChannelInfo::FLOAT32) {
                                            channelTotals[pos] += (float) * (color.data() + pos);
                                        } else if (channel->channelValueType() == KoChannelInfo::FLOAT64) {
                                            channelTotals[pos] += (double) * (color.data() + pos);
                                        } else if (channel->channelValueType() == KoChannelInfo::INT8) {
                                            channelTotals[pos] += (qint8) * (color.data() + pos);
                                        } else if (channel->channelValueType() == KoChannelInfo::INT16) {
                                            channelTotals[pos] += (qint16) * (color.data() + pos);
                                        }
                                    }
                                }
                                totalOpacity += colorSpace->alpha(color.data());
                            }
                        }
                        accessor.moveTo(x - startx, y - starty);
                        quint8 * pixel = accessor.rawData();
                        foreach(KoChannelInfo * channel, channels) {
                            if (channel->channelType() == KoChannelInfo::COLOR) {
                                int pos = channel->pos();
                                quint32 val = channelTotals[pos] / (numSamples * numSamples);
                                memcpy(pixel + pos, &val, channel->size());
                            }
                        }
                        colorSpace->setAlpha(pixel, totalOpacity, 1);
                    }

                    pixelsProcessed++;

                    int progressPercent = (pixelsProcessed * 100) / totalPixels;

                    if (progressPercent > lastProgressPercent) {
                        if (progressUpdater()) progressUpdater()->setProgress(progressPercent);
                        lastProgressPercent = progressPercent;

                        if (progressUpdater() && progressUpdater()->interrupted()) {
                            break;
                        }
                    }
                    ++iter;
                }
                iter.nextRow();
                if (progressUpdater() && progressUpdater()->interrupted()) {
                    break;
                }
            }
        }
    }

    if (!progressUpdater() || (progressUpdater() && !progressUpdater()->interrupted())) {
        bitBlt(startx, starty, dev, startx, starty, width, height);
    }
    delete shapeStrategy;

    if (progressUpdater()) {
        progressUpdater()->setProgress(100);
        return !progressUpdater()->interrupted();
    } else {

        return true;
    }
}
