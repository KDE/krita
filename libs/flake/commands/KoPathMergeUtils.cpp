/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoPathMergeUtils.h"

#include "KoPathPoint.h"

boost::optional<QPointF> KritaUtils::fetchControlPoint(KoPathPoint *pt, bool takeFirst)
{
    boost::optional<QPointF> result;

    if (takeFirst) {
        if (pt->activeControlPoint1()) {
            result = pt->controlPoint1();
        }
    } else {
        if (pt->activeControlPoint2()) {
            result = pt->controlPoint2();
        }
    }

    return result;
}

void KritaUtils::makeSymmetric(KoPathPoint *pt, bool copyFromFirst)
{
    if (copyFromFirst) {
        if (pt->activeControlPoint1()) {
            pt->setControlPoint2(2.0 * pt->point() - pt->controlPoint1());
        }
    } else {
        if (pt->activeControlPoint2()) {
            pt->setControlPoint1(2.0 * pt->point() - pt->controlPoint2());
        }
    }

    pt->setProperty(KoPathPoint::IsSymmetric);
}

void KritaUtils::restoreControlPoint(KoPathPoint *pt, bool restoreFirst, boost::optional<QPointF> savedPoint)
{
    if (restoreFirst) {
        if (savedPoint) {
            pt->setControlPoint1(*savedPoint);
        } else {
            pt->removeControlPoint1();
        }
    } else {
        if (savedPoint) {
            pt->setControlPoint2(*savedPoint);
        } else {
            pt->removeControlPoint2();
        }
    }
}
