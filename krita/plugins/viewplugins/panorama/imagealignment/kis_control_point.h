/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_CONTROL_POINT_H_
#define _KIS_CONTROL_POINT_H_

#include <QList>
#include <QVector>
#include <QPointF>

class KisInterestPoint;

struct KisControlPoint {
  KisControlPoint(int size = 0) : positions(size), interestPoints(size)
  {
  }
  /// list of index of images where this point was seen
  QList<int> frames;
  /// list of postion in the image of a this point
  QVector<QPointF> positions;
  QVector<const KisInterestPoint*> interestPoints;
};

#endif
