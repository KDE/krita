/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#include "FencedElement.h"
#include "OperatorElement.h"
#include "AttributeManager.h"
#include <QPainter>

FencedElement::FencedElement( BasicElement* parent ) : RowElement( parent )
{}

void FencedElement::paint( QPainter& painter, AttributeManager* am )
{
    Q_UNUSED( am )

    QPen pen( painter.pen() );
    pen.setWidth( 1 );
    painter.setPen( pen );
    painter.drawPath( m_fence );
}

void FencedElement::layout( const AttributeManager* am )
{
    m_fence = QPainterPath();  // empty path buffer
    OperatorElement op;
    m_fence.addPath( op.renderForFence( am->stringOf( "open", this ), Prefix ) );

    const QString separators = am->stringOf( "separators", this );
    int count = 0;
    foreach( const BasicElement* tmp, childElements() ) {
        m_fence.moveTo( m_fence.currentPosition() + QPointF( tmp->width() , 0.0 ) );
        if( tmp != childElements().last() )
            m_fence.addPath( op.renderForFence( separators.at( count ), Infix ) );
        count++;
    }

    m_fence.addPath( op.renderForFence( am->stringOf( "close", this ), Postfix ) );

    setWidth( m_fence.boundingRect().width() );
    setHeight( m_fence.boundingRect().height() );
}

QString FencedElement::attributesDefaultValue( const QString& attribute ) const
{
    if( attribute == "open" )
        return "(";
    else if( attribute == "close" )
        return ")";
    else if( attribute == "separators" )
        return ",";
    else
        return QString();
}
    
ElementType FencedElement::elementType() const
{
    return Fenced;
}

