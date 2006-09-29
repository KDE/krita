/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

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
#include <KoXmlWriter.h>

namespace KFormula {

SpaceElement::SpaceElement( BasicElement* parent ) : BasicElement( parent )
{
}

void SpaceElement::paint( QPainter& painter ) const
{
    // paint perhaps some eye candy :-)
}

void SpaceElement::calculateSize()
{
// TODO fetch attributes width and height and set the values accordingly
//    setWidth(  );
//    setHeight();
}

void SpaceElement::readMathML( const QDomElement& element )
{
}

void SpaceElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    writer->startElement( oasisFormat ? "math:mspace" : "mspace" );
    writeMathMLAttributes( writer );
    writer->endElement();
}






void SpaceElement::writeDom(QDomElement element)
{
/*    BasicElement::writeDom(element);
    switch ( spaceWidth ) {
    case NEGTHIN:
        element.setAttribute( "WIDTH", "negthin" );
        break;
    case THIN:
        element.setAttribute( "WIDTH", "thin" );
        break;
    case MEDIUM:
        element.setAttribute( "WIDTH", "medium" );
        break;
    case THICK:
        element.setAttribute( "WIDTH", "thick" );
        break;
    case QUAD:
        element.setAttribute( "WIDTH", "quad" );
        break;
    }
    if ( m_tab ) {
        element.setAttribute( "TAB", "true" );
    }*/
}

bool SpaceElement::readAttributesFromDom( QDomElement element )
{
/*    if ( !BasicElement::readAttributesFromDom( element ) ) {
        return false;
    }
    QString widthStr = element.attribute( "WIDTH" );
    if( !widthStr.isNull() ) {
        if ( widthStr.toLower() == "quad" ) {
            spaceWidth = QUAD;
        }
        else if ( widthStr.toLower() == "thick" ) {
            spaceWidth = THICK;
        }
        else if ( widthStr.toLower() == "medium" ) {
            spaceWidth = MEDIUM;
        }
        else if ( widthStr.toLower() == "negthin" ) {
            spaceWidth = NEGTHIN;
        }
        else {
            spaceWidth = THIN;
        }
    }
    else {
        return false;
    }
    QString tabStr = element.attribute( "TAB" );
    m_tab = !tabStr.isNull();*/
    return true;
}

bool SpaceElement::readContentFromDom(QDomNode& node)
{
    return BasicElement::readContentFromDom( node );
}

} // namespace KFormula
