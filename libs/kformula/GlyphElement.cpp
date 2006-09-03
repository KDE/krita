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

#include <QFontMetrics>
#include <QPainter>

#include "fontstyle.h"
#include "GlyphElement.h"

namespace KFormula {

GlyphElement::GlyphElement( BasicElement* parent ) : TextElement( ' ', false, parent ) {
}

bool GlyphElement::readAttributesFromMathMLDom( const QDomElement& element )
{
    if ( !BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    // MathML Section 3.2.9.2
    m_fontFamily = element.attribute( "fontfamily" );
    if ( m_fontFamily.isNull() ) {
        kdWarning( DEBUGID ) << "Required attribute fontfamily not found in glyph element\n";
        return false;
    }
    QString indexStr = element.attribute( "index" );
    if ( indexStr.isNull() ) {
        kdWarning( DEBUGID ) << "Required attribute index not found in glyph element\n";
        return false;
    }
    bool ok;
    ushort index = indexStr.toUShort( &ok );
    if ( ! ok ) {
        kdWarning( DEBUGID ) << "Invalid index value in glyph element\n";
        return false;
    }
    m_char = QChar( index );

    m_alt = element.attribute( "alt" );
    if ( m_alt.isNull() ) {
        kdWarning( DEBUGID ) << "Required attribute alt not found in glyph element\n";
        return false;
    }

    QStringList missing;
    FontStyle::testFont( missing, m_fontFamily.lower() );
    m_hasFont = missing.isEmpty();

    return true;
}


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void GlyphElement::calcSizes( const ContextStyle& context, 
                              ContextStyle::TextStyle tstyle, 
                              ContextStyle::IndexStyle /*istyle*/,
                              StyleAttributes& style )
{
    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );
    QRect bound;

    if ( m_hasFont ) {
        QFont font( m_fontFamily );
        font.setPointSizeFloat( context.layoutUnitPtToPt( mySize ) );
        QFontMetrics fm ( font );
        bound = fm.boundingRect( m_char );
        setWidth( context.ptToLayoutUnitPt( fm.width( m_char ) ) );
    }
    else {
        QFont font( context.getDefaultFont() );
        font.setPointSizeFloat( context.layoutUnitPtToPt( mySize ) );
        QFontMetrics fm ( font );
        bound = fm.boundingRect( m_alt );
        setWidth( context.ptToLayoutUnitPt( fm.width( m_alt ) ) );
    }
    setHeight( context.ptToLayoutUnitPt( bound.height() ) );
    setBaseline( context.ptToLayoutUnitPt( -bound.top() ) );
    
    // There are some glyphs in TeX that have
    // baseline==0. (\int, \sum, \prod)
    if ( getBaseline() == 0 ) {
        setBaseline( -1 );
    }
}

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void GlyphElement::draw( QPainter& painter, const LuPixelRect& /*r*/,
                        const ContextStyle& context,
                        ContextStyle::TextStyle tstyle,
                        ContextStyle::IndexStyle /*istyle*/,
                        StyleAttributes& style,
                        const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );
    QFont font;
    QString text;
    
    if ( m_hasFont ) {
        painter.setPen( style.color() );
        setCharStyle( style.charStyle() );
        setCharFamily( style.charFamily() );
        font = QFont( m_fontFamily );
        text = m_char;
        painter.fillRect( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y() ),
                          context.layoutUnitToPixelX( getWidth() ),
                          context.layoutUnitToPixelY( getHeight() ),
                          style.background() );
    }
    else {
        painter.setPen( context.getErrorColor() );
        font = context.getDefaultFont();
        text = m_alt;
    }
    font.setPointSizeFloat( context.layoutUnitToFontSize( mySize, false ) );
    painter.setFont( font );
    painter.drawText( context.layoutUnitToPixelX( myPos.x() ),
                      context.layoutUnitToPixelY( myPos.y()+getBaseline() ),
                      text );
}
    
void GlyphElement::writeMathMLAttributes( QDomElement& element ) const
{
    element.setAttribute( "fontfamily", m_fontFamily );
    element.setAttribute( "index", m_char.unicode() );
    element.setAttribute( "alt", m_alt );
}

} // namespace KFormula
