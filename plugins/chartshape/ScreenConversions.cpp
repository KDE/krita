/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Calligra
#include <KoUnit.h>
#include <KoDpi.h>

// KChart
#include "ScreenConversions.h"

// Qt
#include <QPainter>
#include <QSize>
#include <QSizeF>
#include <QRectF>
#include <QRect>

using namespace KChart;

qreal ScreenConversions::pxToPtX(qreal px)
{
    return KoUnit(KoUnit::Inch).fromUserValue(px / KoDpi::dpiX());
}

qreal ScreenConversions::pxToPtY(qreal px)
{
    return KoUnit(KoUnit::Inch).fromUserValue(px / KoDpi::dpiY());
}

qreal ScreenConversions::ptToPxX(qreal pt)
{
    return KoUnit::toInch(pt) * KoDpi::dpiX();
}

qreal ScreenConversions::ptToPxY(qreal pt)
{
    return KoUnit::toInch(pt) * KoDpi::dpiY();
}

void ScreenConversions::scaleFromPtToPx(QPainter &painter)
{
    const qreal inPerPt = KoUnit::toInch(1.0);
    painter.scale(1.0 / (inPerPt * KoDpi::dpiX()), 1.0 / (inPerPt * KoDpi::dpiY()));
}

QSize ScreenConversions::scaleFromPtToPx(const QSizeF &size)
{
    return QSizeF(ptToPxX(size.width()), ptToPxY(size.height())).toSize();
}

QSizeF ScreenConversions::scaleFromPxToPt(const QSize &size)
{
    return QSizeF(pxToPtX(size.width()), pxToPtY(size.height()));
}

QPoint ScreenConversions::scaleFromPtToPx(const QPointF &point)
{
    return QPointF(ptToPxX(point.x()), ptToPxY(point.y())).toPoint();
}

QRect ScreenConversions::scaleFromPtToPx(const QRectF &rect)
{
    return QRect(scaleFromPtToPx(rect.topLeft()),
                 scaleFromPtToPx(rect.size()));
}
