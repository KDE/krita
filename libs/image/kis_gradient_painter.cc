/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2019 Miguel Lopez <reptillia39@live.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_gradient_painter.h"

#include <algorithm>
#include <cfloat>

#include <KoColorSpace.h>
#include <resources/KoAbstractGradient.h>
#include <KoUpdater.h>
#include <KoEphemeralResource.h>

#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include "kis_global.h"
#include "kis_paint_device.h"
#include <resources/KoPattern.h>
#include "kis_selection.h"

#include <KisSequentialIteratorProgress.h>
#include "kis_image.h"
#include "kis_random_accessor_ng.h"
#include "kis_gradient_shape_strategy.h"
#include "kis_polygonal_gradient_shape_strategy.h"
#include "kis_cached_gradient_shape_strategy.h"
#include "krita_utils.h"
#include "KoMixColorsOp.h"
#include <KisDitherOp.h>
#include <KoCachedGradient.h>

namespace
{

class LinearGradientStrategy : public KisGradientShapeStrategy
{

public:
    LinearGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    double valueAt(double x, double y) const override;

protected:
    double m_normalisedVectorX;
    double m_normalisedVectorY;
    double m_vectorLength;
};

LinearGradientStrategy::LinearGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : KisGradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
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

    double valueAt(double x, double y) const override;
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


class RadialGradientStrategy : public KisGradientShapeStrategy
{

public:
    RadialGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    double valueAt(double x, double y) const override;

protected:
    double m_radius;
};

RadialGradientStrategy::RadialGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : KisGradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
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


class SquareGradientStrategy : public KisGradientShapeStrategy
{

public:
    SquareGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    double valueAt(double x, double y) const override;

protected:
    double m_normalisedVectorX;
    double m_normalisedVectorY;
    double m_vectorLength;
};

SquareGradientStrategy::SquareGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : KisGradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
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

    double t;

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
        
        t = qMax(distance1, distance2) / m_vectorLength;
    } else {
        t = 0;
    }

    return t;
}


class ConicalGradientStrategy : public KisGradientShapeStrategy
{

public:
    ConicalGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    double valueAt(double x, double y) const override;

protected:
    double m_vectorAngle;
};

ConicalGradientStrategy::ConicalGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : KisGradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
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


class ConicalSymetricGradientStrategy : public KisGradientShapeStrategy
{
public:
    ConicalSymetricGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

    double valueAt(double x, double y) const override;

protected:
    double m_vectorAngle;
};

ConicalSymetricGradientStrategy::ConicalSymetricGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
        : KisGradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
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

class SpiralGradientStrategy : public KisGradientShapeStrategy
{
public:
   SpiralGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

   double valueAt(double x, double y) const override;

protected:
   double m_vectorAngle;
    double m_radius;
};

SpiralGradientStrategy::SpiralGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
       : KisGradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
{
    double dx = gradientVectorEnd.x() - gradientVectorStart.x();
    double dy = gradientVectorEnd.y() - gradientVectorStart.y();

    // Get angle from 0 to 2 PI.
    m_vectorAngle = atan2(dy, dx) + M_PI;
    m_radius = sqrt((dx * dx) + (dy * dy));
};

double SpiralGradientStrategy::valueAt(double x, double y) const
{
    double dx = x - m_gradientVectorStart.x();
    double dy = y - m_gradientVectorStart.y();

    double distance = sqrt((dx * dx) + (dy * dy));
    double angle = atan2(dy, dx) + M_PI;

    double t;
    angle -= m_vectorAngle;

    if (m_radius < DBL_EPSILON) {
        t = 0;
    } else {
        t = distance / m_radius;
    }

    if (angle < 0) {
        angle += 2 * M_PI;
    }

    t += angle / (2 * M_PI);

    return t;

};

class ReverseSpiralGradientStrategy : public KisGradientShapeStrategy
{
public:
   ReverseSpiralGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);

   double valueAt(double x, double y) const override;

protected:
   double m_vectorAngle;
    double m_radius;
};

ReverseSpiralGradientStrategy::ReverseSpiralGradientStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
       : KisGradientShapeStrategy(gradientVectorStart, gradientVectorEnd)
{
    double dx = gradientVectorEnd.x() - gradientVectorStart.x();
    double dy = gradientVectorEnd.y() - gradientVectorStart.y();

    // Get angle from 0 to 2 PI.
    m_vectorAngle = atan2(dy, dx) + M_PI;
    m_radius = sqrt((dx * dx) + (dy * dy));
};

