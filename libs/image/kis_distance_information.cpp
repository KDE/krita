/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include <kis_distance_information.h>
#include <brushengine/kis_paint_information.h>
#include "kis_debug.h"
#include <QtCore/qmath.h>
#include <QVector2D>
#include <QTransform>
#include "kis_algebra_2d.h"

#include "kis_lod_transform.h"

const qreal MIN_DISTANCE_SPACING = 0.5;

// Smallest allowed interval when timed spacing is enabled, in milliseconds.
const qreal MIN_TIMED_INTERVAL = 0.5;

// Largest allowed interval when timed spacing is enabled, in milliseconds.
const qreal MAX_TIMED_INTERVAL = 1000.0;

struct Q_DECL_HIDDEN KisDistanceInformation::Private {
    Private() :
        accumDistance(),
        accumTime(0.0),
        lastDabInfoValid(false),
        lastPaintInfoValid(false),
        lockedDrawingAngle(0.0),
        hasLockedDrawingAngle(false),
        totalDistance(0.0) {}

    QPointF accumDistance;
    qreal accumTime;
    KisSpacingInformation spacing;

    QPointF lastPosition;
    qreal lastTime;
    bool lastDabInfoValid;

    KisPaintInformation lastPaintInformation;
    qreal lastAngle;
    bool lastPaintInfoValid;

    qreal lockedDrawingAngle;
    bool hasLockedDrawingAngle;
    qreal totalDistance;
};

KisDistanceInformation::KisDistanceInformation()
    : m_d(new Private)
{
}

KisDistanceInformation::KisDistanceInformation(const QPointF &lastPosition,
                                               qreal lastTime,
                                               qreal lastAngle)
    : m_d(new Private)
{
    m_d->lastPosition = lastPosition;
    m_d->lastTime = lastTime;
    m_d->lastAngle = lastAngle;

    m_d->lastDabInfoValid = true;
}

KisDistanceInformation::KisDistanceInformation(const KisDistanceInformation &rhs)
    : m_d(new Private(*rhs.m_d))
{

}

KisDistanceInformation::KisDistanceInformation(const KisDistanceInformation &rhs, int levelOfDetail)
    : m_d(new Private(*rhs.m_d))
{
    KIS_ASSERT_RECOVER_NOOP(!m_d->lastPaintInfoValid &&
                            "The distance information "
                            "should be cloned before the "
                            "actual painting is started");

    KisLodTransform t(levelOfDetail);
    m_d->lastPosition = t.map(m_d->lastPosition);
}

KisDistanceInformation& KisDistanceInformation::operator=(const KisDistanceInformation &rhs)
{
    *m_d = *rhs.m_d;
    return *this;
}

void KisDistanceInformation::overrideLastValues(const QPointF &lastPosition, qreal lastTime,
                                                qreal lastAngle)
{
    m_d->lastPosition = lastPosition;
    m_d->lastTime = lastTime;
    m_d->lastAngle = lastAngle;

    m_d->lastDabInfoValid = true;
}

KisDistanceInformation::~KisDistanceInformation()
{
    delete m_d;
}

const KisSpacingInformation& KisDistanceInformation::currentSpacing() const
{
    return m_d->spacing;
}

bool KisDistanceInformation::hasLastDabInformation() const
{
    return m_d->lastDabInfoValid;
}

QPointF KisDistanceInformation::lastPosition() const
{
    return m_d->lastPosition;
}

qreal KisDistanceInformation::lastTime() const
{
    return m_d->lastTime;
}

qreal KisDistanceInformation::lastDrawingAngle() const
{
    return m_d->lastAngle;
}

bool KisDistanceInformation::hasLastPaintInformation() const
{
    return m_d->lastPaintInfoValid;
}

const KisPaintInformation& KisDistanceInformation::lastPaintInformation() const
{
    return m_d->lastPaintInformation;
}

bool KisDistanceInformation::isStarted() const
{
    return m_d->lastPaintInfoValid;
}

