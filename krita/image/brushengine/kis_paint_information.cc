/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_paint_information.h"

#include <QDomElement>

#include "kis_paintop.h"
#include "kis_distance_information.h"


struct KisPaintInformation::Private {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Private() : currentDistanceInfo(0) {}

    QPointF pos;
    qreal pressure;
    qreal xTilt;
    qreal yTilt;
    qreal rotation;
    qreal tangentialPressure;
    qreal perspective;
    int time;

    KisDistanceInformation *currentDistanceInfo;

    void registerDistanceInfo(KisDistanceInformation *di) {
        currentDistanceInfo = di;
    }

    void unregisterDistanceInfo() {
        currentDistanceInfo = 0;
    }
};

KisPaintInformation::KisPaintInformation(const QPointF & pos_,
                                         qreal pressure_,
                                         qreal xTilt_, qreal yTilt_,
                                         qreal rotation_,
                                         qreal tangentialPressure_,
                                         qreal perspective_,
                                         int time)
    : d(new Private)
{
    d->pos = pos_;
    d->pressure = pressure_;
    d->xTilt = xTilt_;
    d->yTilt = yTilt_;
    d->rotation = rotation_;
    d->tangentialPressure = tangentialPressure_;
    d->perspective = perspective_;
    d->time = time;
}

KisPaintInformation::KisPaintInformation(const KisPaintInformation& rhs) : d(new Private(*rhs.d))
{
}

void KisPaintInformation::operator=(const KisPaintInformation & rhs)
{
    *d = *rhs.d;
}

KisPaintInformation::~KisPaintInformation()
{
    delete d;
}


void KisPaintInformation::toXML(QDomDocument&, QDomElement& e) const
{
    e.setAttribute("pointX", QString::number(pos().x(), 'g', 15));
    e.setAttribute("pointY", QString::number(pos().y(), 'g', 15));
    e.setAttribute("pressure", QString::number(pressure(), 'g', 15));
    e.setAttribute("xTilt", QString::number(xTilt(), 'g', 15));
    e.setAttribute("yTilt", QString::number(yTilt(), 'g', 15));
    e.setAttribute("rotation", QString::number(rotation(), 'g', 15));
    e.setAttribute("tangentialPressure", QString::number(tangentialPressure(), 'g', 15));
    e.setAttribute("perspective", QString::number(perspective(), 'g', 15));
    e.setAttribute("time", d->time);
}

KisPaintInformation KisPaintInformation::fromXML(const QDomElement& e)
{
    qreal pointX = qreal(e.attribute("pointX", "0.0").toDouble());
    qreal pointY = qreal(e.attribute("pointY", "0.0").toDouble());
    qreal pressure = qreal(e.attribute("pressure", "0.0").toDouble());
    qreal rotation = qreal(e.attribute("rotation", "0.0").toDouble());
    qreal tangentialPressure = qreal(e.attribute("tangentialPressure", "0.0").toDouble());
    qreal perspective = qreal(e.attribute("perspective", "0.0").toDouble());
    qreal xTilt = qreal(e.attribute("xTilt", "0.0").toDouble());
    qreal yTilt = qreal(e.attribute("yTilt", "0.0").toDouble());
    int time = e.attribute("time", "0").toInt();

    return KisPaintInformation(QPointF(pointX, pointY), pressure, xTilt, yTilt,
                               rotation, tangentialPressure, perspective, time);
}

void KisPaintInformation::paintAt(KisPaintOp *op, KisDistanceInformation *distanceInfo)
{
    d->registerDistanceInfo(distanceInfo);
    KisSpacingInformation spacingInfo = op->paintAt(*this);
    d->unregisterDistanceInfo();

    distanceInfo->registerPaintedDab(*this, spacingInfo);
}

const QPointF& KisPaintInformation::pos() const
{
    return d->pos;
}

void KisPaintInformation::setPos(const QPointF& p)
{
    d->pos = p;
}

qreal KisPaintInformation::pressure() const
{
    return d->pressure;
}

void KisPaintInformation::setPressure(qreal p)
{
    d->pressure = p;
}

qreal KisPaintInformation::xTilt() const
{
    return d->xTilt;
}

qreal KisPaintInformation::yTilt() const
{
    return d->yTilt;
}

qreal KisPaintInformation::drawingAngle() const
{
    Q_ASSERT_X(d->currentDistanceInfo, "KisPaintInformation::drawingAngle()", "Distance info should be initialized *before* the painting");
    Q_ASSERT(d->currentDistanceInfo->hasLastDabInformation());

    QVector2D diff(pos() - d->currentDistanceInfo->lastPosition());
    return atan2(diff.y(), diff.x());
}

