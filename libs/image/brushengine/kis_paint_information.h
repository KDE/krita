/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_PAINT_INFORMATION_
#define _KIS_PAINT_INFORMATION_

#include <kis_debug.h>
#include <QTime>

#include "kis_global.h"
#include "kritaimage_export.h"
#include <kis_distance_information.h>
#include "kis_random_source.h"
#include "KisPerStrokeRandomSource.h"
#include "kis_spacing_information.h"
#include "kis_timing_information.h"


class QDomDocument;
class QDomElement;
class KisDistanceInformation;


/**
 * KisPaintInformation contains information about the input event that
 * causes the brush action to happen to the brush engine's paint
 * methods.
 *
 * XXX: we directly pass the KoPointerEvent x and y tilt to
 * KisPaintInformation, and their range is -60 to +60!
 *
 * @param pos: the position of the paint event in subpixel accuracy
 * @param pressure: the pressure of the stylus
 * @param xTilt: the angle between the device (a pen, for example) and
 * the perpendicular in the direction of the x axis. Positive values
 * are towards the bottom of the tablet. The angle is within the range
 * 0 to 1
 * @param yTilt: the angle between the device (a pen, for example) and
 * the perpendicular in the direction of the y axis. Positive values
 * are towards the bottom of the tablet. The angle is within the range
 * 0 to .
 * @param movement: current position minus the last position of the call to paintAt
 * @param rotation
 * @param tangentialPressure
 * @param perspective
 **/
class KRITAIMAGE_EXPORT KisPaintInformation
{
public:
    /**
     * Note, that this class is relied on the compiler optimization
     * of the return value. So if it doesn't work for some reason,
     * please implement a proper copy c-tor
     */
    class KRITAIMAGE_EXPORT DistanceInformationRegistrar
    {
    public:
        DistanceInformationRegistrar(KisPaintInformation *_p, KisDistanceInformation *distanceInfo);
        DistanceInformationRegistrar(const DistanceInformationRegistrar &rhs) = delete;
        DistanceInformationRegistrar(DistanceInformationRegistrar &&rhs);
        ~DistanceInformationRegistrar();
    private:
        KisPaintInformation *p;
    };

public:

    /**
     * Create a new KisPaintInformation object.
     */
    KisPaintInformation(const QPointF & pos,
                        qreal pressure,
                        qreal xTilt,
                        qreal yTilt,
                        qreal rotation,
                        qreal tangentialPressure,
                        qreal perspective,
                        qreal time,
                        qreal speed);

    KisPaintInformation(const QPointF & pos,
                        qreal pressure,
                        qreal xTilt,
                        qreal yTilt,
                        qreal rotation);

    KisPaintInformation(const QPointF & pos = QPointF(),
                        qreal pressure = PRESSURE_DEFAULT);

    KisPaintInformation(const KisPaintInformation& rhs);

    void operator=(const KisPaintInformation& rhs);

    ~KisPaintInformation();

    template <class PaintOp>
    void paintAt(PaintOp &op, KisDistanceInformation *distanceInfo) {
        KisSpacingInformation spacingInfo;
        KisTimingInformation timingInfo;
        {
            DistanceInformationRegistrar r = registerDistanceInformation(distanceInfo);
            spacingInfo = op.paintAt(*this);
            timingInfo = op.updateTimingImpl(*this);

            // Initiate the process of locking the drawing angle. The locked value will
            // always be present in the internals, but it will be requested but the users
            // with a special parameter of drawingAngle() only.
            if (!this->isHoveringMode()) {
                distanceInfo->lockCurrentDrawingAngle(*this);
            }
        }

        distanceInfo->registerPaintedDab(*this, spacingInfo, timingInfo);
    }

    const QPointF& pos() const;
    void setPos(const QPointF& p);

    /// The pressure of the value (from 0.0 to 1.0)
    qreal pressure() const;

    /// Set the pressure
    void setPressure(qreal p);

    /// The tilt of the pen on the horizontal axis (from 0.0 to 1.0)
    qreal xTilt() const;

    /// The tilt of the pen on the vertical axis (from 0.0 to 1.0)
    qreal yTilt() const;

    /// XXX !!! :-| Please add dox!
    void overrideDrawingAngle(qreal angle);

    /// XXX !!! :-| Please add dox!
    qreal drawingAngleSafe(const KisDistanceInformation &distance) const;

    /**
     * Causes the specified distance information to be temporarily registered with this
     * KisPaintInformation object, so that the KisPaintInformation can compute certain values that
     * may be needed at painting time, such as the drawing direction. When the returned object is
     * destroyed, the KisDistanceInformation will be unregistered. At most one
     * KisDistanceInformation can be registered with a given KisPaintInformation at a time.
     */
    DistanceInformationRegistrar registerDistanceInformation(KisDistanceInformation *distance);

    /**
     * Current brush direction computed from the cursor movement
     *
     * WARNING: this method is available *only* inside paintAt() call,
     * that is when the distance information is registered.
     */
    qreal drawingAngle(bool considerLockedAngle = false) const;

    /**
     * Current brush direction vector computed from the cursor movement
     *
     * WARNING: this method is available *only* inside paintAt() call,
     * that is when the distance information is registered.
     */
    QPointF drawingDirectionVector() const;