double ReverseSpiralGradientStrategy::valueAt(double x, double y) const
{
    double dx = x - m_gradientVectorStart.x();
    double dy = y - m_gradientVectorStart.y();

    double distance = sqrt((dx * dx) + (dy * dy));
    double angle = atan2(dy, dx) + M_PI;

    double t;
    angle -= m_vectorAngle;

    if (m_radius < DBL_EPSILON) {
        t = 0;
    } else {
        t = distance / m_radius;
    }

    if (angle < 0) {
        angle += 2 * M_PI;
    }

    //Reverse direction of spiral gradient
    t += 1 - (angle / (2 * M_PI));

    return t;

};

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

    double valueAt(double t) const override;

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

    double valueAt(double t) const override;

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

    double valueAt(double t) const override;

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
//Had to create this class to solve alternating mode for cases where values should be repeated for every HalfValues like for example, spirals...
class GradientRepeatModuloDivisiveContinuousHalfStrategy : public GradientRepeatStrategy
{
public:
    static GradientRepeatModuloDivisiveContinuousHalfStrategy *instance();

    double valueAt(double t) const override;

private:
    GradientRepeatModuloDivisiveContinuousHalfStrategy() {}

    static GradientRepeatModuloDivisiveContinuousHalfStrategy *m_instance;
};

GradientRepeatModuloDivisiveContinuousHalfStrategy *GradientRepeatModuloDivisiveContinuousHalfStrategy::m_instance = 0;

GradientRepeatModuloDivisiveContinuousHalfStrategy *GradientRepeatModuloDivisiveContinuousHalfStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new GradientRepeatModuloDivisiveContinuousHalfStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

// Output is 0 to 1, 1 to 0, 0 to 1, 1 to 0 per HalfValues
double GradientRepeatModuloDivisiveContinuousHalfStrategy::valueAt(double t) const
{
    if (t < 0) {
        t = -t;
    }

    int i = static_cast<int>(t*2);
    int ti = static_cast<int>(t);

    double value = t - ti;

    if (i % 2 == 1) {
        value = 1 - value;
    }

    return value*2;
}

class RepeatForwardsPaintPolicy
{
public:
    RepeatForwardsPaintPolicy(KisGradientPainter::enumGradientShape shape);

    void setup(const QPointF& gradientVectorStart,
               const QPointF& gradientVectorEnd,
               const QSharedPointer<KisGradientShapeStrategy> &shapeStrategy,
               const GradientRepeatStrategy *repeatStrategy,
               qreal antiAliasThreshold,
               bool reverseGradient,
               const KoCachedGradient * cachedGradient);

    const quint8 *colorAt(qreal x, qreal y) const;

private:
    KisGradientPainter::enumGradientShape m_shape;
    qreal m_antiAliasThresholdNormalized {0};
    qreal m_antiAliasThresholdNormalizedRev {0};
    qreal m_antiAliasThresholdNormalizedDbl {0};
    QSharedPointer<KisGradientShapeStrategy> m_shapeStrategy;
    const GradientRepeatStrategy *m_repeatStrategy {0};
    bool m_reverseGradient {false};
    const KoCachedGradient *m_cachedGradient {0};
    const quint8 *m_extremeColors[2];
    const KoColorSpace *m_colorSpace {0};
    mutable QVector<quint8> m_resultColor;
};

RepeatForwardsPaintPolicy::RepeatForwardsPaintPolicy(KisGradientPainter::enumGradientShape shape)
    : m_shape(shape)
{}

void RepeatForwardsPaintPolicy::setup(const QPointF& gradientVectorStart,
                                      const QPointF& gradientVectorEnd,
                                      const QSharedPointer<KisGradientShapeStrategy> &shapeStrategy,
                                      const GradientRepeatStrategy *repeatStrategy,
                                      qreal antiAliasThreshold,
                                      bool reverseGradient,
                                      const KoCachedGradient * cachedGradient)
{
    qreal dx = gradientVectorEnd.x() - gradientVectorStart.x();
    qreal dy = gradientVectorEnd.y() - gradientVectorStart.y();
    qreal distanceInPixels = sqrt(dx * dx + dy * dy);
    // Compute the area to be be smoothed
    // based on the length of the gradient
    m_antiAliasThresholdNormalized = antiAliasThreshold / distanceInPixels;
    m_antiAliasThresholdNormalizedRev = 1. - m_antiAliasThresholdNormalized;
    m_antiAliasThresholdNormalizedDbl = 2. * m_antiAliasThresholdNormalized;
    
    m_shapeStrategy = shapeStrategy;
    m_repeatStrategy = repeatStrategy;

    m_reverseGradient = reverseGradient;

    m_cachedGradient = cachedGradient;
    m_extremeColors[0] = m_cachedGradient->cachedAt(1.);
    m_extremeColors[1] = m_cachedGradient->cachedAt(0.);

    m_colorSpace = m_cachedGradient->colorSpace();

    m_resultColor = QVector<quint8>(m_colorSpace->pixelSize());
}

