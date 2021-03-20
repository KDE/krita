/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_distance_information.h>
#include <brushengine/kis_paint_information.h>
#include "kis_spacing_information.h"
#include "kis_timing_information.h"
#include "kis_debug.h"
#include <QtCore/qmath.h>
#include <QVector2D>
#include <QTransform>
#include "kis_algebra_2d.h"
#include "kis_dom_utils.h"

#include "kis_lod_transform.h"

const qreal MIN_DISTANCE_SPACING = 0.5;

// Smallest allowed interval when timed spacing is enabled, in milliseconds.
const qreal MIN_TIMED_INTERVAL = 0.5;

// Largest allowed interval when timed spacing is enabled, in milliseconds.
const qreal MAX_TIMED_INTERVAL = LONG_TIME;

struct Q_DECL_HIDDEN KisDistanceInformation::Private {
    Private() :
        accumDistance(),
        accumTime(0.0),
        spacingUpdateInterval(LONG_TIME),
        timeSinceSpacingUpdate(0.0),
        timingUpdateInterval(LONG_TIME),
        timeSinceTimingUpdate(0.0),
        lastAngle(0.0),
        lastDabInfoValid(false),
        lastPaintInfoValid(false),
        totalDistance(0.0),
        currentDabSeqNo(0),
        levelOfDetail(0)
    {
    }

    // Accumulators of time/distance passed since the last painted dab
    QPointF accumDistance;
    qreal accumTime;

    KisSpacingInformation spacing;
    qreal spacingUpdateInterval;
    // Accumulator of time passed since the last spacing update
    qreal timeSinceSpacingUpdate;

    KisTimingInformation timing;
    qreal timingUpdateInterval;
    // Accumulator of time passed since the last timing update
    qreal timeSinceTimingUpdate;

    // Information about the last position considered (not necessarily a painted dab)
    QPointF lastPosition;
    qreal lastAngle;
    bool lastDabInfoValid;

    // Information about the last painted dab
    KisPaintInformation lastPaintInformation;
    bool lastPaintInfoValid;

    qreal totalDistance;
    boost::optional<qreal> lockedDrawingAngleOptional;

    int currentDabSeqNo;
    int levelOfDetail;

    qreal lastMaxPressure = 0.0;
};

struct Q_DECL_HIDDEN KisDistanceInitInfo::Private {
    Private() :
        hasLastInfo(false),
        lastPosition(),
        lastAngle(0.0),
        spacingUpdateInterval(LONG_TIME),
        timingUpdateInterval(LONG_TIME),
        currentDabSeqNo(0)
    {
    }


    // Indicates whether lastPosition,  and lastAngle are valid or not.
    bool hasLastInfo;

    QPointF lastPosition;
    qreal lastAngle;

    qreal spacingUpdateInterval;
    qreal timingUpdateInterval;

    int currentDabSeqNo;
};

KisDistanceInitInfo::KisDistanceInitInfo()
    : m_d(new Private)
{
}

KisDistanceInitInfo::KisDistanceInitInfo(qreal spacingUpdateInterval, qreal timingUpdateInterval, int currentDabSeqNo)
    : m_d(new Private)
{
    m_d->spacingUpdateInterval = spacingUpdateInterval;
    m_d->timingUpdateInterval = timingUpdateInterval;
    m_d->currentDabSeqNo = currentDabSeqNo;
}

KisDistanceInitInfo::KisDistanceInitInfo(const QPointF &lastPosition,
                                         qreal lastAngle, int currentDabSeqNo)
    : m_d(new Private)
{
    m_d->hasLastInfo = true;
    m_d->lastPosition = lastPosition;
    m_d->lastAngle = lastAngle;
    m_d->currentDabSeqNo = currentDabSeqNo;
}

KisDistanceInitInfo::KisDistanceInitInfo(const QPointF &lastPosition,
                                         qreal lastAngle, qreal spacingUpdateInterval,
                                         qreal timingUpdateInterval,
                                         int currentDabSeqNo)
    : m_d(new Private)
{
    m_d->hasLastInfo = true;
    m_d->lastPosition = lastPosition;
    m_d->lastAngle = lastAngle;
    m_d->spacingUpdateInterval = spacingUpdateInterval;
    m_d->timingUpdateInterval = timingUpdateInterval;
    m_d->currentDabSeqNo = currentDabSeqNo;
}

