/* This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "creationstrategy.h"
#include "ActionElement.h"

namespace KFormula {

ActionElement::ActionElement( BasicElement* parent ) : SequenceElement( parent ),
                                                       m_selection( 0 )
{
}

bool ActionElement::readAttributesFromMathMLDom(const QDomElement& element)
{
    if ( ! BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    m_actionType = element.attribute( "actiontype" );
    QString selectionStr = element.attribute( "selection" );
    if ( ! selectionStr.isNull() ) {
        bool ok;
        m_selection = selectionStr.toUInt( &ok ); 
        if ( ! ok ) m_selection = 0;
    }

    return true;
}

int ActionElement::buildChildrenFromMathMLDom(QPtrList<BasicElement>& list, QDomNode n) 
{
    if ( ! n.isElement() )
        return -1;
    QDomElement e = n.toElement();
    QString tag = e.tagName().lower();
    BasicElement* child = getCreationStrategy()->createElement( tag, e );
    if ( child == 0 )
        return -1;
    child->setParent( this );
    if ( child->buildFromMathMLDom( e ) == -1 ) {
        delete child;
        return -1;
    }
    list.append( child );
    parse();
    return 1;
}

void ActionElement::writeMathMLAttributes( QDomElement& element ) const
{
    if ( ! m_actionType.isNull() ) {
        element.setAttribute( "actiontype", m_actionType );
    }
    if ( m_selection ) {
        element.setAttribute( "selection", QString( "%1" ).arg( m_selection ) );
    }
}

} // namespace KFormula