const quint8 *RepeatForwardsPaintPolicy::colorAt(qreal x, qreal y) const
{
    qreal t = m_shapeStrategy->valueAt(x, y);
    // Early return if the pixel is near the center of the gradient if
    // the shape is radial or square.
    // This prevents applying smoothing since there are
    // no aliasing artifacts in these gradient shapes at the center
    if (t <= m_antiAliasThresholdNormalized &&
        (m_shape == KisGradientPainter::GradientShapeBiLinear ||
         m_shape == KisGradientPainter::GradientShapeRadial ||
         m_shape == KisGradientPainter::GradientShapeSquare)) {
        if (m_reverseGradient) {
            t = 1 - t;
        }
        return m_cachedGradient->cachedAt(t);
    }

    t = m_repeatStrategy->valueAt(t);

    if (m_reverseGradient) {
        t = 1 - t;
    }

    // If this pixel is in the area of the smoothing,
    // then perform bilinear interpolation between the extreme colors.
    if (t <= m_antiAliasThresholdNormalized || t >= m_antiAliasThresholdNormalizedRev) {
        qreal s;
        if (t <= m_antiAliasThresholdNormalized) {
            s = .5 + t / m_antiAliasThresholdNormalizedDbl;
        } else {
            s = (t - m_antiAliasThresholdNormalizedRev) / m_antiAliasThresholdNormalizedDbl;
        }

        qint16 colorWeights[2];
        colorWeights[0] = std::lround((1.0 - s) * qint16_MAX);
        colorWeights[1] = qint16_MAX - colorWeights[0];

        m_colorSpace->mixColorsOp()->mixColors(m_extremeColors, colorWeights, 2, m_resultColor.data(), qint16_MAX);
        
        return m_resultColor.data();
    }

    return m_cachedGradient->cachedAt(t);
}

class ConicalGradientPaintPolicy
{
public:
    void setup(const QPointF& gradientVectorStart,
               const QPointF& gradientVectorEnd,
               const QSharedPointer<KisGradientShapeStrategy> &shapeStrategy,
               const GradientRepeatStrategy *repeatStrategy,
               qreal antiAliasThreshold,
               bool reverseGradient,
               const KoCachedGradient * cachedGradient);

    const quint8 *colorAt(qreal x, qreal y) const;

private:
    QPointF m_gradientVectorStart;
    QSharedPointer<KisGradientShapeStrategy> m_shapeStrategy;
    const GradientRepeatStrategy *m_repeatStrategy;
    qreal m_singularityThreshold;
    qreal m_antiAliasThreshold;
    bool m_reverseGradient;
    const KoCachedGradient *m_cachedGradient;
    const quint8 *m_extremeColors[2];
    const KoColorSpace *m_colorSpace;
    mutable QVector<quint8> m_resultColor;
};

void ConicalGradientPaintPolicy::setup(const QPointF& gradientVectorStart,
                                       const QPointF& gradientVectorEnd,
                                       const QSharedPointer<KisGradientShapeStrategy> &shapeStrategy,
                                       const GradientRepeatStrategy *repeatStrategy,
                                       qreal antiAliasThreshold,
                                       bool reverseGradient,
                                       const KoCachedGradient * cachedGradient)
{
    Q_UNUSED(gradientVectorEnd);

    m_gradientVectorStart = gradientVectorStart;
    
    m_shapeStrategy = shapeStrategy;
    m_repeatStrategy = repeatStrategy;

    m_singularityThreshold = 8.;
    m_antiAliasThreshold = antiAliasThreshold;

    m_reverseGradient = reverseGradient;

    m_cachedGradient = cachedGradient;
    m_extremeColors[0] = m_cachedGradient->cachedAt(1.);
    m_extremeColors[1] = m_cachedGradient->cachedAt(0.);

    m_colorSpace = m_cachedGradient->colorSpace();

    m_resultColor = QVector<quint8>(m_colorSpace->pixelSize());
}

