/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_POLYGON_H_
#define _KIS_POLYGON_H_

#include <qvaluevector.h>

typedef QValueVector<KisPoint> KisPointVector;
class KisPolygon : public KisPointVector
{
        public:
                void addPoint(double x, double y);
                void translate(double tx, double ty);
                void rotate(double theta);
                Q_INT32 extents(double &minX, double &minY, double &maxX, double &maxY);
                Q_INT32 numberOfPoints();
};

#endif
