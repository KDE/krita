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
    ClippingRect():
        top(0), right(1), bottom(1), left(0),
        uniform(true), inverted(false) { }
    
    ClippingRect(const ClippingRect& rect):
        top(rect.top), right(rect.right), bottom(rect.bottom), left(rect.left),
        uniform(rect.uniform), inverted(rect.inverted) { }

    explicit ClippingRect(const QRectF& rect, bool isUniform=false)
    {
        setRect(rect, isUniform);
    }
    
    void scale(const QSizeF& size, bool isUniform=false)
    {
        top    *= size.height();
        right  *= size.width();
        bottom *= size.height();
        left   *= size.width();
        uniform = isUniform;
    }
    
    void normalize(const QSizeF& size)
    {
        if (!uniform) {
            scale(QSizeF(1.0/size.width(), 1.0/size.height()), true);
        }
        
        if(inverted) {
            right    = 1.0 - right;
            bottom   = 1.0 - bottom;
            inverted = false;
        }
    }
    
    void setRect(const QRectF& rect, bool isUniform=false)
    {
        top      = rect.top();
        right    = rect.right();
        bottom   = rect.bottom();
        left     = rect.left();
        uniform  = isUniform;
        inverted = false;
    }
    
    qreal  width()  const { return right - left; }
    qreal  height() const { return bottom - top; }
    QRectF toRect() const { return QRectF(left, top, width(), height()); }
    
    qreal top;
    qreal right;
    qreal bottom;
    qreal left;
    bool uniform;
    bool inverted;
};

#endif // H_CLIPPING_RECT_H