const quint8 *ConicalGradientPaintPolicy::colorAt(qreal x, qreal y) const
{
    // Compute the distance from the center of the gradient to thecurrent pixel
    qreal dx = x - m_gradientVectorStart.x();
    qreal dy = y - m_gradientVectorStart.y();
    qreal distanceInPixels = sqrt(dx * dx + dy * dy);
    // Compute the perimeter for this distance
    qreal perimeter = 2. * M_PI * distanceInPixels;
    // The smoothing is applied in the vicinity of the aliased border.
    // The width of the vicinity is an area antiAliasThreshold pixels wide
    // to each side of the border, but in this case the area is scaled down
    // if it is too close to the center
    qreal antiAliasThresholdNormalized;
    if (distanceInPixels < m_singularityThreshold){
        antiAliasThresholdNormalized = distanceInPixels * m_antiAliasThreshold / m_singularityThreshold;
    } else {
        antiAliasThresholdNormalized = m_antiAliasThreshold;
    }
    antiAliasThresholdNormalized = antiAliasThresholdNormalized / perimeter;
    qreal antiAliasThresholdNormalizedRev = 1. - antiAliasThresholdNormalized;
    qreal antiAliasThresholdNormalizedDbl = 2. * antiAliasThresholdNormalized;

    qreal t = m_shapeStrategy->valueAt(x, y); 
    t = m_repeatStrategy->valueAt(t);

    if (m_reverseGradient) {
        t = 1 - t;
    }

    // If this pixel is in the area of the smoothing,
    // then perform bilinear interpolation between the extreme colors.
    if (t <= antiAliasThresholdNormalized || t >= antiAliasThresholdNormalizedRev) {
        qreal s;
        if (t <= antiAliasThresholdNormalized) {
            s = .5 + t / antiAliasThresholdNormalizedDbl;
        } else {
            s = (t - antiAliasThresholdNormalizedRev) / antiAliasThresholdNormalizedDbl;
        }

        qint16 colorWeights[2];
        colorWeights[0] = std::lround((1.0 - s) * qint16_MAX);
        colorWeights[1] = qint16_MAX - colorWeights[0];

        m_colorSpace->mixColorsOp()->mixColors(m_extremeColors, colorWeights, 2, m_resultColor.data(), qint16_MAX);

        return m_resultColor.data();
    }

    return m_cachedGradient->cachedAt(t);
}

class SpyralGradientRepeatNonePaintPolicy
{
public:
    SpyralGradientRepeatNonePaintPolicy(bool isReverseSpiral = false);

    void setup(const QPointF& gradientVectorStart,
               const QPointF& gradientVectorEnd,
               const QSharedPointer<KisGradientShapeStrategy> &shapeStrategy,
               const GradientRepeatStrategy *repeatStrategy,
               qreal antiAliasThreshold,
               bool reverseGradient,
               const KoCachedGradient * cachedGradient);

    const quint8 *colorAt(qreal x, qreal y) const;

private:
    QPointF m_gradientVectorStart;
    qreal m_distanceInPixels {0};
    qreal m_singularityThreshold {0};
    qreal m_angle {0};
    QSharedPointer<KisGradientShapeStrategy> m_shapeStrategy;
    const GradientRepeatStrategy *m_repeatStrategy {0};
    qreal m_antiAliasThreshold {0};
    bool m_reverseGradient {false};
    const KoCachedGradient *m_cachedGradient {0};
    mutable const quint8 *m_extremeColors[2];
    const KoColorSpace *m_colorSpace {0};
    mutable QVector<quint8> m_resultColor;
    bool m_isReverseSpiral {false};
};

SpyralGradientRepeatNonePaintPolicy::SpyralGradientRepeatNonePaintPolicy(bool isReverseSpiral)
    : m_isReverseSpiral(isReverseSpiral)
{
}

void SpyralGradientRepeatNonePaintPolicy::setup(const QPointF& gradientVectorStart,
                                                const QPointF& gradientVectorEnd,
                                                const QSharedPointer<KisGradientShapeStrategy> &shapeStrategy,
                                                const GradientRepeatStrategy *repeatStrategy,
                                                qreal antiAliasThreshold,
                                                bool reverseGradient,
                                                const KoCachedGradient * cachedGradient)
{
    m_gradientVectorStart = gradientVectorStart;

    qreal dx = gradientVectorEnd.x() - gradientVectorStart.x();
    qreal dy = gradientVectorEnd.y() - gradientVectorStart.y();
    m_distanceInPixels = sqrt(dx * dx + dy * dy);
    m_singularityThreshold = m_distanceInPixels / 32.;
    m_angle = atan2(dy, dx) + M_PI;
    
    m_shapeStrategy = shapeStrategy;
    m_repeatStrategy = repeatStrategy;

    m_antiAliasThreshold = antiAliasThreshold;

    m_reverseGradient = reverseGradient;

    m_cachedGradient = cachedGradient;

    m_colorSpace = m_cachedGradient->colorSpace();

    m_resultColor = QVector<quint8>(m_colorSpace->pixelSize());
}

