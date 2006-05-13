/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#include <QFontMetrics>
#include <QPainter>

#include <kdebug.h>
#include <kprinter.h>

#include "contextstyle.h"
#include "elementvisitor.h"
#include "spaceelement.h"


KFORMULA_NAMESPACE_BEGIN


SpaceElement::SpaceElement( SpaceWidth space, bool tab, BasicElement* parent )
    : BasicElement( parent ), spaceWidth( space ), m_tab( tab )
{
}


SpaceElement::SpaceElement( const SpaceElement& other )
    : BasicElement( other ), spaceWidth( other.spaceWidth )
{
}


bool SpaceElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void SpaceElement::calcSizes( const ContextStyle& style,
                              ContextStyle::TextStyle tstyle,
                              ContextStyle::IndexStyle /*istyle*/ )
{
    luPt mySize = style.getAdjustedSize( tstyle );

    QFont font = style.getDefaultFont();
    font.setPointSize( mySize );

    QFontMetrics fm( font );
    QChar ch = 'x';
    LuPixelRect bound = fm.boundingRect( ch );

    setWidth( style.getSpace( tstyle, spaceWidth ) );
    setHeight( bound.height() );
    setBaseline( -bound.top() );
    //setMidline( getBaseline() - fm.strikeOutPos() );

    if ( m_tab ) {
        getParent()->registerTab( this );
    }
}

void SpaceElement::draw( QPainter& painter, const LuPixelRect& /*r*/,
                         const ContextStyle& style,
                         ContextStyle::TextStyle /*tstyle*/,
                         ContextStyle::IndexStyle /*istyle*/,
                         const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos(parentOrigin.x()+getX(), parentOrigin.y()+getY());
    // there is such a thing as negative space!
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    if ( style.edit() ) {
        painter.setPen( style.getEmptyColor() );
        painter.drawLine( style.layoutUnitToPixelX( myPos.x() ),
                          style.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          style.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          style.layoutUnitToPixelY( myPos.y()+getHeight() ) );
        painter.drawLine( style.layoutUnitToPixelX( myPos.x() ),
                          style.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          style.layoutUnitToPixelX( myPos.x() ),
                          style.layoutUnitToPixelY( myPos.y()+getHeight()-getHeight()/5 ) );
        painter.drawLine( style.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          style.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          style.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          style.layoutUnitToPixelY( myPos.y()+getHeight()-getHeight()/5 ) );
    }
}


void SpaceElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);
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
    }
}

bool SpaceElement::readAttributesFromDom( QDomElement element )
{
    if ( !BasicElement::readAttributesFromDom( element ) ) {
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
    m_tab = !tabStr.isNull();
    return true;
}

bool SpaceElement::readContentFromDom(QDomNode& node)
{
    return BasicElement::readContentFromDom( node );
}

void SpaceElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{

    QDomElement de = doc.createElement( oasisFormat ? "math:mspace" : "mspace" );
    QString width;

    switch ( spaceWidth ) {
    case NEGTHIN:
        width = "-3/18em";
        break;
    case THIN:
        width = "thinmathspace";
        break;
    case MEDIUM:
        width = "mediummathspace";
        break;
    case THICK:
        width = "thickmathspace";
        break;
    case QUAD:
        width = "veryverythickmathspace"; // double 'very' is appropriate.
        break;
    }

    de.setAttribute( "width", width );

    parent.appendChild( de );


    /* // worked, but I redecided.
    switch ( spaceWidth )
    {
    case NEGTHIN:
        return doc.createEntityReference( "NegativeThinSpace" );
    case THIN:
        return doc.createEntityReference( "ThinSpace" );
    case MEDIUM:
        return doc.createEntityReference( "MediumSpace" );
    case THICK:
        return doc.createEntityReference( "ThickSpace" );
    case QUAD:
        //return doc.createEntityReference( "Space" ); // misused &Space;???
        QDomElement de = doc.createElement( "mspace" );
        de.setAttribute( "width", "veryverythickmathspace" );
        return de;
    }*/

}

QString SpaceElement::toLatex()
{
    switch ( spaceWidth ) {
    case NEGTHIN: return "\\!";
    case THIN:    return "\\,";
    case MEDIUM:  return "\\>";
    case THICK:   return "\\;";
    case QUAD:    return "\\quad ";
    }
    return "";
}

KFORMULA_NAMESPACE_END
