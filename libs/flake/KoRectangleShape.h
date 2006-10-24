/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KORECTANGLESHAPE_H
#define KORECTANGLESHAPE_H

#include "KoParameterShape.h"

#define KoRectangleShapeId "KoRectangleShape"

class KoRectangleShape : public KoParameterShape
{
public:
    KoRectangleShape();
    ~KoRectangleShape();

protected:
    void moveHandleAction( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers = Qt::NoModifier );
    void updatePath( const QSizeF &size );
    void createPath( const QSizeF &size );

private:
    double m_cornerRadiusX; //in precent a number between 0 and 100
    double m_cornerRadiusY; //in precent a number between 0 and 100

    KoSubpath m_points;
};

#endif /* KORECTANGLESHAPE_H */


