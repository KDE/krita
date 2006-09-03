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
 * Boston, MA 02110-1301, USA.
*/

#include "textelement.h"
#include "stringelement.h"

KFORMULA_NAMESPACE_BEGIN

StringElement::StringElement( BasicElement* parent ) : TokenElement( parent ) {
}

bool StringElement::readAttributesFromMathMLDom(const QDomElement& element)
{
    if ( ! BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    if ( ! inherited::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    m_lquote = element.attribute( "lquote" );
    if ( ! m_lquote.isNull() ) {
        m_customLquote = true;
    }
    m_rquote = element.attribute( "rquote" );
    if ( ! m_rquote.isNull() ) {
        m_customRquote = true;
    }

    return true;
}

int StringElement::buildChildrenFromMathMLDom(QPtrList<BasicElement>& list, QDomNode n) 
{
    int count = inherited::buildChildrenFromMathMLDom( list, n );
    if ( count == -1 )
        return -1;
    TextElement* child = new TextElement( '"' );
    child->setParent( this );
    child->setCharFamily( charFamily() );
    child->setCharStyle( charStyle() );
    insert( 0, child );
    child = new TextElement( '"' );
    child->setParent( this );
    child->setCharFamily( charFamily() );
    child->setCharStyle( charStyle() );
    insert( countChildren(), child );
    return count;
}

void StringElement::writeMathMLAttributes( QDomElement& element ) const
{
    inherited::writeMathMLAttributes( element );
    if ( m_customLquote ) {
        element.setAttribute( "lquote", m_lquote );
    }
    if ( m_customRquote ) {
        element.setAttribute( "rquote", m_rquote );
    }
}

void StringElement::writeMathMLContent( QDomDocument& doc, QDomElement& element, bool oasisFormat ) const
{
    for ( uint i = 1; i < countChildren() - 1; ++i ) {
        const BasicElement* e = getChild( i );
        e->writeMathML( doc, element, oasisFormat );
    }
}

KFORMULA_NAMESPACE_END
