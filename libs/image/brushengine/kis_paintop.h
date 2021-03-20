/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004, 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINTOP_H_
#define KIS_PAINTOP_H_

#include <kis_distance_information.h>
#include "kis_shared.h"
#include "kis_types.h"

#include <kritaimage_export.h>

class QPointF;
class KoColorSpace;

class KisPainter;
class KisPaintInformation;
class KisRunnableStrokeJobData;

/**
 * KisPaintOp are use by tools to draw on a paint device. A paintop takes settings
 * and input information, like pressure, tilt or motion and uses that to draw pixels
 */
class KRITAIMAGE_EXPORT KisPaintOp : public KisShared
{
    struct Private;
public:

    KisPaintOp(KisPainter * painter);
    virtual ~KisPaintOp();

    /**
     * Paint at the subpixel point pos using the specified paint information..
     *
     * The distance/time between two calls of the paintAt is always specified by spacing and timing,
     * which are automatically saved into the current distance information object.
     */
    void paintAt(const KisPaintInformation& info, KisDistanceInformation *currentDistance);

    /**
     * Updates the spacing settings in currentDistance based on the provided information. Note that
     * the spacing is updated automatically in the paintAt method, so there is no need to call this
     * method if paintAt has just been called.
     */
    void updateSpacing(const KisPaintInformation &info, KisDistanceInformation &currentDistance)
        const;

    /**
     * Updates the timing settings in currentDistance based on the provided information. Note that
     * the timing is updated automatically in the paintAt method, so there is no need to call this
     * method if paintAt has just been called.
     */
    void updateTiming(const KisPaintInformation &info, KisDistanceInformation &currentDistance)
        const;

    /**
     * Draw a line between pos1 and pos2 using the currently set brush and color.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the line using the spacing setting.
     *
     * @return the drag distance, that is the remains of the distance
     * between p1 and p2 not covered because the currently set brush
     * has a spacing greater than that distance.
     */
    virtual void paintLine(const KisPaintInformation &pi1,
                           const KisPaintInformation &pi2,
                           KisDistanceInformation *currentDistance);

    /**
     * Draw a Bezier curve between pos1 and pos2 using control points 1 and 2.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the curve using the spacing setting.
     * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
     * because the currently set brush has a spacing greater than that distance.
     */
    virtual void paintBezierCurve(const KisPaintInformation &pi1,
                                  const QPointF &control1,
                                  const QPointF &control2,
                                  const KisPaintInformation &pi2,
                                  KisDistanceInformation *currentDistance);


    /**
    * Whether this paintop can paint. Can be false in case that some setting isn't read correctly.
    * @return if paintop is ready for painting, default is true
    */
    virtual bool canPaint() const {
        return true;
    }

    /**
     * Split the coordinate into whole + fraction, where fraction is always >= 0.
     */
    static void splitCoordinate(qreal coordinate, qint32 *whole, qreal *fraction);

    /**
     * If the preset supports asynchronous updates, then the stroke execution core will
     * call this method with a desired frame rate. The jobs that should be run to prepare the update
     * are returned via \p jobs
     *
     * @return a pair of <the desired FPS rate (period of updates); are there any unprocessed update jobs left?>
     */
    virtual std::pair<int, bool> doAsyncronousUpdate(QVector<KisRunnableStrokeJobData*> &jobs);

protected:
    friend class KisPaintInformation;
    /**
     * The implementation of painting of a dab and updating spacing. This does NOT need to update
     * the timing information.
     */
    virtual KisSpacingInformation paintAt(const KisPaintInformation& info) = 0;

    /**
     * Implementation of a spacing update
     */
    virtual KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const = 0;

    /**
     * Implementation of a timing update. The default implementation always disables timing. This is
     * suitable for paintops that do not support airbrushing.
     */
    virtual KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const;

    KisFixedPaintDeviceSP cachedDab();
    KisFixedPaintDeviceSP cachedDab(const KoColorSpace *cs);

    /**
     * Return the painter this paintop is owned by
     */
    KisPainter* painter() const;

    /**
     * Return the paintdevice the painter this paintop is owned by
     */
    KisPaintDeviceSP source() const;

private:
    friend class KisPressureRotationOption;
    void setFanCornersInfo(bool fanCornersEnabled, qreal fanCornersStep);

private:
    Private* const d;
};


#endif // KIS_PAINTOP_H_
