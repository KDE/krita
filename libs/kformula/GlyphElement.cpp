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

#include <QFontMetrics>
#include <QPainter>

#include "fontstyle.h"
#include "AttributeManager.h"
#include "TokenElement.h"
#include "GlyphElement.h"

namespace FormulaShape {

GlyphElement::GlyphElement( BasicElement* parent ) : TextElement( ' ', false, parent )
{
}



/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
#warning "Port to flake"  
#if 0
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
#endif

bool GlyphElement::hasFont( const AttributeManager *am )
{
    QVariant fontFamily = am->valueOf( "fontfamily" );
    if ( fontFamily.canConvert( QVariant::String ) ) {
        QStringList missing;
        FontStyle::testFont( missing, fontFamily.toString() );
        return missing.isEmpty();
    }
    return false;
}

/**
 * Render the element to the given QPainter
 * @param painter The QPainter to paint the element to
 */
void GlyphElement::paint( QPainter& painter, const AttributeManager* am )
{
    QFont font;
    QString text;

    if ( hasFont( am ) ) {
        QVariant index = am->valueOf( "index" );
		painter.setFont( am->valueOf( "fontfamily" ).toString() );
        if ( index.canConvert( QVariant::UInt ) )
            painter.drawText( 0, 0, QChar( index.toUInt() ) );
    }
}

/**
 * Calculate the size of the element and the positions of its children
 * @param am The AttributeManager providing information about attributes values
 */
void GlyphElement::layout( const AttributeManager* am )
{
    QRect bound;
    if ( hasFont( am ) ) {
        QFont font( am->valueOf( "fontfamily" ).toString() );
        QFontMetrics fm ( font );
        QVariant index = am->valueOf( "index" );
        if ( index.canConvert( QVariant::UInt ) ) {
            QChar ch = QChar( index.toUInt() );
            bound = fm.boundingRect( ch );
            setWidth( fm.width( ch ) );
        }
    }
    else {
        QFont font = static_cast<TokenElement*>(parentElement())->font( am );
        QFontMetrics fm ( font );
        QVariant alt = am->valueOf( "alt" );
        if ( alt.canConvert( QVariant::String ) ) {
            QString alternate = alt.toString();
            bound = fm.boundingRect( alternate );
            setWidth( fm.width( alternate ) );
        }
    }
    setHeight( bound.height() );
    setBaseLine( -bound.top() );
    
    // There are some glyphs in TeX that have
    // baseline==0. (\int, \sum, \prod)
    if ( baseLine() == 0 ) {
        setBaseLine( -1 );
    }
}

} // namespace FormulaShape