KisDistanceInitInfo::KisDistanceInitInfo(const KisDistanceInitInfo &rhs)
    : m_d(new Private(*rhs.m_d))
{
}

KisDistanceInitInfo::~KisDistanceInitInfo()
{
    delete m_d;
}

bool KisDistanceInitInfo::operator==(const KisDistanceInitInfo &other) const
{
    if (m_d->spacingUpdateInterval != other.m_d->spacingUpdateInterval
        || m_d->timingUpdateInterval != other.m_d->timingUpdateInterval
        || m_d->hasLastInfo != other.m_d->hasLastInfo)
    {
        return false;
    }
    if (m_d->hasLastInfo) {
        if (m_d->lastPosition != other.m_d->lastPosition
            || m_d->lastAngle != other.m_d->lastAngle)
        {
            return false;
        }
    }

    if (m_d->currentDabSeqNo != other.m_d->currentDabSeqNo) {
        return false;
    }

    return true;
}

bool KisDistanceInitInfo::operator!=(const KisDistanceInitInfo &other) const
{
    return !(*this == other);
}

KisDistanceInitInfo &KisDistanceInitInfo::operator=(const KisDistanceInitInfo &rhs)
{
    *m_d = *rhs.m_d;
    return *this;
}

KisDistanceInformation KisDistanceInitInfo::makeDistInfo()
{
    if (m_d->hasLastInfo) {
        return KisDistanceInformation(m_d->lastPosition, m_d->lastAngle,
                                      m_d->spacingUpdateInterval, m_d->timingUpdateInterval,
                                      m_d->currentDabSeqNo);
    }
    else {
        return KisDistanceInformation(m_d->spacingUpdateInterval, m_d->timingUpdateInterval, m_d->currentDabSeqNo);
    }
}

void KisDistanceInitInfo::toXML(QDomDocument &doc, QDomElement &elt) const
{
    elt.setAttribute("spacingUpdateInterval", QString::number(m_d->spacingUpdateInterval, 'g', 15));
    elt.setAttribute("timingUpdateInterval", QString::number(m_d->timingUpdateInterval, 'g', 15));
    elt.setAttribute("currentDabSeqNo", QString::number(m_d->currentDabSeqNo));
    if (m_d->hasLastInfo) {
        QDomElement lastInfoElt = doc.createElement("LastInfo");
        lastInfoElt.setAttribute("lastPosX", QString::number(m_d->lastPosition.x(), 'g', 15));
        lastInfoElt.setAttribute("lastPosY", QString::number(m_d->lastPosition.y(), 'g', 15));
        lastInfoElt.setAttribute("lastAngle", QString::number(m_d->lastAngle, 'g', 15));
        elt.appendChild(lastInfoElt);
    }
}

KisDistanceInitInfo KisDistanceInitInfo::fromXML(const QDomElement &elt)
{
    const qreal spacingUpdateInterval = qreal(KisDomUtils::toDouble(elt.attribute("spacingUpdateInterval",
                                                                                  QString::number(LONG_TIME, 'g', 15))));
    const qreal timingUpdateInterval = qreal(KisDomUtils::toDouble(elt.attribute("timingUpdateInterval",
                                                                                  QString::number(LONG_TIME, 'g', 15))));
    const qreal currentDabSeqNo = KisDomUtils::toInt(elt.attribute("currentDabSeqNo", "0"));

    const QDomElement lastInfoElt = elt.firstChildElement("LastInfo");
    const bool hasLastInfo = !lastInfoElt.isNull();

    if (hasLastInfo) {
        const qreal lastPosX = qreal(KisDomUtils::toDouble(lastInfoElt.attribute("lastPosX",
                                                                                 "0.0")));
        const qreal lastPosY = qreal(KisDomUtils::toDouble(lastInfoElt.attribute("lastPosY",
                                                                                 "0.0")));
        const qreal lastAngle = qreal(KisDomUtils::toDouble(lastInfoElt.attribute("lastAngle",
                                                                                  "0.0")));

        return KisDistanceInitInfo(QPointF(lastPosX, lastPosY), lastAngle,
                                   spacingUpdateInterval, timingUpdateInterval,
                                   currentDabSeqNo);
    }
    else {
        return KisDistanceInitInfo(spacingUpdateInterval, timingUpdateInterval,
                                   currentDabSeqNo);
    }
}

