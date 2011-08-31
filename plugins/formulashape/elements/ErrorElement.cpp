/* This file is part of the KDE project
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "ErrorElement.h"
#include <QPainter>

ErrorElement::ErrorElement( BasicElement* parent ) : RowElement( parent )
{
}

void ErrorElement::paint( QPainter& painter, AttributeManager* am )
{
    Q_UNUSED( am )
    // just paint a red rectangle
    painter.save();
    QRectF tmp( 0.0, 0.0, width(), height() );
    painter.fillRect( tmp, QColor( "red" ) );
    painter.restore();
}

ElementType ErrorElement::elementType() const
{
    return Error;
}