const quint8 *SpyralGradientRepeatNonePaintPolicy::colorAt(qreal x, qreal y) const
{
    // Compute the distance from the center of the gradient to thecurrent pixel
    qreal dx = x - m_gradientVectorStart.x();
    qreal dy = y - m_gradientVectorStart.y();
    qreal distanceInPixels = sqrt(dx * dx + dy * dy);
    // Compute the perimeter for this distance
    qreal perimeter = 2. * M_PI * distanceInPixels;
    // The smoothing is applied in the vicinity of the aliased border.
    // The width of the vicinity is an area antiAliasThreshold pixels wide
    // to each side of the border, but in this case the area is scaled down
    // if it is too close to the center
    qreal antiAliasThresholdNormalized;
    if (distanceInPixels < m_singularityThreshold) {
        antiAliasThresholdNormalized = distanceInPixels * m_antiAliasThreshold / m_singularityThreshold;
    } else {
        antiAliasThresholdNormalized = m_antiAliasThreshold;
    }
    antiAliasThresholdNormalized = antiAliasThresholdNormalized / perimeter;
    qreal antiAliasThresholdNormalizedRev = 1. - antiAliasThresholdNormalized;
    qreal antiAliasThresholdNormalizedDbl = 2. * antiAliasThresholdNormalized;

    qreal t = m_shapeStrategy->valueAt(x, y); 
    t = m_repeatStrategy->valueAt(t);

    if (m_reverseGradient) {
        t = 1 - t;
    }

    // Compute the area to be be smoothed based on the angle of the gradient
    // and the angle of the current pixel to the center of the gradient
    qreal angle = atan2(dy, dx) + M_PI;
    angle -= m_angle;
    if (angle < 0.) {
        angle += 2. * M_PI;
    }
    angle /= (2. * M_PI);
    
    angle = m_repeatStrategy->valueAt(angle);

    // If this pixel is in the area of the smoothing,
    // then perform bilinear interpolation between the extreme colors.
    if (distanceInPixels < m_distanceInPixels && (angle <= antiAliasThresholdNormalized || angle >= antiAliasThresholdNormalizedRev)) {
        qreal s;
        if (angle <= antiAliasThresholdNormalized) {
            s = .5 + angle / antiAliasThresholdNormalizedDbl;
        } else {
            s = (angle - antiAliasThresholdNormalizedRev) / antiAliasThresholdNormalizedDbl;
        }

        if (m_reverseGradient) {
            distanceInPixels = m_distanceInPixels - distanceInPixels;
            m_extremeColors[0] = m_cachedGradient->cachedAt(0.);
        } else {
            m_extremeColors[0] = m_cachedGradient->cachedAt(1.);
        }

        if (m_isReverseSpiral) {
            m_extremeColors[1] = m_extremeColors[0];
            m_extremeColors[0] = (m_cachedGradient->cachedAt(distanceInPixels / m_distanceInPixels));
        } else {
            m_extremeColors[1] = (m_cachedGradient->cachedAt(distanceInPixels / m_distanceInPixels));
        }

        qint16 colorWeights[2];
        colorWeights[0] = std::lround((1.0 - s) * qint16_MAX);
        colorWeights[1] = qint16_MAX - colorWeights[0];

        m_colorSpace->mixColorsOp()->mixColors(m_extremeColors, colorWeights, 2, m_resultColor.data(), qint16_MAX);

        return m_resultColor.data();
    }

    return m_cachedGradient->cachedAt(t);
}

class NoAntialiasPaintPolicy
{
public:
    void setup(const QPointF& gradientVectorStart,
               const QPointF& gradientVectorEnd,
               const QSharedPointer<KisGradientShapeStrategy> &shapeStrategy,
               const GradientRepeatStrategy *repeatStrategy,
               qreal antiAliasThreshold,
               bool reverseGradient,
               const KoCachedGradient * cachedGradient);

    const quint8 *colorAt(qreal x, qreal y) const;

private:
    QSharedPointer<KisGradientShapeStrategy> m_shapeStrategy;
    const GradientRepeatStrategy *m_repeatStrategy {0};
    bool m_reverseGradient {false};
    const KoCachedGradient *m_cachedGradient {0};
};

