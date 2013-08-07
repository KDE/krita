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

#include <QDebug>
#include <QTime>

#include "kis_global.h"
#include "kis_vec.h"
#include "krita_export.h"

class QDomDocument;
class QDomElement;
class KisPaintOp;
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
     * Create a new KisPaintInformation object.

     */
    KisPaintInformation(const QPointF & pos = QPointF(),
                        qreal pressure = PRESSURE_DEFAULT,
                        qreal xTilt = 0.0,
                        qreal yTilt = 0.0,
                        qreal rotation = 0.0,
                        qreal tangentialPressure = 0.0,
                        qreal perspective = 1.0,
                        int   time = 0);

    KisPaintInformation(const KisPaintInformation& rhs);

    void operator=(const KisPaintInformation& rhs);

    ~KisPaintInformation();

    void paintAt(KisPaintOp *op, KisDistanceInformation *distanceInfo);

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

    void overrideDrawingAngle(qreal angle);

    qreal drawingAngleSafe(const KisDistanceInformation &distance) const;

    /**
     * Current brush direction computed from the cursor movement
     *
     * WARNING: this method is available *only* inside paintAt() call,
     * that is when the distance information is registered.
     */
    qreal drawingAngle() const;

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
    int currentTime() const;

    void toXML(QDomDocument&, QDomElement&) const;

    static KisPaintInformation fromXML(const QDomElement&);
    
    /// (1-t) * p1 + t * p2
    static KisPaintInformation mix(const QPointF& p, qreal t, const KisPaintInformation& p1, const KisPaintInformation& p2);
    static qreal ascension(const KisPaintInformation& info, bool normalize=true);
    static qreal declination(const KisPaintInformation& info, qreal maxTiltX=60.0, qreal maxTiltY=60.0, bool normalize=true);

private:
    struct Private;
    Private* const d;
};

KRITAIMAGE_EXPORT QDebug operator<<(QDebug debug, const KisPaintInformation& info);


#endif
