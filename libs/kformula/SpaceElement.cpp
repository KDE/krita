/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#include "SpaceElement.h"
#include "AttributeManager.h"
#include <KoXmlWriter.h>
#include <QPainter>
#include <QBrush>

namespace FormulaShape {

SpaceElement::SpaceElement( BasicElement* parent ) : BasicElement( parent )
{}

void SpaceElement::paint( QPainter& painter, const AttributeManager* )
{
    painter.setBrush( QBrush( Qt::lightGray, Qt::DiagCrossPattern ) );
    painter.drawRect( boundingRect() );
}

void SpaceElement::layout( const AttributeManager* am )
{
    double height =  am->valueOf( "height" ).toDouble(); 
    setHeight( height + am->valueOf( "depth" ).toDouble() );
    setWidth( am->valueOf( "width" ).toDouble() );
    setBaseLine( height );
}

QVariant SpaceElement::attributesDefaultValue( const QString& attribute ) const
{
    if( attribute == "width" || attribute == "height" || attribute == "depth" )
        return 0.0;
    else
        return Auto;
}

void SpaceElement::readMathML( const KoXmlElement& element )
{
    Q_UNUSED( element )
}

void SpaceElement::writeMathML( KoXmlWriter* writer, bool oasisFormat ) const
{
    Q_UNUSED( writer )
    Q_UNUSED( oasisFormat )
}

ElementType SpaceElement::elementType() const
{
    return Space;
}

} // namespace FormulaShape