void KisDistanceInformation::registerPaintedDab(const KisPaintInformation &info,
                                                const KisSpacingInformation &spacing)
{
    m_d->totalDistance += KisAlgebra2D::norm(info.pos() - m_d->lastPosition);

    m_d->lastPaintInformation = info;
    m_d->lastPaintInfoValid = true;

    m_d->lastAngle = nextDrawingAngle(info.pos());
    m_d->lastPosition = info.pos();
    m_d->lastTime = info.currentTime();
    m_d->lastDabInfoValid = true;

    m_d->spacing = spacing;
}

qreal KisDistanceInformation::getNextPointPosition(const QPointF &start,
                                                   const QPointF &end,
                                                   qreal startTime,
                                                   qreal endTime)
{
    // Compute interpolation factor based on distance.
    qreal distanceFactor = -1.0;
    if (m_d->spacing.isDistanceSpacingEnabled()) {
        distanceFactor = m_d->spacing.isIsotropic() ?
            getNextPointPositionIsotropic(start, end) :
            getNextPointPositionAnisotropic(start, end);
    }

    // Compute interpolation factor based on time.
    qreal timeFactor = -1.0;
    if (m_d->spacing.isTimedSpacingEnabled()) {
        timeFactor = getNextPointPositionTimed(startTime, endTime);
    }

    // Return the distance-based or time-based factor, whichever is smallest.
    if (distanceFactor < 0.0) {
        return timeFactor;
    } else if (timeFactor < 0.0) {
        return distanceFactor;
    } else {
        return qMin(distanceFactor, timeFactor);
    }
}

qreal KisDistanceInformation::getNextPointPositionIsotropic(const QPointF &start,
                                                            const QPointF &end)
{
    qreal distance = m_d->accumDistance.x();
    qreal spacing = qMax(MIN_DISTANCE_SPACING, m_d->spacing.distanceSpacing().x());

    if (start == end) {
        return -1;
    }

    qreal dragVecLength = QVector2D(end - start).length();
    qreal nextPointDistance = spacing - distance;

    qreal t;

    if (nextPointDistance <= dragVecLength) {
        t = nextPointDistance / dragVecLength;
        resetAccumulators();
    } else {
        t = -1;
        m_d->accumDistance.rx() += dragVecLength;
    }

    return t;
}

qreal KisDistanceInformation::getNextPointPositionAnisotropic(const QPointF &start,
                                                              const QPointF &end)
{
    if (start == end) {
        return -1;
    }

    qreal a_rev = 1.0 / qMax(MIN_DISTANCE_SPACING, m_d->spacing.distanceSpacing().x());
    qreal b_rev = 1.0 / qMax(MIN_DISTANCE_SPACING, m_d->spacing.distanceSpacing().y());

    qreal x = m_d->accumDistance.x();
    qreal y = m_d->accumDistance.y();

    static const qreal eps = 2e-3; // < 0.2 deg

    qreal currentRotation = m_d->spacing.rotation();
    if (m_d->spacing.coordinateSystemFlipped()) {
        currentRotation = 2 * M_PI - currentRotation;
    }

    QPointF diff = end - start;

    if (currentRotation > eps) {
        QTransform rot;
        // since the ellipse is symmetrical, the sign
        // of rotation doesn't matter
        rot.rotateRadians(currentRotation);
        diff = rot.map(diff);
    }

    qreal dx = qAbs(diff.x());
    qreal dy = qAbs(diff.y());

    qreal alpha = pow2(dx * a_rev) + pow2(dy * b_rev);
    qreal beta = x * dx * a_rev * a_rev + y * dy * b_rev * b_rev;
    qreal gamma = pow2(x * a_rev) + pow2(y * b_rev) - 1;

    qreal D_4 = pow2(beta) - alpha * gamma;

    qreal t = -1.0;

    if (D_4 >= 0) {
        qreal k = (-beta + qSqrt(D_4)) / alpha;

        if (k >= 0.0 && k <= 1.0) {
            t = k;
            resetAccumulators();
        } else {
            m_d->accumDistance += KisAlgebra2D::abs(diff);
        }
    } else {
        warnKrita << "BUG: No solution for elliptical spacing equation has been found. This shouldn't have happened.";
    }

    return t;
}

