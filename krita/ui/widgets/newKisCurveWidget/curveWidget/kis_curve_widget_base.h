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

#ifndef KIS_CURVE_WIDGET_BASE_H
#define KIS_CURVE_WIDGET_BASE_H

#include <QtGui/QWidget>

class QPainter;
class QVector2D;


class KisCurveWidgetBase : public QWidget
{
    Q_OBJECT

protected:
    const qreal CURVE_RANGE;

public:
    KisCurveWidgetBase(QWidget *parent = 0);
    ~KisCurveWidgetBase();

    virtual QList<QPointF> controlPoints() const;
    virtual void setControlPoints(const QList<QPointF> &points);

public slots:
    virtual void reset();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);

    void paintBlips(QPainter* painter);
    void paintBackground(QPainter* painter);

    void addPoint(const QVector2D& pos);
    // returns true, if pos is a point, also if the point is not removed.
    bool removePoint(const QVector2D& pos);

    QList<QPointF> m_points;
    QMatrix m_converterMatrix;

private:
    int m_currentPoint;
};

#endif // KIS_CURVE_WIDGET_BASE_H
