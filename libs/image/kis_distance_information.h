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
#include <QDomDocument>
#include <QDomElement>
#include "kritaimage_export.h"

class KisPaintInformation;
class KisDistanceInformation;

/**
 * A time in milliseconds that is assumed to be longer than any stroke (or other paint operation)
 * will ever last. This is used instead of infinity to avoid potential errors. The value is
 * approximately ten years.
 */
const qreal LONG_TIME = 320000000000.0;

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
     * @return The desired maximum amount of time between dabs, in milliseconds. Returns LONG_TIME
     * if time-based spacing is disabled.
     */
    inline qreal timedSpacingInterval() const {
        return isTimedSpacingEnabled() ?
                    m_timedSpacingInterval :
                    LONG_TIME;
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
 * Represents some information that can be used to initialize a KisDistanceInformation object. The
 * main purpose of this class is to allow serialization of KisDistanceInformation initial settings
 * to XML.
 */
class KRITAIMAGE_EXPORT KisDistanceInitInfo {

public:

    /**
     * Creates a KisDistanceInitInfo with no initial last dab information, and spacing update
     * interval set to LONG_TIME.
     */
    explicit KisDistanceInitInfo();

    /**
     * Creates a KisDistanceInitInfo with no initial last dab information, and the specified spacing
     * update interval.
     */
    explicit KisDistanceInitInfo(qreal spacingUpdateInterval);

    /**
     * Creates a KisDistanceInitInfo with the specified last dab information, and spacing update
     * interval set to LONG_TIME.
     */
    explicit KisDistanceInitInfo(const QPointF &lastPosition, qreal lastTime, qreal lastAngle);

    /**
     * Creates a KisDistanceInitInfo with the specified last dab information and spacing update
     * interval.
     */
    explicit KisDistanceInitInfo(const QPointF &lastPosition, qreal lastTime, qreal lastAngle,
                        qreal spacingUpdateInterval);

    /**
     * Constructs a KisDistanceInformation with initial settings based on this object.
     */
    KisDistanceInformation makeDistInfo();

    void toXML(QDomDocument &doc, QDomElement &elt) const;

    static KisDistanceInitInfo fromXML(const QDomElement &elt);

private:
    // Indicates whether lastPosition, lastTime, and lastAngle are valid or not.
    bool m_hasLastInfo;

    QPointF m_lastPosition;
    qreal m_lastTime;
    qreal m_lastAngle;

    qreal m_spacingUpdateInterval;
};

/**
 * This structure is used as return value of paintLine to contain
 * information that is needed to be passed for the next call.
 */
class KRITAIMAGE_EXPORT KisDistanceInformation {
public:
    KisDistanceInformation();
    KisDistanceInformation(qreal spacingUpdateInterval);
    KisDistanceInformation(const QPointF &lastPosition, qreal lastTime, qreal lastAngle);
    /**
     * @param spacingUpdateInterval The amount of time allowed between spacing updates, in
     *                              milliseconds. Only used when timed spacing is enabled.
     */
    KisDistanceInformation(const QPointF &lastPosition, qreal lastTime, qreal lastAngle,
                           qreal spacingUpdateInterval);
    KisDistanceInformation(const KisDistanceInformation &rhs);
    KisDistanceInformation(const KisDistanceInformation &rhs, int levelOfDetail);
    KisDistanceInformation& operator=(const KisDistanceInformation &rhs);

    ~KisDistanceInformation();

    const KisSpacingInformation& currentSpacing() const;
    void setSpacing(const KisSpacingInformation &spacing);
    /**
     * Returns true if this KisDistanceInformation should have its spacing information updated
     * immediately (regardless of whether a dab is ready to be painted).
     */
    bool needsSpacingUpdate() const;

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

    /**
     * Computes the next drawing angle assuming that the next painting position will be nextPos.
     * This method should not be called when hasLastDabInformation() is false.
     */
    qreal nextDrawingAngle(const QPointF &nextPos, bool considerLockedAngle = true) const;

    /**
     * Returns a unit vector pointing in the direction that would have been indicated by a call to
     * nextDrawingAngle. This method should not be called when hasLastDabInformation() is false.
     */
    QPointF nextDrawingDirectionVector(const QPointF &nextPos,
                                       bool considerLockedAngle = true) const;

    qreal scalarDistanceApprox() const;

    void overrideLastValues(const QPointF &lastPosition, qreal lastTime, qreal lastAngle);

private:
    qreal getNextPointPositionIsotropic(const QPointF &start,
                                        const QPointF &end);
    qreal getNextPointPositionAnisotropic(const QPointF &start,
                                          const QPointF &end);
    qreal getNextPointPositionTimed(qreal startTime,
                                    qreal endTime);
    void resetAccumulators();

    qreal drawingAngleImpl(const QPointF &start, const QPointF &end,
                           bool considerLockedAngle = true, qreal defaultAngle = 0.0) const;
    QPointF drawingDirectionVectorImpl(const QPointF &start, const QPointF &end,
                                       bool considerLockedAngle = true,
                                       qreal defaultAngle = 0.0) const;

private:
    struct Private;
    Private * const m_d;
};

#endif