void NoAntialiasPaintPolicy::setup(const QPointF& gradientVectorStart,
                                   const QPointF& gradientVectorEnd,
                                   const QSharedPointer<KisGradientShapeStrategy> &shapeStrategy,
                                   const GradientRepeatStrategy *repeatStrategy,
                                   qreal antiAliasThreshold,
                                   bool reverseGradient,
                                   const KoCachedGradient * cachedGradient)
{
    Q_UNUSED(gradientVectorStart);
    Q_UNUSED(gradientVectorEnd);
    Q_UNUSED(antiAliasThreshold);
    m_shapeStrategy = shapeStrategy;
    m_repeatStrategy = repeatStrategy;
    m_reverseGradient = reverseGradient;
    m_cachedGradient = cachedGradient;
}

const quint8 *NoAntialiasPaintPolicy::colorAt(qreal x, qreal y) const
{
    qreal t = m_shapeStrategy->valueAt(x, y);
    t = m_repeatStrategy->valueAt(t);

    if (m_reverseGradient) {
        t = 1 - t;
    }

    return m_cachedGradient->cachedAt(t);
}

}

struct Q_DECL_HIDDEN KisGradientPainter::Private
{
    enumGradientShape shape;

    struct ProcessRegion {
        ProcessRegion() {}
        ProcessRegion(QSharedPointer<KisGradientShapeStrategy> _precalculatedShapeStrategy,
                      const QRect &_processRect)
            : precalculatedShapeStrategy(_precalculatedShapeStrategy),
              processRect(_processRect) {}

        QSharedPointer<KisGradientShapeStrategy> precalculatedShapeStrategy;
        QRect processRect;
    };

    QVector<ProcessRegion> processRegions;
};

KisGradientPainter::KisGradientPainter()
    : m_d(new Private())
{
}

KisGradientPainter::KisGradientPainter(KisPaintDeviceSP device)
    : KisPainter(device),
      m_d(new Private())
{
}

KisGradientPainter::KisGradientPainter(KisPaintDeviceSP device, KisSelectionSP selection)
    : KisPainter(device, selection),
      m_d(new Private())
{
}

KisGradientPainter::~KisGradientPainter()
{
}

void KisGradientPainter::setGradientShape(enumGradientShape shape)
{
    m_d->shape = shape;
}

KisGradientShapeStrategy* createPolygonShapeStrategy(const QPainterPath &path, const QRect &boundingRect)
{
    // TODO: implement UI for exponent option
    const qreal exponent = 2.0;
    KisGradientShapeStrategy *strategy =
        new KisPolygonalGradientShapeStrategy(path, exponent);

    KIS_ASSERT_RECOVER_NOOP(boundingRect.width() >= 3 &&
                            boundingRect.height() >= 3);

    const qreal step =
        qMin(qreal(8.0), KritaUtils::maxDimensionPortion(boundingRect, 0.01, 2));

    return new KisCachedGradientShapeStrategy(boundingRect, step, step, strategy);
}

/**
 * TODO: make this call happen asynchronously when the user does nothing
 */
void KisGradientPainter::precalculateShape()
{
    if (!m_d->processRegions.isEmpty()) return;

    QPainterPath path;

    if (selection()) {
        if (!selection()->outlineCacheValid()) {
            selection()->recalculateOutlineCache();
        }

        KIS_ASSERT_RECOVER_RETURN(selection()->outlineCacheValid());
        KIS_ASSERT_RECOVER_RETURN(!selection()->outlineCache().isEmpty());

        path = selection()->outlineCache();
    } else {
        path.addRect(device()->defaultBounds()->bounds());
    }

    QList<QPainterPath> splitPaths = KritaUtils::splitDisjointPaths(path);

    Q_FOREACH (const QPainterPath &subpath, splitPaths) {
        QRect boundingRect = subpath.boundingRect().toAlignedRect();

        if (boundingRect.width() < 3 || boundingRect.height() < 3) {
            boundingRect = kisGrowRect(boundingRect, 2);
        }

        Private::ProcessRegion r(toQShared(createPolygonShapeStrategy(subpath, boundingRect)),
                                 boundingRect);
        m_d->processRegions << r;
    }
}

bool KisGradientPainter::paintGradient(const QPointF& gradientVectorStart,
                                       const QPointF& gradientVectorEnd,
                                       enumGradientRepeat repeat,
                                       double antiAliasThreshold,
                                       bool reverseGradient,
                                       qint32 startx,
                                       qint32 starty,
                                       qint32 width,
                                       qint32 height,
                                       bool useDithering)
{
    return paintGradient(gradientVectorStart,
                         gradientVectorEnd,
                         repeat,
                         antiAliasThreshold,
                         reverseGradient,
                         QRect(startx, starty, width, height),
                         useDithering);
}

