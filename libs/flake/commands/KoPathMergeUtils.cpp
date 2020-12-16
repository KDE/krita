/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