qreal KisDistanceInformation::getNextPointPositionTimed(qreal startTime,
                                                        qreal endTime)
{
    // If start time is not before end time, do not interpolate.
    if (!(startTime < endTime)) {
        return -1.0;
    }

    qreal timedSpacingInterval = qBound(MIN_TIMED_INTERVAL, m_d->spacing.timedSpacingInterval(),
                                        MAX_TIMED_INTERVAL);
    qreal nextPointInterval = timedSpacingInterval - m_d->accumTime;
    
    // Note: nextPointInterval SHOULD always be positive, but I wasn't sure if floating point
    // roundoff error might make it nonpositive in some cases, so I included this check.
    if (nextPointInterval <= 0.0) {
        resetAccumulators();
        return 0.0;
    }
    else if (nextPointInterval <= endTime - startTime) {
        resetAccumulators();
        return nextPointInterval / (endTime - startTime);
    }
    else {
        m_d->accumTime += endTime - startTime;
        return -1.0;
    }
}

void KisDistanceInformation::resetAccumulators()
{
    m_d->accumDistance = QPointF();
    m_d->accumTime = 0.0;
}

bool KisDistanceInformation::hasLockedDrawingAngle() const
{
    return m_d->hasLockedDrawingAngle;
}

qreal KisDistanceInformation::lockedDrawingAngle() const
{
    return m_d->lockedDrawingAngle;
}

void KisDistanceInformation::setLockedDrawingAngle(qreal angle)
{
    m_d->hasLockedDrawingAngle = true;
    m_d->lockedDrawingAngle = angle;
}

qreal KisDistanceInformation::nextDrawingAngle(const QPointF &nextPos,
                                               bool considerLockedAngle) const
{
    if (!m_d->lastDabInfoValid) {
        warnKrita << "KisDistanceInformation::nextDrawingAngle()" << "No last dab data";
        return 0.0;
    }

    // Compute the drawing angle. If the new position is the same as the previous position, an angle
    // can't be computed. In that case, act as if the angle is the same as in the previous dab.
    return drawingAngleImpl(m_d->lastPosition, nextPos, considerLockedAngle, m_d->lastAngle);
}

QPointF KisDistanceInformation::nextDrawingDirectionVector(const QPointF &nextPos,
                                                           bool considerLockedAngle) const
{
    if (!m_d->lastDabInfoValid) {
        warnKrita << "KisDistanceInformation::nextDrawingDirectionVector()" << "No last dab data";
        return QPointF(1.0, 0.0);
    }

    // Compute the direction vector. If the new position is the same as the previous position, a
    // direction can't be computed. In that case, act as if the direction is the same as in the
    // previous dab.
    return drawingDirectionVectorImpl(m_d->lastPosition, nextPos, considerLockedAngle,
                                      m_d->lastAngle);
}

qreal KisDistanceInformation::scalarDistanceApprox() const
{
    return m_d->totalDistance;
}

qreal KisDistanceInformation::drawingAngleImpl(const QPointF &start, const QPointF &end,
                                               bool considerLockedAngle, qreal defaultAngle) const
{
    if (m_d->hasLockedDrawingAngle && considerLockedAngle) {
        return m_d->lockedDrawingAngle;
    }

    // If the start and end positions are the same, we can't compute an angle. In that case, use the
    // provided default.
    return KisAlgebra2D::directionBetweenPoints(start, end, defaultAngle);
}

QPointF KisDistanceInformation::drawingDirectionVectorImpl(const QPointF &start, const QPointF &end,
                                                           bool considerLockedAngle,
                                                           qreal defaultAngle) const
{
    if (m_d->hasLockedDrawingAngle && considerLockedAngle) {
        return QPointF(cos(m_d->lockedDrawingAngle), sin(m_d->lockedDrawingAngle));
    }

    // If the start and end positions are the same, we can't compute a drawing direction. In that
    // case, use the provided default.
    if (KisAlgebra2D::fuzzyPointCompare(start, end)) {
        return QPointF(cos(defaultAngle), sin(defaultAngle));
    }

    const QPointF diff(end - start);
    return KisAlgebra2D::normalize(diff);
}
