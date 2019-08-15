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

#include <cfloat>

#include <KoColorSpace.h>
#include <resources/KoAbstractGradient.h>
#include <KoUpdater.h>

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


class CachedGradient : public KoAbstractGradient
{

public:
    explicit CachedGradient(const KoAbstractGradient *gradient, qint32 steps, const KoColorSpace *cs)
        : KoAbstractGradient(gradient->filename())
    {
        m_subject = gradient;
        m_max = steps - 1;
        m_colorSpace = cs;

        m_black = KoColor(cs);

        KoColor tmpColor(m_colorSpace);
        for(qint32 i = 0; i < steps; i++) {
            m_subject->colorAt(tmpColor, qreal(i) / m_max);
            m_colors << tmpColor;
        }
    }

    ~CachedGradient() override {}

    KoAbstractGradient* clone() const override {
        return new CachedGradient(m_subject, m_max + 1, m_colorSpace);
    }

    /**
    * Creates a QGradient from the gradient.
    * The resulting QGradient might differ from original gradient
    */
    QGradient* toQGradient() const override
    {
        return m_subject->toQGradient();
    }


    /// gets the color data at position 0 <= t <= 1
    const quint8 *cachedAt(qreal t) const
    {
        qint32 tInt = t * m_max + 0.5;
        if (m_colors.size() > tInt) {
            return m_colors[tInt].data();
        }
        else {
            return m_black.data();
        }
    }

    void setColorSpace(KoColorSpace* colorSpace) { m_colorSpace = colorSpace; }
    const KoColorSpace * colorSpace() const { return m_colorSpace; }

    QByteArray generateMD5() const override { return QByteArray(); }

private:
    const KoAbstractGradient *m_subject;
    const KoColorSpace *m_colorSpace;
    qint32 m_max;
    QVector<KoColor> m_colors;
    KoColor m_black;
};

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
                                       qint32 height)
{
    return paintGradient(gradientVectorStart,
                         gradientVectorEnd,
                         repeat,
                         antiAliasThreshold,
                         reverseGradient,
                         QRect(startx, starty, width, height));
}

bool KisGradientPainter::paintGradient(const QPointF& gradientVectorStart,
                                       const QPointF& gradientVectorEnd,
                                       enumGradientRepeat repeat,
                                       double antiAliasThreshold,
                                       bool reverseGradient,
                                       const QRect &applyRect)
{
    Q_UNUSED(antiAliasThreshold);

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

    const KoColorSpace * colorSpace = dev->colorSpace();
    const qint32 pixelSize = colorSpace->pixelSize();

    Q_FOREACH (const Private::ProcessRegion &r, m_d->processRegions) {
        QRect processRect = r.processRect;
        QSharedPointer<KisGradientShapeStrategy> shapeStrategy = r.precalculatedShapeStrategy;

        CachedGradient cachedGradient(gradient(), qMax(processRect.width(), processRect.height()), colorSpace);

        KisSequentialIteratorProgress it(dev, processRect, progressUpdater());

        while (it.nextPixel()) {
            double t = shapeStrategy->valueAt(it.x(), it.y());
            t = repeatStrategy->valueAt(t);

            if (reverseGradient) {
                t = 1 - t;
            }

            memcpy(it.rawData(), cachedGradient.cachedAt(t), pixelSize);
        }

        bitBlt(processRect.topLeft(), dev, processRect);
    }

    return true;
}
