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

#include "kis_freehand_curve_widget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QVector2D>

#include <QDebug>

KisFreehandCurveWidget::KisFreehandCurveWidget(QWidget *parent) :
    KisCurveWidgetBase(parent), m_lastPointX(-1)
{
    reset();
}

QList<QPointF> KisFreehandCurveWidget::controlPoints() const
{
    QList<QPointF> retPoints;

    for (QMap<int, int>::const_iterator iter=m_points.begin(); iter!=(m_points.end()); iter++) {
        retPoints.append(QPointF(iter.key(), iter.value()));
    }

    return retPoints;
}

void KisFreehandCurveWidget::setControlPoints(const QList<QPointF> &points)
{
    deletePoints(0, CURVE_RANGE);

    for(int i=0; i<points.size(); i++) {
        m_points.insert(points.at(i).x(), points.at(i).y());
    }
}

void KisFreehandCurveWidget::reset()
{
    m_points.clear();

    m_points.insert(0,0);
    m_points.insert(CURVE_RANGE, CURVE_RANGE);

    update();
}

void KisFreehandCurveWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    paintBackground(&painter);

    painter.setMatrix(m_converterMatrix);

    QPainterPath path;
    QMap<int, int>::iterator firstPoint = m_points.begin();
    path.moveTo(firstPoint.key(), firstPoint.value());


    for (QMap<int, int>::iterator iter=m_points.begin(); iter!=(m_points.end()); iter++) {
        path.lineTo(iter.key(), iter.value());
    }

    painter.drawPath(path);
}

void KisFreehandCurveWidget::deletePoints(int fromX, int toX)
{
    int start = qMin(fromX, toX);
    int end = qMax(fromX, toX);

    QList<int> keys = m_points.keys();
    for(int i=0; i<keys.size(); i++) {
        if(keys.at(i)>start && keys.at(i)<end)
            m_points.remove(keys.at(i));
    }
}

void KisFreehandCurveWidget::mousePressEvent(QMouseEvent *event)
{
    QVector2D mousePos(m_converterMatrix.inverted().map(event->posF()));
    mousePos.setX(qBound(0., mousePos.x(), CURVE_RANGE));
    mousePos.setY(qBound(0., mousePos.y(), CURVE_RANGE));

    if(event->button()==Qt::LeftButton) {
        m_lastPointX = mousePos.x();
    }
}

void KisFreehandCurveWidget::mouseMoveEvent(QMouseEvent *event)
{
    QVector2D mousePos(m_converterMatrix.inverted().map(event->posF()));
    mousePos.setX(qBound(0., mousePos.x(), CURVE_RANGE));
    mousePos.setY(qBound(0., mousePos.y(), CURVE_RANGE));

    if(event->buttons()&Qt::LeftButton) {
        deletePoints(m_lastPointX, mousePos.x());
        m_points.insert(mousePos.x(), mousePos.y());
        m_lastPointX=mousePos.x();
        update();
    }
}
