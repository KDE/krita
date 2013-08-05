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


struct KisDistanceInformation::Private {
    Private() : lastDabInfoValid(false) {}

    QPointF distance;
    KisSpacingInformation spacing;
    QPointF lastPosition;
    int lastTime;
    bool lastDabInfoValid;
};

KisDistanceInformation::KisDistanceInformation()
    : m_d(new Private)
{
}

KisDistanceInformation::KisDistanceInformation(const QPointF &lastPosition,
                                               int lastTime)
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

int KisDistanceInformation::lastTime() const
{
    return m_d->lastTime;
}

void KisDistanceInformation::registerPaintedDab(const KisPaintInformation &info,
                                                const KisSpacingInformation &spacing)
{
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
    qreal spacing = qMax(0.5, m_d->spacing.spacing().x());

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

    qreal a_rev = 1.0 / qMax(0.5, m_d->spacing.spacing().x());
    qreal b_rev = 1.0 / qMax(0.5, m_d->spacing.spacing().y());

    qreal x = m_d->distance.x();
    qreal y = m_d->distance.y();

    qreal dx = qAbs(end.x() - start.x());
    qreal dy = qAbs(end.y() - start.y());

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
            m_d->distance += qAbs(end - start);
        }
    } else {
        qWarning() << "BUG: No solution for elliptical spacing equation has been found. This shouldn't have happened.";
    }

    return t;
}