KisDistanceInformation::KisDistanceInformation()
    : m_d(new Private)
{
}

KisDistanceInformation::KisDistanceInformation(qreal spacingUpdateInterval,
                                               qreal timingUpdateInterval,
                                               int currentDabSeqNo)
    : m_d(new Private)
{
    m_d->spacingUpdateInterval = spacingUpdateInterval;
    m_d->timingUpdateInterval = timingUpdateInterval;
    m_d->currentDabSeqNo = currentDabSeqNo;
}

KisDistanceInformation::KisDistanceInformation(const QPointF &lastPosition,
                                               qreal lastAngle)
    : m_d(new Private)
{
    m_d->lastPosition = lastPosition;
    m_d->lastAngle = lastAngle;

    m_d->lastDabInfoValid = true;
}

KisDistanceInformation::KisDistanceInformation(const QPointF &lastPosition,
                                               qreal lastAngle,
                                               qreal spacingUpdateInterval,
                                               qreal timingUpdateInterval,
                                               int currentDabSeqNo)
    : KisDistanceInformation(lastPosition, lastAngle)
{
    m_d->spacingUpdateInterval = spacingUpdateInterval;
    m_d->timingUpdateInterval = timingUpdateInterval;
    m_d->currentDabSeqNo = currentDabSeqNo;
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

    m_d->levelOfDetail = levelOfDetail;

    KisLodTransform t(levelOfDetail);
    m_d->lastPosition = t.map(m_d->lastPosition);
}

KisDistanceInformation& KisDistanceInformation::operator=(const KisDistanceInformation &rhs)
{
    *m_d = *rhs.m_d;
    return *this;
}