bool KisGradientPainter::paintGradient(const QPointF& gradientVectorStart,
                                       const QPointF& gradientVectorEnd,
                                       enumGradientRepeat repeat,
                                       double antiAliasThreshold,
                                       bool reverseGradient,
                                       const QRect &applyRect,
                                       bool useDithering)
{
    // The following combinations of options have aliasing artifacts
    // where the first color meets the last color of the gradient.
    // so antialias threshold is used to compute if the pixel is in
    // the smothing area. Then linear interpolation is used to blend
    // between the first and last colors
    if (antiAliasThreshold > DBL_EPSILON) {
        if ((m_d->shape == GradientShapeLinear || m_d->shape == GradientShapeBiLinear ||
            m_d->shape == GradientShapeRadial || m_d->shape == GradientShapeSquare ||
            m_d->shape == GradientShapeSpiral || m_d->shape == GradientShapeReverseSpiral)
            && repeat == GradientRepeatForwards) {
            RepeatForwardsPaintPolicy paintPolicy(m_d->shape);
            return paintGradient(gradientVectorStart,
                                 gradientVectorEnd,
                                 repeat,
                                 antiAliasThreshold,
                                 reverseGradient,
                                 useDithering,
                                 applyRect,
                                 paintPolicy);

        } else if (m_d->shape == GradientShapeConical) {
            ConicalGradientPaintPolicy paintPolicy;
            return paintGradient(gradientVectorStart,
                                 gradientVectorEnd,
                                 repeat,
                                 antiAliasThreshold,
                                 reverseGradient,
                                 useDithering,
                                 applyRect,
                                 paintPolicy);

        } else if ((m_d->shape == GradientShapeSpiral || m_d->shape == GradientShapeReverseSpiral) &&
                   repeat == GradientRepeatNone) {
            SpyralGradientRepeatNonePaintPolicy paintPolicy(m_d->shape == GradientShapeReverseSpiral);
            return paintGradient(gradientVectorStart,
                                 gradientVectorEnd,
                                 repeat,
                                 antiAliasThreshold,
                                 reverseGradient,
                                 useDithering,
                                 applyRect,
                                 paintPolicy);
        }
    }

    // Default behavior: no antialiasing required
    NoAntialiasPaintPolicy paintPolicy;
    return paintGradient(gradientVectorStart,
                         gradientVectorEnd,
                         repeat,
                         antiAliasThreshold,
                         reverseGradient,
                         useDithering,
                         applyRect,
                         paintPolicy);
}

