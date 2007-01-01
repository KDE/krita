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

void SpaceElement::writeMathMLAttributes( KoXmlWriter* writer ) const
{
    /*
    switch ( m_widthType ) {
    case AbsoluteSize:
        element.setAttribute( "width", QString( "%1pt" ).arg( m_width ) );
        break;
    case RelativeSize:
        element.setAttribute( "width", QString( "%1%" ).arg( m_width * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( "width", QString( "%1px" ).arg( m_width ) );
        break;
    case NegativeVeryVeryThinMathSpace:
        element.setAttribute( "width", "negativeveryverythinmathspace" );
        break;
    case NegativeVeryThinMathSpace:
        element.setAttribute( "width", "negativeverythinmathspace" );
        break;
    case NegativeThinMathSpace:
        element.setAttribute( "width", "negativethinmathspace" );
        break;
    case NegativeMediumMathSpace:
        element.setAttribute( "width", "negativemediummathspace" );
        break;
    case NegativeThickMathSpace:
        element.setAttribute( "width", "negativethickmathspace" );
        break;
    case NegativeVeryThickMathSpace:
        element.setAttribute( "width", "negativeverythickmathspace" );
        break;
    case NegativeVeryVeryThickMathSpace:
        element.setAttribute( "width", "negativeveryverythickmathspace" );
        break;
    case VeryVeryThinMathSpace:
        element.setAttribute( "width", "veryverythinmathspace" );
        break;
    case VeryThinMathSpace:
        element.setAttribute( "width", "verythinmathspace" );
        break;
    case ThinMathSpace:
        element.setAttribute( "width", "thinmathspace" );
        break;
    case MediumMathSpace:
        element.setAttribute( "width", "mediummathspace" );
        break;
    case ThickMathSpace:
        element.setAttribute( "width", "thickmathspace" );
        break;
    case VeryThickMathSpace:
        element.setAttribute( "width", "verythickmathspace" );
        break;
    case VeryVeryThickMathSpace:
        element.setAttribute( "width", "veryverythickmathspace" );
        break;
    default:
        break;
    }
    switch ( m_heightType ) {
    case AbsoluteSize:
        element.setAttribute( "height", QString( "%1pt" ).arg( m_height ) );
        break;
    case RelativeSize:
        element.setAttribute( "height", QString( "%1%" ).arg( m_height * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( "height", QString( "%1px" ).arg( m_height ) );
        break;
    default:
        break;
    }
    switch ( m_depthType ) {
    case AbsoluteSize:
        element.setAttribute( "depth", QString( "%1pt" ).arg( m_depth ) );
        break;
    case RelativeSize:
        element.setAttribute( "depth", QString( "%1%" ).arg( m_depth * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( "depth", QString( "%1px" ).arg( m_depth ) );
        break;
    default:
        break;
    }
    switch ( m_lineBreak ) {
    case AutoBreak:
        element.setAttribute( "linebreak", "auto" );
        break;
    case NewLineBreak:
        element.setAttribute( "linebreak", "newline" );
        break;
    case IndentingNewLineBreak:
        element.setAttribute( "linebreak", "indentingnewline" );
        break;
    case NoBreak:
        element.setAttribute( "linebreak", "nobreak" );
        break;
    case GoodBreak:
        element.setAttribute( "linebreak", "goodbreak" );
        break;
    case BadBreak:
        element.setAttribute( "linebreak", "badbreak" );
        break;
    default:
        break;
    }
    */
}





void SpaceElement::writeDom(QDomElement element)
{
    /*
    BasicElement::writeDom(element);
    switch ( m_widthType ) {
    case NegativeVeryVeryThinMathSpace:
    case NegativeVeryThinMathSpace:
    case NegativeThinMathSpace:
    case NegativeMediumMathSpace:
    case NegativeThickMathSpace:
    case NegativeVeryThickMathSpace:
    case NegativeVeryVeryThickMathSpace:
        element.setAttribute( "WIDTH", "negthin" );
        break;
    case VeryVeryThinMathSpace:
    case VeryThinMathSpace:
    case ThinMathSpace:
        element.setAttribute( "WIDTH", "thin" );
        break;
    case MediumMathSpace:
        element.setAttribute( "WIDTH", "medium" );
        break;
    case ThickMathSpace:
        element.setAttribute( "WIDTH", "thick" );
        break;
    case VeryThickMathSpace:
    case VeryVeryThickMathSpace:
        element.setAttribute( "WIDTH", "quad" );
        break;
    case AbsoluteSize:
    case RelativeSize:
    case PixelSize:
        if ( m_width < 0 ) {
            element.setAttribute( "WIDTH", "negthin" );
        }
        else {
            element.setAttribute( "WIDTH", "thin" );
        }
    default:
        break;
    }
    if ( m_tab ) {
        element.setAttribute( "TAB", "true" );
    }
    */
}

