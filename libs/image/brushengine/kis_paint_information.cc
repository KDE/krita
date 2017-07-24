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

#include <brushengine/kis_paint_information.h>

#include <QDomElement>
#include <boost/optional.hpp>

#include "kis_paintop.h"
#include "kis_algebra_2d.h"
#include "kis_lod_transform.h"
#include "kis_spacing_information.h"

#include <kis_dom_utils.h>

struct KisPaintInformation::Private {
    Private(const QPointF & pos_,
            qreal pressure_,
            qreal xTilt_, qreal yTilt_,
            qreal rotation_,
            qreal tangentialPressure_,
            qreal perspective_,
            qreal time_,
            qreal speed_,
            bool isHoveringMode_)
        :
        pos(pos_),
        pressure(pressure_),
        xTilt(xTilt_),
        yTilt(yTilt_),
        rotation(rotation_),
        tangentialPressure(tangentialPressure_),
        perspective(perspective_),
        time(time_),
        speed(speed_),
        isHoveringMode(isHoveringMode_),
        randomSource(0),
        currentDistanceInfo(0),
        levelOfDetail(0)
    {
    }



    ~Private() {
        KIS_ASSERT_RECOVER_NOOP(!currentDistanceInfo);
    }
    Private(const Private &rhs) {
        copy(rhs);
    }
    Private& operator=(const Private &rhs) {
        copy(rhs);
        return *this;
    }

    void copy(const Private &rhs) {
        pos = rhs.pos;
        pressure = rhs.pressure;
        xTilt = rhs.xTilt;
        yTilt = rhs.yTilt;
        rotation = rhs.rotation;
        tangentialPressure = rhs.tangentialPressure;
        perspective = rhs.perspective;
        time = rhs.time;
        speed = rhs.speed;
        isHoveringMode = rhs.isHoveringMode;
        randomSource = rhs.randomSource;
        currentDistanceInfo = rhs.currentDistanceInfo;
        canvasRotation = rhs.canvasRotation;
        canvasMirroredH = rhs.canvasMirroredH;
        if (rhs.drawingAngleOverride) {
            drawingAngleOverride = *rhs.drawingAngleOverride;
        }

        levelOfDetail = rhs.levelOfDetail;
    }


    QPointF pos;
    qreal pressure;
    qreal xTilt;
    qreal yTilt;
    qreal rotation;
    qreal tangentialPressure;
    qreal perspective;
    qreal time;
    qreal speed;
    bool isHoveringMode;
    KisRandomSourceSP randomSource;
    int canvasRotation;
    bool canvasMirroredH;

    boost::optional<qreal> drawingAngleOverride;
    KisDistanceInformation *currentDistanceInfo;

    int levelOfDetail;

    void registerDistanceInfo(KisDistanceInformation *di) {
        currentDistanceInfo = di;
    }

    void unregisterDistanceInfo() {
        currentDistanceInfo = 0;
    }
};

KisPaintInformation::DistanceInformationRegistrar::
DistanceInformationRegistrar(KisPaintInformation *_p, KisDistanceInformation *distanceInfo)
    : p(_p)
{
    p->d->registerDistanceInfo(distanceInfo);
}

KisPaintInformation::DistanceInformationRegistrar::
~DistanceInformationRegistrar()
{
    p->d->unregisterDistanceInfo();
}

KisPaintInformation::KisPaintInformation(const QPointF & pos,
                                         qreal pressure,
                                         qreal xTilt, qreal yTilt,
                                         qreal rotation,
                                         qreal tangentialPressure,
                                         qreal perspective,
                                         qreal time,
                                         qreal speed)
    : d(new Private(pos,
                    pressure,
                    xTilt, yTilt,
                    rotation,
                    tangentialPressure,
                    perspective,
                    time,
                    speed,
                    false))
{
}

KisPaintInformation::KisPaintInformation(const QPointF & pos,
                                         qreal pressure,
                                         qreal xTilt,
                                         qreal yTilt,
                                         qreal rotation)
    : d(new Private(pos,
                    pressure,
                    xTilt, yTilt,
                    rotation,
                    0.0,
                    1.0,
                    0.0,
                    0.0,
                    false))
{

}

KisPaintInformation::KisPaintInformation(const QPointF &pos,
                                         qreal pressure)
    : d(new Private(pos,
                    pressure,
                    0.0, 0.0,
                    0.0,
                    0.0,
                    1.0,
                    0.0,
                    0.0,
                    false))
{
}

