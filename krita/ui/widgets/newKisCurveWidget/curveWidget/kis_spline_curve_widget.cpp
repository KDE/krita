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

#include "kis_spline_curve_widget.h"

#include <cmath>
#include <QVector2D>
#include <QPainter>
#include <QPainterPath>

#include <QDebug>

#include "kis_spline_curve.h"

KisSplineCurveWidget::KisSplineCurveWidget(QWidget *parent) :
    KisCurveWidgetBase(parent)
{
}

void KisSplineCurveWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    paintBackground(&painter);
    painter.setMatrix(m_converterMatrix);

    KisSplineCurve curve;
    curve.setPoints(m_points);
    curve.updatePainterPath();
    painter.drawPath(curve.painterPath());

    paintBlips(&painter);

    // debug output:
//    for(int i=0; i<controlPoints.size(); i++) {
//        painter.drawEllipse(controlPoints.at(i).first.toPoint(), 2, 2);
//        painter.drawText(controlPoints.at(i).first.toPoint(), QString::number(i));
//        painter.drawEllipse(controlPoints.at(i).second.toPoint(), 3, 3);
//        painter.drawEllipse(controlPoints.at(i).second.toPoint(), 1, 1);
//        painter.drawText(controlPoints.at(i).second.toPoint(), QString::number(i));
//    }
}