bool SpaceElement::readAttributesFromDom( QDomElement element )
{
/*
    if ( !BasicElement::readAttributesFromDom( element ) ) {
        return false;
    }
    QString widthStr = element.attribute( "WIDTH" );
    if( !widthStr.isNull() ) {
        if ( widthStr.lower() == "quad" ) {
            m_widthType = VeryVeryThickMathSpace;
        }
        else if ( widthStr.lower() == "thick" ) {
            m_widthType = ThickMathSpace;
        }
        else if ( widthStr.lower() == "medium" ) {
            m_widthType = MediumMathSpace;
        }
        else if ( widthStr.lower() == "negthin" ) {
            m_widthType = NegativeThinMathSpace;
        }
        else {
            m_widthType = ThinMathSpace;
        }
    }
    else {
        return false;
    }
    QString tabStr = element.attribute( "TAB" );
    m_tab = !tabStr.isNull();
*/
    return true;
}

bool SpaceElement::readContentFromDom(QDomNode& node)
{
    return BasicElement::readContentFromDom( node );
}


void SpaceElement::calcSizes( const ContextStyle& context,
                              ContextStyle::TextStyle tstyle,
                              ContextStyle::IndexStyle /*istyle*/,
                              StyleAttributes& style )
{
    /*
    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );
    QFont font = context.getDefaultFont();
    font.setPointSize( mySize );

    QFontMetrics fm( font );
    QChar w = 'M';
    LuPixelRect hbound = fm.boundingRect( w );
    QChar h = 'x';
    LuPixelRect vbound = fm.boundingRect( h );

    double width = style.getSpace( m_widthType, m_width );
    if ( m_widthType == AbsoluteSize ) {
        width = m_width / context.layoutUnitPtToPt( context.getBaseSize() );
    }
    else if ( m_widthType == PixelSize ) {
        width = context.pixelYToPt( m_width ) / context.layoutUnitPtToPt( context.getBaseSize() );
    }
    double height = style.getSpace( m_heightType, m_height );
    if ( m_heightType == AbsoluteSize ) {
        height = m_height / context.layoutUnitPtToPt( context.getBaseSize() );
    }
    else if ( m_heightType == PixelSize ) {
        height = context.pixelYToPt( m_height ) / context.layoutUnitPtToPt( context.getBaseSize() );
    }
    double depth = style.getSpace( m_depthType, m_depth );
    if ( m_depthType == AbsoluteSize ) {
        depth = m_depth / context.layoutUnitPtToPt( context.getBaseSize() );
    }
    else if ( m_depthType == PixelSize ) {
        depth = context.pixelYToPt( m_depth ) / context.layoutUnitPtToPt( context.getBaseSize() );
    }

    setWidth( hbound.width() * width );
    setHeight( vbound.height() * height + vbound.height() * depth );
    setBaseline( vbound.height() * height );

    if ( m_tab ) {
        getParent()->registerTab( this );
    }
    */
}

void SpaceElement::draw( QPainter& painter, const LuPixelRect& /*r*/,
                         const ContextStyle& context,
                         ContextStyle::TextStyle /*tstyle*/,
                         ContextStyle::IndexStyle /*istyle*/,
                         StyleAttributes& /*style*/,
                         const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos(parentOrigin.x()+getX(), parentOrigin.y()+getY());
    // there is such a thing as negative space!
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    /*
    if ( context.edit() ) {
        painter.setPen( context.getEmptyColor() );
        painter.drawLine( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          context.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ) );
        painter.drawLine( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight()-getHeight()/5 ) );
        painter.drawLine( context.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          context.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight()-getHeight()/5 ) );
    }
    */
}

bool SpaceElement::readAttributesFromMathMLDom(const QDomElement& element)
{
/*
    if ( ! BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    QString widthStr = element.attribute( "width" ).stripWhiteSpace().lower();
    if ( ! widthStr.isNull() ) {
        m_width = getSize( widthStr, &m_widthType );
        if ( m_widthType == NoSize ) {
            m_widthType = getSpace( widthStr );
        }
    }
    QString heightStr = element.attribute( "height" ).stripWhiteSpace().lower();
    if ( ! heightStr.isNull() ) {
        m_height = getSize( heightStr, &m_heightType );
    }
    QString depthStr = element.attribute( "depth" ).stripWhiteSpace().lower();
    if ( ! depthStr.isNull() ) {
        m_depth = getSize( depthStr, &m_depthType );
    }
    QString linebreakStr = element.attribute( "linebreak" ).stripWhiteSpace().lower();
    if ( ! linebreakStr.isNull() ) {
        if ( linebreakStr == "auto" ) {
            m_lineBreak = AutoBreak;
        }
        else if ( linebreakStr == "newline" ) {
            m_lineBreak = NewLineBreak;
        }
        else if ( linebreakStr == "indentingnewline" ) {
            m_lineBreak = IndentingNewLineBreak;
        }
        else if ( linebreakStr == "nobreak" ) {
            m_lineBreak = NoBreak;
        }
        else if ( linebreakStr == "goodbreak" ) {
            m_lineBreak = GoodBreak;
        }
        else if ( linebreakStr == "badbreak" ) {
            m_lineBreak = BadBreak;
        }
    }
*/
    return true;
}

} // namespace KFormula
