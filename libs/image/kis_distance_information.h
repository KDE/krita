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

#ifndef _KIS_DISTANCE_INFORMATION_H_
#define _KIS_DISTANCE_INFORMATION_H_

#include <QPointF>
#include <QVector2D>
#include "kritaimage_export.h"

class KisPaintInformation;


/**
 * This structure contains information about the desired spacing
 * requested by the paintAt call
 */
class KisSpacingInformation {
public:
    explicit KisSpacingInformation()
        : m_distanceSpacingEnabled(true)
        , m_distanceSpacing(0.0, 0.0)
        , m_timedSpacingEnabled(false)
        , m_timedSpacingInterval(0.0)
        , m_rotation(0.0)
        , m_coordinateSystemFlipped(false)
    {
    }

    explicit KisSpacingInformation(qreal isotropicSpacing)
        : m_distanceSpacingEnabled(true)
        , m_distanceSpacing(isotropicSpacing, isotropicSpacing)
        , m_timedSpacingEnabled(false)
        , m_timedSpacingInterval(0.0)
        , m_rotation(0.0)
        , m_coordinateSystemFlipped(false)
    {
    }

    explicit KisSpacingInformation(const QPointF &anisotropicSpacing, qreal rotation, bool coordinateSystemFlipped)
        : m_distanceSpacingEnabled(true)
        , m_distanceSpacing(anisotropicSpacing)
        , m_timedSpacingEnabled(false)
        , m_timedSpacingInterval(0.0)
        , m_rotation(rotation)
        , m_coordinateSystemFlipped(coordinateSystemFlipped)
    {
    }

    explicit KisSpacingInformation(qreal isotropicSpacing,
                                   qreal timedSpacingInterval)
        : m_distanceSpacingEnabled(true)
        , m_distanceSpacing(isotropicSpacing, isotropicSpacing)
        , m_timedSpacingEnabled(true)
        , m_timedSpacingInterval(timedSpacingInterval)
        , m_rotation(0.0)
        , m_coordinateSystemFlipped(false)
    {
    }

    explicit KisSpacingInformation(const QPointF &anisotropicSpacing,
                                   qreal rotation,
                                   bool coordinateSystemFlipped,
                                   qreal timedSpacingInterval)
        : m_distanceSpacingEnabled(true)
        , m_distanceSpacing(anisotropicSpacing)
        , m_timedSpacingEnabled(true)
        , m_timedSpacingInterval(timedSpacingInterval)
        , m_rotation(rotation)
        , m_coordinateSystemFlipped(coordinateSystemFlipped)
    {
    }

    explicit KisSpacingInformation(bool distanceSpacingEnabled,
                                   qreal isotropicSpacing,
                                   bool timedSpacingEnabled,
                                   qreal timedSpacingInterval)
        : m_distanceSpacingEnabled(distanceSpacingEnabled)
        , m_distanceSpacing(isotropicSpacing, isotropicSpacing)
        , m_timedSpacingEnabled(timedSpacingEnabled)
        , m_timedSpacingInterval(timedSpacingInterval)
        , m_rotation(0.0)
        , m_coordinateSystemFlipped(false)
    {
    }

    explicit KisSpacingInformation(bool distanceSpacingEnabled,
                                   const QPointF &anisotropicSpacing,
                                   qreal rotation,
                                   bool coordinateSystemFlipped,
                                   bool timedSpacingEnabled,
                                   qreal timedSpacingInterval)
        : m_distanceSpacingEnabled(distanceSpacingEnabled)
        , m_distanceSpacing(anisotropicSpacing)
        , m_timedSpacingEnabled(timedSpacingEnabled)
        , m_timedSpacingInterval(timedSpacingInterval)
        , m_rotation(rotation)
        , m_coordinateSystemFlipped(coordinateSystemFlipped)
    {
    }

    /**
     * @return True if and only if distance-based spacing is enabled.
     */
    inline bool isDistanceSpacingEnabled() const {
        return m_distanceSpacingEnabled;
    }

    inline QPointF distanceSpacing() const {
        return m_distanceSpacing;
    }

    /**
     * @return True if and only if time-based spacing is enabled.
     */
    inline bool isTimedSpacingEnabled() const {
        return m_timedSpacingEnabled;
    }

    /**
     * @return The desired maximum amount of time between dabs, in milliseconds. Returns a time of
     * approximately 1 year if time-based spacing is disabled.
     */
    inline qreal timedSpacingInterval() const {
        return isTimedSpacingEnabled() ?
                    m_timedSpacingInterval :
                    32000000000.0;
    }

    inline bool isIsotropic() const {
        return m_distanceSpacing.x() == m_distanceSpacing.y();
    }

    inline qreal scalarApprox() const {
        return isIsotropic() ? m_distanceSpacing.x() : QVector2D(m_distanceSpacing).length();
    }

    inline qreal rotation() const {
        return m_rotation;
    }

    bool coordinateSystemFlipped() const {
        return m_coordinateSystemFlipped;
    }

private:

    // Distance-based spacing
    bool m_distanceSpacingEnabled;
    QPointF m_distanceSpacing;

    // Time-based spacing (interval is in milliseconds)
    bool m_timedSpacingEnabled;
    qreal m_timedSpacingInterval;

    qreal m_rotation;
    bool m_coordinateSystemFlipped;
};

/**
 * This structure is used as return value of paintLine to contain
 * information that is needed to be passed for the next call.
 */
class KRITAIMAGE_EXPORT KisDistanceInformation {
public:
    KisDistanceInformation();
    KisDistanceInformation(const QPointF &lastPosition, qreal lastTime);
    KisDistanceInformation(const KisDistanceInformation &rhs);
    KisDistanceInformation(const KisDistanceInformation &rhs, int levelOfDetail);
    KisDistanceInformation& operator=(const KisDistanceInformation &rhs);

    ~KisDistanceInformation();

    const KisSpacingInformation& currentSpacing() const;
    bool hasLastDabInformation() const;
    QPointF lastPosition() const;
    qreal lastTime() const;
    qreal lastDrawingAngle() const;

    bool hasLastPaintInformation() const;
    const KisPaintInformation& lastPaintInformation() const;

    void registerPaintedDab(const KisPaintInformation &info,
                            const KisSpacingInformation &spacing);

    qreal getNextPointPosition(const QPointF &start,
                               const QPointF &end,
                               qreal startTime,
                               qreal endTime);

    /**
     * \return true if at least one dab has been painted with this
     *         distance information
     */
    bool isStarted() const;

    bool hasLockedDrawingAngle() const;
    qreal lockedDrawingAngle() const;
    void setLockedDrawingAngle(qreal angle);

    qreal scalarDistanceApprox() const;

    void overrideLastValues(const QPointF &lastPosition, qreal lastTime);

private:
    qreal getNextPointPositionIsotropic(const QPointF &start,
                                        const QPointF &end);
    qreal getNextPointPositionAnisotropic(const QPointF &start,
                                          const QPointF &end);
    qreal getNextPointPositionTimed(qreal startTime,
                                    qreal endTime);
    void resetAccumulators();
private:
    struct Private;
    Private * const m_d;
};

#endif