KisPaintInformation::KisPaintInformation(const KisPaintInformation& rhs)
    : d(new Private(*rhs.d))
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

bool KisPaintInformation::isHoveringMode() const
{
    return d->isHoveringMode;
}


KisPaintInformation
KisPaintInformation::createHoveringModeInfo(const QPointF &pos,
        qreal pressure,
        qreal xTilt, qreal yTilt,
        qreal rotation,
        qreal tangentialPressure,
        qreal perspective,
        qreal speed,
        int canvasrotation,
        bool canvasMirroredH)
{
    KisPaintInformation info(pos,
                             pressure,
                             xTilt, yTilt,
                             rotation,
                             tangentialPressure,
                             perspective, 0, speed);
    info.d->isHoveringMode = true;
    info.d->canvasRotation = canvasrotation;
    info.d->canvasMirroredH = canvasMirroredH;
    return info;
}


int KisPaintInformation::canvasRotation() const
{
    return d->canvasRotation;
}

void KisPaintInformation::setCanvasRotation(int rotation)
{
    if (rotation < 0) {
        d->canvasRotation= 360- abs(rotation % 360);
    } else {
        d->canvasRotation= rotation % 360;
    }

}

bool KisPaintInformation::canvasMirroredH() const
{
    return d->canvasMirroredH;
}

void KisPaintInformation::setCanvasHorizontalMirrorState(bool mir)
{
    d->canvasMirroredH = mir;

}

void KisPaintInformation::toXML(QDomDocument&, QDomElement& e) const
{
    // hovering mode infos are not supposed to be saved
    KIS_ASSERT_RECOVER_NOOP(!d->isHoveringMode);

    e.setAttribute("pointX", QString::number(pos().x(), 'g', 15));
    e.setAttribute("pointY", QString::number(pos().y(), 'g', 15));
    e.setAttribute("pressure", QString::number(pressure(), 'g', 15));
    e.setAttribute("xTilt", QString::number(xTilt(), 'g', 15));
    e.setAttribute("yTilt", QString::number(yTilt(), 'g', 15));
    e.setAttribute("rotation", QString::number(rotation(), 'g', 15));
    e.setAttribute("tangentialPressure", QString::number(tangentialPressure(), 'g', 15));
    e.setAttribute("perspective", QString::number(perspective(), 'g', 15));
    e.setAttribute("time", QString::number(d->time, 'g', 15));
    e.setAttribute("speed", QString::number(d->speed, 'g', 15));
}

