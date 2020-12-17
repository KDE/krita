/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOPATHMERGEUTILS_H
#define KOPATHMERGEUTILS_H

#include <boost/optional.hpp>

#include <QPointF>

class KoPathPoint;

namespace KritaUtils {

boost::optional<QPointF> fetchControlPoint(KoPathPoint *pt, bool takeFirst);
void makeSymmetric(KoPathPoint *pt, bool copyFromFirst);
void restoreControlPoint(KoPathPoint *pt, bool restoreFirst, boost::optional<QPointF> savedPoint);

}


#endif // KOPATHMERGEUTILS_H
