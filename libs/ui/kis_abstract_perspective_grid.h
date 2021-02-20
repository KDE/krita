/*
 * This file is part of Krita
 *
 *  SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