qreal KisPaintInformation::drawingDistance() const
{
    Q_ASSERT_X(d->currentDistanceInfo, "KisPaintInformation::drawingDistance()", "Distance info should be initialized *before* the painting");
    Q_ASSERT(d->currentDistanceInfo->hasLastDabInformation());

    QVector2D diff(pos() - d->currentDistanceInfo->lastPosition());
    return diff.length();
}

qreal KisPaintInformation::drawingSpeed() const
{
    Q_ASSERT_X(d->currentDistanceInfo, "KisPaintInformation::drawingSpeed()", "Distance info should be initialized *before* the painting");
    Q_ASSERT(d->currentDistanceInfo->hasLastDabInformation());

    int timeDiff = currentTime() - d->currentDistanceInfo->lastTime();

    if (timeDiff <= 0) {
        return 6.66;
    }

    QVector2D diff(pos() - d->currentDistanceInfo->lastPosition());
    return diff.length() / timeDiff;
}

qreal KisPaintInformation::rotation() const
{
    return d->rotation;
}

qreal KisPaintInformation::tangentialPressure() const
{
    return d->tangentialPressure;
}

qreal KisPaintInformation::perspective() const
{
    return d->perspective;
}

int KisPaintInformation::currentTime() const
{
    return d->time;
}

QDebug operator<<(QDebug dbg, const KisPaintInformation &info)
{
#ifdef NDEBUG
    Q_UNUSED(info);
#else
    dbg.nospace() << "Position: " << info.pos();
    dbg.nospace() << ", Pressure: " << info.pressure();
    dbg.nospace() << ", X Tilt: " << info.xTilt();
    dbg.nospace() << ", Y Tilt: " << info.yTilt();
    dbg.nospace() << ", Rotation: " << info.rotation();
    dbg.nospace() << ", Tangential Pressure: " << info.tangentialPressure();
    dbg.nospace() << ", Perspective: " << info.perspective();
    dbg.nospace() << ", Drawing Angle: " << info.drawingAngle();
    dbg.nospace() << ", Drawing Speed: " << info.drawingSpeed();
    dbg.nospace() << ", Drawing Distance: " << info.drawingDistance();
    dbg.nospace() << ", Time: " << info.currentTime();
#endif
    return dbg.space();
}

KisPaintInformation KisPaintInformation::mix(const QPointF& p, qreal t, const KisPaintInformation& pi1, const KisPaintInformation& pi2)
{
    qreal pressure = (1 - t) * pi1.pressure() + t * pi2.pressure();
    qreal xTilt = (1 - t) * pi1.xTilt() + t * pi2.xTilt();
    qreal yTilt = (1 - t) * pi1.yTilt() + t * pi2.yTilt();
    qreal rotation = (1 - t) * pi1.rotation() + t * pi2.rotation();
    qreal tangentialPressure = (1 - t) * pi1.tangentialPressure() + t * pi2.tangentialPressure();
    qreal perspective = (1 - t) * pi1.perspective() + t * pi2.perspective();
    int   time = (1 - t) * pi1.currentTime() + t * pi2.currentTime();
    return KisPaintInformation(p, pressure, xTilt, yTilt, rotation, tangentialPressure, perspective, time);
}

qreal KisPaintInformation::ascension(const KisPaintInformation& info, bool normalize)
{
    qreal xTilt = info.xTilt();
    qreal yTilt = info.yTilt();
    // radians -PI, PI
    qreal ascension = atan2(-xTilt, yTilt);
    // if normalize is true map to 0.0..1.0
    return normalize ? (ascension / (2 * M_PI) + 0.5) : ascension;
}

qreal KisPaintInformation::declination(const KisPaintInformation& info, qreal maxTiltX, qreal maxTiltY, bool normalize)
{
    qreal xTilt = qBound(qreal(-1.0), info.xTilt() / maxTiltX , qreal(1.0));
    qreal yTilt = qBound(qreal(-1.0), info.yTilt() / maxTiltY , qreal(1.0));
    
    qreal e;
    if (fabs(xTilt) > fabs(yTilt)) {
        e = sqrt(qreal(1.0) + yTilt*yTilt);
    } else {
        e = sqrt(qreal(1.0) + xTilt*xTilt);
    }
    
    qreal cosAlpha    = sqrt(xTilt*xTilt + yTilt*yTilt)/e;
    qreal declination = acos(cosAlpha); // in radians in [0, 0.5 * PI]
    
    // mapping to 0.0..1.0 if normalize is true
    return normalize ? (declination / (M_PI * qreal(0.5))) : declination;
}