KisPaintInformation KisPaintInformation::fromXML(const QDomElement& e)
{
    qreal pointX = qreal(KisDomUtils::toDouble(e.attribute("pointX", "0.0")));
    qreal pointY = qreal(KisDomUtils::toDouble(e.attribute("pointY", "0.0")));
    qreal pressure = qreal(KisDomUtils::toDouble(e.attribute("pressure", "0.0")));
    qreal rotation = qreal(KisDomUtils::toDouble(e.attribute("rotation", "0.0")));
    qreal tangentialPressure = qreal(KisDomUtils::toDouble(e.attribute("tangentialPressure", "0.0")));
    qreal perspective = qreal(KisDomUtils::toDouble(e.attribute("perspective", "0.0")));
    qreal xTilt = qreal(KisDomUtils::toDouble(e.attribute("xTilt", "0.0")));
    qreal yTilt = qreal(KisDomUtils::toDouble(e.attribute("yTilt", "0.0")));
    qreal time = KisDomUtils::toDouble(e.attribute("time", "0"));
    qreal speed = KisDomUtils::toDouble(e.attribute("speed", "0"));

    return KisPaintInformation(QPointF(pointX, pointY), pressure, xTilt, yTilt,
                               rotation, tangentialPressure, perspective, time, speed);
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

void KisPaintInformation::overrideDrawingAngle(qreal angle)
{
    d->drawingAngleOverride = angle;
}

qreal KisPaintInformation::drawingAngleSafe(const KisDistanceInformation &distance) const
{
    if (d->drawingAngleOverride) return *d->drawingAngleOverride;

    if (!distance.hasLastDabInformation()) {
        warnKrita << "KisPaintInformation::drawingAngleSafe()" << "Cannot access Distance Info last dab data";
        return 0.0;
    }

    return distance.nextDrawingAngle(pos());
}

KisPaintInformation::DistanceInformationRegistrar
KisPaintInformation::registerDistanceInformation(KisDistanceInformation *distance)
{
    return DistanceInformationRegistrar(this, distance);
}

qreal KisPaintInformation::drawingAngle() const
{
    if (d->drawingAngleOverride) return *d->drawingAngleOverride;

    if (!d->currentDistanceInfo || !d->currentDistanceInfo->hasLastDabInformation()) {
        warnKrita << "KisPaintInformation::drawingAngle()" << "Cannot access Distance Info last dab data";
        return 0.0;
    }

    return d->currentDistanceInfo->nextDrawingAngle(pos());
}

void KisPaintInformation::lockCurrentDrawingAngle(qreal alpha_unused) const
{
    Q_UNUSED(alpha_unused);

    if (!d->currentDistanceInfo || !d->currentDistanceInfo->hasLastDabInformation()) {
        warnKrita << "KisPaintInformation::lockCurrentDrawingAngle()" << "Cannot access Distance Info last dab data";
        return;
    }

    qreal angle = d->currentDistanceInfo->nextDrawingAngle(pos(), false);

    qreal newAngle = angle;

    if (d->currentDistanceInfo->hasLockedDrawingAngle()) {
        const qreal stabilizingCoeff = 20.0;
        const qreal dist = stabilizingCoeff * d->currentDistanceInfo->currentSpacing().scalarApprox();
        const qreal alpha = qMax(0.0, dist - d->currentDistanceInfo->scalarDistanceApprox()) / dist;

        const qreal oldAngle = d->currentDistanceInfo->lockedDrawingAngle();

        if (shortestAngularDistance(oldAngle, newAngle) < M_PI / 6) {
            newAngle = (1.0 - alpha) * oldAngle + alpha * newAngle;
        } else {
            newAngle = oldAngle;
        }
    }

    d->currentDistanceInfo->setLockedDrawingAngle(newAngle);
}

QPointF KisPaintInformation::drawingDirectionVector() const
{
    if (d->drawingAngleOverride) {
        qreal angle = *d->drawingAngleOverride;
        return QPointF(cos(angle), sin(angle));
    }

    if (!d->currentDistanceInfo || !d->currentDistanceInfo->hasLastDabInformation()) {
        warnKrita << "KisPaintInformation::drawingDirectionVector()" << "Cannot access Distance Info last dab data";
        return QPointF(1.0, 0.0);
    }

    return d->currentDistanceInfo->nextDrawingDirectionVector(pos());
}

qreal KisPaintInformation::drawingDistance() const
{
    if (!d->currentDistanceInfo || !d->currentDistanceInfo->hasLastDabInformation()) {
        warnKrita << "KisPaintInformation::drawingDistance()" << "Cannot access Distance Info last dab data";
        return 1.0;
    }

    QVector2D diff(pos() - d->currentDistanceInfo->lastPosition());
    qreal length = diff.length();

    if (d->levelOfDetail) {
        length *= KisLodTransform::lodToInvScale(d->levelOfDetail);
    }

    return length;
}

qreal KisPaintInformation::drawingSpeed() const
{
    return d->speed;
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

qreal KisPaintInformation::currentTime() const
{
    return d->time;
}

KisRandomSourceSP KisPaintInformation::randomSource() const
{
    if (!d->randomSource) {
        d->randomSource = new KisRandomSource();
    }

    return d->randomSource;
}

void KisPaintInformation::setRandomSource(KisRandomSourceSP value)
{
    d->randomSource = value;
}

void KisPaintInformation::setLevelOfDetail(int levelOfDetail)
{
    d->levelOfDetail = levelOfDetail;
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

KisPaintInformation KisPaintInformation::mixOnlyPosition(qreal t, const KisPaintInformation& mixedPi, const KisPaintInformation& basePi)
{
    QPointF pt = (1 - t) * mixedPi.pos() + t * basePi.pos();
    return mixImpl(pt, t, mixedPi, basePi, true, false);
}

KisPaintInformation KisPaintInformation::mix(qreal t, const KisPaintInformation& pi1, const KisPaintInformation& pi2)
{
    QPointF pt = (1 - t) * pi1.pos() + t * pi2.pos();
    return mix(pt, t, pi1, pi2);
}

KisPaintInformation KisPaintInformation::mix(const QPointF& p, qreal t, const KisPaintInformation& pi1, const KisPaintInformation& pi2)
{
    return mixImpl(p, t, pi1, pi2, false, true);
}

KisPaintInformation KisPaintInformation::mixWithoutTime(qreal t, const KisPaintInformation& pi1, const KisPaintInformation& pi2)
{
    QPointF pt = (1 - t) * pi1.pos() + t * pi2.pos();
    return mixWithoutTime(pt, t, pi1, pi2);
}

KisPaintInformation KisPaintInformation::mixWithoutTime(const QPointF& p, qreal t, const KisPaintInformation& pi1, const KisPaintInformation& pi2)
{
    return mixImpl(p, t, pi1, pi2, false, false);
}

void KisPaintInformation::mixOtherOnlyPosition(qreal t, const KisPaintInformation& other)
{
    QPointF pt = (1 - t) * other.pos() + t * this->pos();
    this->mixOtherImpl(pt, t, other, true, false);
}

void KisPaintInformation::mixOtherWithoutTime(qreal t, const KisPaintInformation& other)
{
    QPointF pt = (1 - t) * other.pos() + t * this->pos();
    this->mixOtherImpl(pt, t, other, false, false);
}

KisPaintInformation KisPaintInformation::mixImpl(const QPointF &p, qreal t, const KisPaintInformation &pi1, const KisPaintInformation &pi2, bool posOnly, bool mixTime)
{
    KisPaintInformation result(pi2);
    result.mixOtherImpl(p, t, pi1, posOnly, mixTime);
    return result;
}

void KisPaintInformation::mixOtherImpl(const QPointF &p, qreal t, const KisPaintInformation &other, bool posOnly, bool mixTime)
{
    if (posOnly) {
        this->d->pos = p;
        this->d->isHoveringMode = false;
        this->d->currentDistanceInfo = 0;
        this->d->levelOfDetail = 0;
        return;
    }
    else {
        qreal pressure = (1 - t) * other.pressure() + t * this->pressure();
        qreal xTilt = (1 - t) * other.xTilt() + t * this->xTilt();
        qreal yTilt = (1 - t) * other.yTilt() + t * this->yTilt();

        qreal rotation = other.rotation();

        if (other.rotation() != this->rotation()) {
            qreal a1 = kisDegreesToRadians(other.rotation());
            qreal a2 = kisDegreesToRadians(this->rotation());
            qreal distance = shortestAngularDistance(a2, a1);

            rotation = kisRadiansToDegrees(incrementInDirection(a1, t * distance, a2));
        }

        qreal tangentialPressure = (1 - t) * other.tangentialPressure() + t * this->tangentialPressure();
        qreal perspective = (1 - t) * other.perspective() + t * this->perspective();
        qreal time = mixTime ? ((1 - t) * other.currentTime() + t * this->currentTime()) : this->currentTime();
        qreal speed = (1 - t) * other.drawingSpeed() + t * this->drawingSpeed();

        KIS_ASSERT_RECOVER_NOOP(other.isHoveringMode() == this->isHoveringMode());
        *(this->d) = Private(p, pressure, xTilt, yTilt, rotation, tangentialPressure, perspective, time, speed, other.isHoveringMode());
        this->d->randomSource = other.d->randomSource;
        // this->d->isHoveringMode = other.isHoveringMode();
        this->d->currentDistanceInfo = 0;
        this->d->levelOfDetail = other.d->levelOfDetail;
    }
}

qreal KisPaintInformation::tiltDirection(const KisPaintInformation& info, bool normalize)
{
    qreal xTilt = info.xTilt();
    qreal yTilt = info.yTilt();
    // radians -PI, PI
    qreal tiltDirection = atan2(-xTilt, yTilt);
    // if normalize is true map to 0.0..1.0
    return normalize ? (tiltDirection / (2 * M_PI) + 0.5) : tiltDirection;
}

qreal KisPaintInformation::tiltElevation(const KisPaintInformation& info, qreal maxTiltX, qreal maxTiltY, bool normalize)
{
    qreal xTilt = qBound(qreal(-1.0), info.xTilt() / maxTiltX , qreal(1.0));
    qreal yTilt = qBound(qreal(-1.0), info.yTilt() / maxTiltY , qreal(1.0));

    qreal e;
    if (fabs(xTilt) > fabs(yTilt)) {
        e = sqrt(qreal(1.0) + yTilt * yTilt);
    } else {
        e = sqrt(qreal(1.0) + xTilt * xTilt);
    }

    qreal cosAlpha    = sqrt(xTilt * xTilt + yTilt * yTilt) / e;
    qreal tiltElevation = acos(cosAlpha); // in radians in [0, 0.5 * PI]

    // mapping to 0.0..1.0 if normalize is true
    return normalize ? (tiltElevation / (M_PI * qreal(0.5))) : tiltElevation;
}