    /**
     * Current brush speed computed from the cursor movement
     *
     * WARNING: this method is available *only* inside paintAt() call,
     * that is when the distance information is registered.
     */
    qreal drawingSpeed() const;

    /**
     * Current distance from the previous dab
     *
     * WARNING: this method is available *only* inside paintAt() call,
     * that is when the distance information is registered.
     */
    qreal drawingDistance() const;

    /// rotation as given by the tablet event
    qreal rotation() const;

    /// tangential pressure (i.e., rate for an airbrush device)
    qreal tangentialPressure() const;

    /// reciprocal of distance on the perspective grid
    qreal perspective() const;

    /// Number of ms since the beginning of the stroke
    qreal currentTime() const;

    /// Number of dabs painted since the beginning of the stroke
    int currentDabSeqNo() const;

    /// The length of the stroke **before** painting the current dab
    qreal totalStrokeLength() const;

    // random source for generating in-stroke effects
    KisRandomSourceSP randomSource() const;

    // the stroke should initialize random source of all the used
    // paint info objects, otherwise it shows a warning
    void setRandomSource(KisRandomSourceSP value);

    // random source for generating in-stroke effects, generates one(!) value per stroke
    KisPerStrokeRandomSourceSP perStrokeRandomSource() const;

    // the stroke should initialize per stroke random source of all the used
    // paint info objects, otherwise it shows a warning
    void setPerStrokeRandomSource(KisPerStrokeRandomSourceSP value);

    // set level of detail which info object has been generated for
    void setLevelOfDetail(int levelOfDetail);

    /**
     * The paint information may be generated not only during real
     * stroke when the actual painting is happening, but also when the
     * cursor is hovering the canvas. In this mode some of the sensors
     * work a bit differently. The most outstanding example is Fuzzy
     * sensor, which returns unit value in this mode, otherwise it is
     * too irritating for a user.
     *
     * This value is true only for paint information objects created with
     * createHoveringModeInfo() constructor.
     *
     * \see createHoveringModeInfo()
     */
    bool isHoveringMode() const;

    /**
     * Create a fake info object with isHoveringMode() property set to
     * true.
     *
     * \see isHoveringMode()
     */
    static KisPaintInformation createHoveringModeInfo(const QPointF &pos,
            qreal pressure = PRESSURE_DEFAULT,
            qreal xTilt = 0.0, qreal yTilt = 0.0,
            qreal rotation = 0.0,
            qreal tangentialPressure = 0.0,
            qreal perspective = 1.0,
	        qreal speed = 0.0,
            int canvasrotation = 0,
            bool canvasMirroredH = false);
    /**
     *Returns the canvas rotation if that has been given to the kispaintinformation.
     */
    int canvasRotation() const;
    /**
     *set the canvas rotation.
     */
    void setCanvasRotation(int rotation);

    /*
     *Whether the canvas is mirrored for the paint-operation.
     */
    bool canvasMirroredH() const;

    /*
     *Set whether the canvas is mirrored for the paint-operation.
     */
    void setCanvasHorizontalMirrorState(bool mir);

    void toXML(QDomDocument&, QDomElement&) const;

    static KisPaintInformation fromXML(const QDomElement&);

    // TODO: Refactor the static mix functions to non-static in-place mutation
    //       versions like mixOtherOnlyPosition and mixOtherWithoutTime.
    // Heap allocation on Windows is awfully slow and will fragment the memory
    // badly. Since KisPaintInformation allocates on the heap, we should re-use
    // existing instance whenever possible, especially in loops.
    // Ref: https://phabricator.kde.org/D6578

    /// (1-t) * p1 + t * p2
    static KisPaintInformation mixOnlyPosition(qreal t, const KisPaintInformation& mixedPi, const KisPaintInformation& basePi);
    static KisPaintInformation mix(const QPointF& p, qreal t, const KisPaintInformation& p1, const KisPaintInformation& p2);
    static KisPaintInformation mix(qreal t, const KisPaintInformation& pi1, const KisPaintInformation& pi2);
    static KisPaintInformation mixWithoutTime(const QPointF &p, qreal t, const KisPaintInformation &p1, const KisPaintInformation &p2);
    static KisPaintInformation mixWithoutTime(qreal t, const KisPaintInformation &pi1, const KisPaintInformation &pi2);
    void mixOtherOnlyPosition(qreal t, const KisPaintInformation& other);
    void mixOtherWithoutTime(qreal t, const KisPaintInformation& other);
    static qreal tiltDirection(const KisPaintInformation& info, bool normalize = true);
    static qreal tiltElevation(const KisPaintInformation& info, qreal maxTiltX = 60.0, qreal maxTiltY = 60.0, bool normalize = true);

private:
    static KisPaintInformation mixImpl(const QPointF &p, qreal t, const KisPaintInformation &p1, const KisPaintInformation &p2, bool posOnly, bool mixTime);
    void mixOtherImpl(const QPointF &p, qreal t, const KisPaintInformation &other, bool posOnly, bool mixTime);

private:
    struct Private;
    Private* const d;
};

KRITAIMAGE_EXPORT QDebug operator<<(QDebug debug, const KisPaintInformation& info);


#endif
