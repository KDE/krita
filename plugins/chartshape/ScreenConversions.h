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

#ifndef KCHART_SCREEN_CONVERSIONS_H
#define KCHART_SCREEN_CONVERSIONS_H

class QPainter;
class QPoint;
class QPointF;
class QSize;
class QSizeF;
class QRect;
class QRectF;

namespace KChart {

class ScreenConversions
{
public:
    // pt --> px
    static qreal ptToPxX(qreal pt);
    static qreal ptToPxY(qreal pt);
    static void   scaleFromPtToPx(QPainter &painter);
    static QPoint scaleFromPtToPx(const QPointF &point);
    static QSize  scaleFromPtToPx(const QSizeF &size);
    static QRect  scaleFromPtToPx(const QRectF &rect);

    // px --> pt
    static qreal pxToPtX(qreal px);
    static qreal pxToPtY(qreal px);
    static QSizeF scaleFromPxToPt(const QSize &size);
};

} // namespace KChart

#endif // KCHART_SCREEN_CONVERSIONS_H