void KisDistanceInformation::overrideLastValues(const QPointF &lastPosition,
                                                qreal lastAngle)
{
    m_d->lastPosition = lastPosition;
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

void KisDistanceInformation::updateSpacing(const KisSpacingInformation &spacing)
{
    m_d->spacing = spacing;
    m_d->timeSinceSpacingUpdate = 0.0;
}

bool KisDistanceInformation::needsSpacingUpdate() const
{
    return m_d->timeSinceSpacingUpdate >= m_d->spacingUpdateInterval;
}

const KisTimingInformation &KisDistanceInformation::currentTiming() const
{
    return m_d->timing;
}

void KisDistanceInformation::updateTiming(const KisTimingInformation &timing)
{
    m_d->timing = timing;
    m_d->timeSinceTimingUpdate = 0.0;
}

bool KisDistanceInformation::needsTimingUpdate() const
{
    return m_d->timeSinceTimingUpdate >= m_d->timingUpdateInterval;
}

bool KisDistanceInformation::hasLastDabInformation() const
{
    return m_d->lastDabInfoValid;
}

QPointF KisDistanceInformation::lastPosition() const
{
    return m_d->lastPosition;
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

int KisDistanceInformation::currentDabSeqNo() const
{
    return m_d->currentDabSeqNo;
}

qreal KisDistanceInformation::maxPressure() const
{
    return m_d->lastMaxPressure;
}

bool KisDistanceInformation::isStarted() const
{
    return m_d->lastPaintInfoValid;
}

void KisDistanceInformation::registerPaintedDab(const KisPaintInformation &info,
                                                const KisSpacingInformation &spacing,
                                                const KisTimingInformation &timing)
{
    m_d->totalDistance +=
        KisAlgebra2D::norm(info.pos() - m_d->lastPosition) *
        KisLodTransform::lodToInvScale(m_d->levelOfDetail);

    m_d->lastPaintInformation = info;
    m_d->lastPaintInfoValid = true;

    m_d->lastAngle = info.drawingAngle(false);
    m_d->lastPosition = info.pos();
    m_d->lastDabInfoValid = true;

    m_d->spacing = spacing;
    m_d->timing = timing;

    m_d->currentDabSeqNo++;

    m_d->lastMaxPressure = qMax(info.pressure(), m_d->lastMaxPressure);
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
    if (m_d->timing.isTimedSpacingEnabled()) {
        timeFactor = getNextPointPositionTimed(startTime, endTime);
    }

    // Return the distance-based or time-based factor, whichever is smallest.
    qreal t = -1.0;
    if (distanceFactor < 0.0) {
        t = timeFactor;
    } else if (timeFactor < 0.0) {
        t = distanceFactor;
    } else {
        t = qMin(distanceFactor, timeFactor);
    }

    // If we aren't ready to paint a dab, accumulate time for the spacing/timing updates that might
    // be needed between dabs.
    if (t < 0.0) {
        m_d->timeSinceSpacingUpdate += endTime - startTime;
        m_d->timeSinceTimingUpdate += endTime - startTime;
    }

    // If we are ready to paint a dab, reset the accumulated time for spacing/timing updates.
    else {
        m_d->timeSinceSpacingUpdate = 0.0;
        m_d->timeSinceTimingUpdate = 0.0;
    }

    return t;
}

qreal KisDistanceInformation::getSpacingInterval() const
{
    return m_d->spacingUpdateInterval;
}

qreal KisDistanceInformation::getTimingUpdateInterval() const
{
    return m_d->timingUpdateInterval;
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

    // nextPointDistance can sometimes be negative if the spacing info has been modified since the
    // last interpolation attempt. In that case, have a point painted immediately.
    if (nextPointDistance <= 0.0) {
        resetAccumulators();
        t = 0.0;
    }
    else if (nextPointDistance <= dragVecLength) {
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

    qreal gamma = pow2(x * a_rev) + pow2(y * b_rev) - 1;

    // If the distance accumulator is already past the spacing ellipse, have a point painted
    // immediately. This can happen if the spacing info has been modified since the last
    // interpolation attempt.
    if (gamma >= 0.0) {
        resetAccumulators();
        return 0.0;
    }

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

    qreal timedSpacingInterval = qBound(MIN_TIMED_INTERVAL, m_d->timing.timedSpacingInterval(),
                                        MAX_TIMED_INTERVAL);
    qreal nextPointInterval = timedSpacingInterval - m_d->accumTime;
    
    qreal t = -1.0;

    // nextPointInterval can sometimes be negative if the spacing info has been modified since the
    // last interpolation attempt. In that case, have a point painted immediately.
    if (nextPointInterval <= 0.0) {
        resetAccumulators();
        t = 0.0;
    }
    else if (nextPointInterval <= endTime - startTime) {
        resetAccumulators();
        t = nextPointInterval / (endTime - startTime);
    }
    else {
        m_d->accumTime += endTime - startTime;
        t = -1.0;
    }
    
    return t;
}

void KisDistanceInformation::resetAccumulators()
{
    m_d->accumDistance = QPointF();
    m_d->accumTime = 0.0;
}

boost::optional<qreal> KisDistanceInformation::lockedDrawingAngleOptional() const
{
    return m_d->lockedDrawingAngleOptional;
}

void KisDistanceInformation::lockCurrentDrawingAngle(const KisPaintInformation &info) const
{
    const qreal angle = info.drawingAngle(false);

    qreal newAngle = angle;

    if (m_d->lockedDrawingAngleOptional) {
        const qreal stabilizingCoeff = 20.0;
        const qreal dist = stabilizingCoeff * m_d->spacing.scalarApprox();
        const qreal alpha = qMax(0.0, dist - scalarDistanceApprox()) / dist;

        const qreal oldAngle = *m_d->lockedDrawingAngleOptional;

        if (shortestAngularDistance(oldAngle, newAngle) < M_PI / 6) {
            newAngle = (1.0 - alpha) * oldAngle + alpha * newAngle;
        } else {
            newAngle = oldAngle;
        }
    }

    m_d->lockedDrawingAngleOptional = newAngle;
}


qreal KisDistanceInformation::scalarDistanceApprox() const
{
    return m_d->totalDistance;
}

