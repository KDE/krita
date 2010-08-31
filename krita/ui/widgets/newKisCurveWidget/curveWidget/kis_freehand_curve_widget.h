/* This file is part of the KDE project
 * Copyright (C) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_FREEHAND_CURVE_WIDGET_H
#define KIS_FREEHAND_CURVE_WIDGET_H

#include "kis_curve_widget_base.h"

#include <QMap>

class KisFreehandCurveWidget : public KisCurveWidgetBase {
    Q_OBJECT;
public:
    KisFreehandCurveWidget(QWidget *parent = 0);

    virtual QList<QPointF> controlPoints() const;
    virtual void setControlPoints(const QList<QPointF> &points);

public slots:
    void reset();

protected:
    void paintEvent(QPaintEvent *);
    void deletePoints(int fromX, int toX);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *) {} //do nothing, overide superclass
    void mouseReleaseEvent(QMouseEvent *) {}     //same here

private:
    QMap<int, int> m_points;
    int m_lastPointX;
};

#endif // KIS_FREEHAND_CURVE_WIDGET_H
