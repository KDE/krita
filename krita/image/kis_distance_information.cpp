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

#include "kis_distance_information.h"
#include "kis_paint_information.h"
#include "kis_debug.h"
#include <QtCore/qmath.h>
#include <QVector2D>
#include <QTransform>
#include "kis_algebra_2d.h"

#include "kis_paint_device.h"
#include "kis_lod_transform.h"


struct KisDistanceInformation::Private {
    Private() : lastDabInfoValid(false),
                lastPaintInfoValid(false) {}

    QPointF distance;
    KisSpacingInformation spacing;
    QPointF lastPosition;
    qreal lastTime;
    bool lastDabInfoValid;

    KisPaintInformation lastPaintInformation;
    qreal lastAngle;
    bool lastPaintInfoValid;
};

KisDistanceInformation::KisDistanceInformation()
    : m_d(new Private)
{
}

KisDistanceInformation::KisDistanceInformation(const QPointF &lastPosition,
                                               qreal lastTime)
    : m_d(new Private)
{
    m_d->lastPosition = lastPosition;
    m_d->lastTime = lastTime;

    m_d->lastDabInfoValid = true;
}

KisDistanceInformation::KisDistanceInformation(const KisDistanceInformation &rhs)
    : m_d(new Private(*rhs.m_d))
{

}

KisDistanceInformation::KisDistanceInformation(const KisDistanceInformation &rhs, int levelOfDetail)
    : m_d(new Private(*rhs.m_d))
{
    KIS_ASSERT_RECOVER_NOOP(m_d->lastDabInfoValid &&
                            !m_d->lastPaintInfoValid &&
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
    m_d->lastAngle = info.drawingAngleSafe(*this);
    m_d->lastPaintInformation = info;
    m_d->lastPaintInfoValid = true;

    m_d->lastPosition = info.pos();
    m_d->lastTime = info.currentTime();
    m_d->lastDabInfoValid = true;

    m_d->spacing = spacing;
}

qreal KisDistanceInformation::getNextPointPosition(const QPointF &start,
                                                   const QPointF &end)
{
    return m_d->spacing.isIsotropic() ?
        getNextPointPositionIsotropic(start, end) :
        getNextPointPositionAnisotropic(start, end);
}

qreal KisDistanceInformation::getNextPointPositionIsotropic(const QPointF &start,
                                                            const QPointF &end)
{
    qreal distance = m_d->distance.x();
    qreal spacing = qMax(qreal(0.5), m_d->spacing.spacing().x());

    if (start == end) {
        return -1;
    }

    qreal dragVecLength = QVector2D(end - start).length();
    qreal nextPointDistance = spacing - distance;

    qreal t;

    if (nextPointDistance <= dragVecLength) {
        t = nextPointDistance / dragVecLength;
        m_d->distance = QPointF();
    } else {
        t = -1;
        m_d->distance.rx() += dragVecLength;
    }

    return t;
}

qreal KisDistanceInformation::getNextPointPositionAnisotropic(const QPointF &start,
                                                              const QPointF &end)
{
    if (start == end) {
        return -1;
    }

    qreal a_rev = 1.0 / qMax(qreal(0.5), m_d->spacing.spacing().x());
    qreal b_rev = 1.0 / qMax(qreal(0.5), m_d->spacing.spacing().y());

    qreal x = m_d->distance.x();
    qreal y = m_d->distance.y();

    static const qreal eps = 2e-3; // < 0.2 deg
    const qreal currentRotation = m_d->spacing.rotation();

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
            m_d->distance = QPointF();
        } else {
            m_d->distance += KisAlgebra2D::abs(diff);
        }
    } else {
        qWarning() << "BUG: No solution for elliptical spacing equation has been found. This shouldn't have happened.";
    }

    return t;
}
