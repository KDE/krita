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
class KisSpacingInformation;
class KisTimingInformation;
class KisDistanceInformation;

/**
 * Represents some information that can be used to initialize a KisDistanceInformation object. The
 * main purpose of this class is to allow serialization of KisDistanceInformation initial settings
 * to XML.
 */
class KRITAIMAGE_EXPORT KisDistanceInitInfo {

public:

    /**
     * Creates a KisDistanceInitInfo with no initial last dab information, and spacing and timing
     * update intervals set to LONG_TIME.
     */
    explicit KisDistanceInitInfo();

    /**
     * Creates a KisDistanceInitInfo with no initial last dab information, and the specified spacing
     * and timing update intervals.
     */
    explicit KisDistanceInitInfo(qreal spacingUpdateInterval, qreal timingUpdateInterval);

    /**
     * Creates a KisDistanceInitInfo with the specified last dab information, and spacing and timing
     * update intervals set to LONG_TIME.
     */
    explicit KisDistanceInitInfo(const QPointF &lastPosition, qreal lastTime, qreal lastAngle);

    /**
     * Creates a KisDistanceInitInfo with the specified last dab information and spacing and timing
     * update intervals.
     */
    explicit KisDistanceInitInfo(const QPointF &lastPosition, qreal lastTime, qreal lastAngle,
                                 qreal spacingUpdateInterval, qreal timingUpdateInterval);

    KisDistanceInitInfo(const KisDistanceInitInfo &rhs);

    ~KisDistanceInitInfo();

    bool operator==(const KisDistanceInitInfo &other) const;

    bool operator!=(const KisDistanceInitInfo &other) const;

    KisDistanceInitInfo &operator=(const KisDistanceInitInfo &rhs);

    /**
     * Constructs a KisDistanceInformation with initial settings based on this object.
     */
    KisDistanceInformation makeDistInfo();

    void toXML(QDomDocument &doc, QDomElement &elt) const;

    static KisDistanceInitInfo fromXML(const QDomElement &elt);

private:
    struct Private;
    Private * const m_d;
};

/**
 * This structure keeps track of distance and timing information during a stroke, e.g. the time or
 * distance moved since the last dab.
 */
class KRITAIMAGE_EXPORT KisDistanceInformation {
public:
    KisDistanceInformation();
    KisDistanceInformation(qreal spacingUpdateInterval, qreal timingUpdateInterval);
    KisDistanceInformation(const QPointF &lastPosition, qreal lastTime, qreal lastAngle);
    /**
     * @param spacingUpdateInterval The amount of time allowed between spacing updates, in
     *                              milliseconds. Use LONG_TIME to only allow spacing updates when a
     *                              dab is painted.
     * @param timingUpdateInterval The amount of time allowed between time-based spacing updates, in
     *                             milliseconds. Use LONG_TIME to only allow timing updates when a
     *                             dab is painted.
     */
    KisDistanceInformation(const QPointF &lastPosition, qreal lastTime, qreal lastAngle,
                           qreal spacingUpdateInterval, qreal timingUpdateInterval);
    KisDistanceInformation(const KisDistanceInformation &rhs);
    KisDistanceInformation(const KisDistanceInformation &rhs, int levelOfDetail);
    KisDistanceInformation& operator=(const KisDistanceInformation &rhs);

    ~KisDistanceInformation();

    const KisSpacingInformation& currentSpacing() const;
    void updateSpacing(const KisSpacingInformation &spacing);
    /**
     * Returns true if this KisDistanceInformation should have its spacing information updated
     * immediately (regardless of whether a dab is ready to be painted).
     */
    bool needsSpacingUpdate() const;

    const KisTimingInformation &currentTiming() const;
    void updateTiming(const KisTimingInformation &timing);
    /**
     * Returns true if this KisDistanceInformation should have its timing information updated
     * immediately (regardless of whether a dab is ready to be painted).
     */
    bool needsTimingUpdate() const;

    bool hasLastDabInformation() const;
    QPointF lastPosition() const;
    qreal lastTime() const;
    qreal lastDrawingAngle() const;

    bool hasLastPaintInformation() const;
    const KisPaintInformation& lastPaintInformation() const;

    /**
     * @param spacing The new effective spacing after the dab. (Painting a dab is always supposed to
     *                cause a spacing update.)
     * @param timing The new effective timing after the dab. (Painting a dab is always supposed to
     *               cause a timing update.)
     */
    void registerPaintedDab(const KisPaintInformation &info,
                            const KisSpacingInformation &spacing,
                            const KisTimingInformation &timing);

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
