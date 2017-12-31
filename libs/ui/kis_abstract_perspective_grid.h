/*
 * This file is part of Krita
 *
 *  Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
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

#ifndef KIS_ABSTRACT_PERSPECTIVE_GRID_H
#define KIS_ABSTRACT_PERSPECTIVE_GRID_H

#include <QPointF>
#include <QObject>

#include "kritaui_export.h"

class KRITAUI_EXPORT KisAbstractPerspectiveGrid : public QObject
{
    Q_OBJECT
public:

    KisAbstractPerspectiveGrid(QObject * parent = 0);

    ~KisAbstractPerspectiveGrid() override {}

    virtual bool contains(const QPointF& pt) const = 0;
    /**
     * Returns the reciprocal of the distance from the given point
     * to the 'observer', in the range [0, 1] where 0 = infinite
     * distance and 1 = closest.
     */
    virtual qreal distance(const QPointF& pt) const = 0;
};

#endif