template <class T> 
bool KisGradientPainter::paintGradient(const QPointF& gradientVectorStart,
                                       const QPointF& gradientVectorEnd,
                                       enumGradientRepeat repeat,
                                       double antiAliasThreshold,
                                       bool reverseGradient,
                                       bool useDithering,
                                       const QRect &applyRect,
                                       T & paintPolicy)
{
    if (!gradient()) return false;

    QRect requestedRect = applyRect;

    //If the device has a selection only iterate over that selection united with our area of interest
    if (selection()) {
        requestedRect &= selection()->selectedExactRect();
    }

    QSharedPointer<KisGradientShapeStrategy> shapeStrategy;

    switch (m_d->shape) {
    case GradientShapeLinear: {
        Private::ProcessRegion r(toQShared(new LinearGradientStrategy(gradientVectorStart, gradientVectorEnd)),
                                 requestedRect);
        m_d->processRegions.clear();
        m_d->processRegions << r;
        break;
    }
    case GradientShapeBiLinear: {
        Private::ProcessRegion r(toQShared(new BiLinearGradientStrategy(gradientVectorStart, gradientVectorEnd)),
                                 requestedRect);
        m_d->processRegions.clear();
        m_d->processRegions << r;
        break;
    }
    case GradientShapeRadial: {
        Private::ProcessRegion r(toQShared(new RadialGradientStrategy(gradientVectorStart, gradientVectorEnd)),
                                 requestedRect);
        m_d->processRegions.clear();
        m_d->processRegions << r;
        break;
    }
    case GradientShapeSquare: {
        Private::ProcessRegion r(toQShared(new SquareGradientStrategy(gradientVectorStart, gradientVectorEnd)),
                                 requestedRect);
        m_d->processRegions.clear();
        m_d->processRegions << r;
        break;
    }
    case GradientShapeConical: {
        Private::ProcessRegion r(toQShared(new ConicalGradientStrategy(gradientVectorStart, gradientVectorEnd)),
                                 requestedRect);
        m_d->processRegions.clear();
        m_d->processRegions << r;
        break;
    }
    case GradientShapeConicalSymetric: {
        Private::ProcessRegion r(toQShared(new ConicalSymetricGradientStrategy(gradientVectorStart, gradientVectorEnd)),
                                 requestedRect);
        m_d->processRegions.clear();
        m_d->processRegions << r;
        break;
    }
    case GradientShapeSpiral: {
        Private::ProcessRegion r(toQShared(new SpiralGradientStrategy(gradientVectorStart, gradientVectorEnd)),
                                 requestedRect);
        m_d->processRegions.clear();
        m_d->processRegions << r;
        break;
    }
    case GradientShapeReverseSpiral: {
        Private::ProcessRegion r(toQShared(new ReverseSpiralGradientStrategy(gradientVectorStart, gradientVectorEnd)),
                                 requestedRect);
        m_d->processRegions.clear();
        m_d->processRegions << r;
        break;
    }
    case GradientShapePolygonal:
        precalculateShape();
        repeat = GradientRepeatNone;
        break;
    }

    GradientRepeatStrategy *repeatStrategy = 0;

    switch (repeat) {
    case GradientRepeatNone:
        repeatStrategy = GradientRepeatNoneStrategy::instance();
        break;
    case GradientRepeatForwards:
        repeatStrategy = GradientRepeatForwardsStrategy::instance();
        break;
    case GradientRepeatAlternate:
        if (m_d->shape == GradientShapeSpiral || m_d->shape == GradientShapeReverseSpiral) {repeatStrategy = GradientRepeatModuloDivisiveContinuousHalfStrategy::instance();}
        else {repeatStrategy = GradientRepeatAlternateStrategy::instance();}
        break;
    }
    Q_ASSERT(repeatStrategy != 0);


    KisPaintDeviceSP dev = device()->createCompositionSourceDevice();

    KoID depthId;
    const KoColorSpace *destCs = dev->colorSpace();

    if (destCs->colorDepthId() == Integer8BitsColorDepthID) {
        depthId = Integer16BitsColorDepthID;
    } else {
        depthId = destCs->colorDepthId();
    }

    const KoColorSpace *mixCs = KoColorSpaceRegistry::instance()->colorSpace(destCs->colorModelId().id(), depthId.id(), destCs->profile());
    const quint32 mixPixelSize = mixCs->pixelSize();

    KisPaintDeviceSP tmp(new KisPaintDevice(mixCs));
    tmp->setDefaultBounds(dev->defaultBounds());
    tmp->clear();

    const KisDitherOp* op = mixCs->ditherOp(destCs->colorDepthId().id(), useDithering ? DITHER_BEST : DITHER_NONE);

    Q_FOREACH (const Private::ProcessRegion &r, m_d->processRegions) {
        QRect processRect = r.processRect;
        QSharedPointer<KisGradientShapeStrategy> shapeStrategy = r.precalculatedShapeStrategy;

        KoCachedGradient cachedGradient(gradient(), qMax(processRect.width(), processRect.height()), mixCs);

        KisSequentialIteratorProgress it(tmp, processRect, progressUpdater());

        paintPolicy.setup(gradientVectorStart,
                          gradientVectorEnd,
                          shapeStrategy,
                          repeatStrategy,
                          antiAliasThreshold,
                          reverseGradient,
                          &cachedGradient);

        while (it.nextPixel()) {
            const quint8 *const pixel {paintPolicy.colorAt(it.x(), it.y())};
            memcpy(it.rawData(), pixel, mixPixelSize);
        }

        KisRandomAccessorSP dstIt = dev->createRandomAccessorNG();
        KisRandomConstAccessorSP srcIt = tmp->createRandomConstAccessorNG();

        int rows = 1;
        int columns = 1;

        for (int y = processRect.y(); y <= processRect.bottom(); y += rows) {
            rows = qMin(srcIt->numContiguousRows(y), qMin(dstIt->numContiguousRows(y), processRect.bottom() - y + 1));

            for (int x = processRect.x(); x <= processRect.right(); x += columns) {
                columns = qMin(srcIt->numContiguousColumns(x), qMin(dstIt->numContiguousColumns(x), processRect.right() - x + 1));

                srcIt->moveTo(x, y);
                dstIt->moveTo(x, y);

                const qint32 srcRowStride = srcIt->rowStride(x, y);
                const qint32 dstRowStride = dstIt->rowStride(x, y);
                const quint8 *srcPtr = srcIt->rawDataConst();
                quint8 *dstPtr = dstIt->rawData();

                op->dither(srcPtr, srcRowStride, dstPtr, dstRowStride, x, y, columns, rows);
            }
        }
    }

    bitBlt(requestedRect.topLeft(), dev, requestedRect);

    return true;
}
