/* This file is part of the KDE project
   Copyright 2011 Silvio Heinrich <plassy@web.de>

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

#ifndef H_CLIPPING_RECT_H
#define H_CLIPPING_RECT_H

#include <QRectF>
#include <QSizeF>

/**
 * This is a helper class. It helps converting clipping information
 * from standard cartesian coordinates to the format required to
 * save the information to an ODF file (using the fo::clip attribute)
 */
struct ClippingRect
{
    ClippingRect();
    ClippingRect(const ClippingRect& rect);
    explicit ClippingRect(const QRectF& rect, bool isUniform=false);

    void scale(const QSizeF& size, bool isUniform=false);
    void normalize(const QSizeF& size);
    void setRect(const QRectF& rect, bool isUniform=false);

    qreal width() const;
    qreal height() const;
    QRectF toRect() const;

    qreal top;
    qreal right;
    qreal bottom;
    qreal left;
    bool uniform;
    bool inverted;
};

#endif // H_CLIPPING_RECT_H
