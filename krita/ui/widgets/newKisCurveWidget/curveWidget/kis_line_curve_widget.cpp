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

#include "kis_linear_curve_widget.h"

#include <QPainter>
#include <QPainterPath>

KisLineCurveWidget::KisLineCurveWidget(QWidget *parent) :
    KisCurveWidgetBase(parent)
{
}

void KisLineCurveWidget::paintEvent(QPaintEvent *e) {

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    paintBackground(&painter);

    painter.setMatrix(m_converterMatrix);

    QPainterPath path;
    path.moveTo(m_points.first());

    for(int i=1; i<m_points.size(); i++) {
        path.lineTo(m_points.at(i));
    }

    painter.drawPath(path);

    paintBlips(&painter);
}
