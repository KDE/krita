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

#include <algorithm>

#include <qpainter.h>

#include <klocale.h>

#include "Entities.h"
#include "SequenceElement.h"
#include "TextElement.h"
#include "GlyphElement.h"
#include "TokenElement.h"
#include "IdentifierElement.h"
#include "FormulaElement.h"
#include "ElementFactory.h"

KFORMULA_NAMESPACE_BEGIN

TokenElement::TokenElement( BasicElement* parent ) : TokenStyleElement( parent ),
                                                     m_textOnly( true )
{
}

int TokenElement::buildChildrenFromMathMLDom(QList<BasicElement*>& list, QDomNode n) 
{
    while ( ! n.isNull() ) {
        if ( n.isText() ) {
            QString textelements = n.toText().data();
            textelements = textelements.stripWhiteSpace();
                
            for (uint i = 0; i < textelements.length(); i++) {
                TextElement* child = new TextElement(textelements[i]);
                child->setParent(this);
                child->setCharFamily( charFamily() );
                child->setCharStyle( charStyle() );
                list.append(child);
            }
        }
        else if ( n.isEntityReference() ) {
            QString entity = n.toEntityReference().nodeName();
            const entityMap* begin = entities;
            const entityMap* end = entities + entityMap::size();
            const entityMap* pos = std::lower_bound( begin, end, entity.ascii() );
            if ( pos == end || QString( pos->name ) != entity ) {
                kWarning() << "Invalid entity refererence: " << entity << endl;
            }
            else {
                TextElement* child = new TextElement( QChar( pos->unicode ) );
                child->setParent(this);
                child->setCharFamily( charFamily() );
                child->setCharStyle( charStyle() );
                list.append(child);
            }
        }
        else if ( n.isElement() ) {
            m_textOnly = false;
            // Only mglyph element is allowed
            QDomElement e = n.toElement();
            if ( e.tagName().lower() != "mglyph" ) {
                kWarning( DEBUGID ) << "Invalid element inside Token Element\n";
                return -1;
            }
            GlyphElement* child = new GlyphElement();
            child->setParent(this);
            child->setCharFamily( charFamily() );
            child->setCharStyle( charStyle() );
            /*
            if ( child->buildFromMathMLDom( e ) == -1 ) {
                return -1;
            }
            */
            list.append( child );
        }
        else {
            kWarning() << "Invalid content in TokenElement\n";
        }
        n = n.nextSibling();
    }
//	parse();
	kWarning() << "Num of children " << list.count() << endl;
    return 1;
}

luPt TokenElement::getSpaceBefore( const ContextStyle& context, 
                                   ContextStyle::TextStyle tstyle,
                                   double factor )
{
    if ( !context.isScript( tstyle ) ) {
        return context.getMediumSpace( tstyle, factor );
    }
    return 0;
}

luPt TokenElement::getSpaceAfter( const ContextStyle& context, 
                                  ContextStyle::TextStyle tstyle,
                                  double factor )
{
    if ( !context.isScript( tstyle ) ) {
        return context.getThinSpace( tstyle, factor );
    }
    return 0;
}

KFORMULA_NAMESPACE_END
