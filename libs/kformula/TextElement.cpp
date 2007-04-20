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

#include "TextElement.h"

#include <math.h>

#include <QFontMetrics>
#include <QPainter>

#include <kdebug.h>
#include <KoXmlWriter.h>

#include "BasicElement.h"
#include "contextstyle.h"
#include "fontstyle.h"
#include "FormulaElement.h"
//#include "kformulacommand.h"
#include "symboltable.h"
#include "AttributeManager.h"


namespace FormulaShape {

TextElement::TextElement( QChar ch, bool beSymbol, BasicElement* parent ) 
    : BasicElement( parent ), m_character( ch ), symbol( beSymbol )
{
}

const QList<BasicElement*> TextElement::childElements()
{
    return QList<BasicElement*>();
}

void TextElement::writeMathMLContent( KoXmlWriter* writer, bool /* oasisFormat */ ) const
{
    writer->addTextNode( character() );
}


bool TextElement::isInvisible() const
{
    /*if (getElementType() != 0) {
        return getElementType()->isInvisible(*this);
    }*/
    // no longer needed as BasicElement implements this
    return false;
}


void TextElement::setStyle( QPainter& painter, const AttributeManager* am )
{
     // Color handling.
     // TODO: most of this should be handled by AttributeManager
     QVariant color = am->valueOf( "mathcolor" );
     if ( color.isNull() )
         color = am->valueOf( "color" );
     if ( color.canConvert( QVariant::Color ) )
         painter.setPen( color.value<QColor>() );
     else
         painter.setPen( QColor( "black" ) );
     
     // TODO: Default fonts should be read from settings
     // Font style handling
     QFont font = getFont( am );
     painter.setFont( font );
}

/**
 * Render the element to the given QPainter
 * @param painter The QPainter to paint the element to
 */
void TextElement::paint( QPainter& painter, const AttributeManager* am )
{
    // Invisible characters
    if ( character() == applyFunctionChar || character() == invisibleTimes || character() == invisibleComma )
        return;

	setStyle( painter, am );
     // FIXME: get rid of context
     /*
    if ( character() != QChar::Null ) {
        luPixel bl = getBaseline();
        if ( bl == -1 ) {
            // That's quite hacky and actually not the way it's
            // meant to be. You shouldn't calculate a lot in
            // draw. But I don't see how else to deal with
            // baseline==0 glyphs without yet another flag.

            bl = -( getHeight()/2 + context.axisHeight( tstyle, factor ) );
        }
        painter.drawText( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+bl ),
                          character() );
    }
    else {
        painter.setPen( QPen( context.getErrorColor(),
                              context.layoutUnitToPixelX( context.getLineWidth( factor ) ) ) );
        painter.drawRect( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y() ),
                          context.layoutUnitToPixelX( getWidth() ),
                          context.layoutUnitToPixelY( getHeight() ) );
        }
     */
     painter.drawText( 0, 0, character() ); // FIXME
}

void TextElement::layout( const AttributeManager* am )
{
    QFont font = getFont( am );
	// TODO: Font size
	double factor = 1.0;

    QFontMetrics fm( font );
    if ( character() == applyFunctionChar || character() == invisibleTimes || character() == invisibleComma ) {
        setWidth( 0 );
        setHeight( 0 );
        setBaseLine( height() );
    }
    else {
        QChar ch = character();
        if ( ch == QChar::null ) {
		  /*
            setWidth( qRound( context.getEmptyRectWidth( factor ) * 2./3. ) );
            setHeight( qRound( context.getEmptyRectHeight( factor ) * 2./3. ) );
		  */
            setBaseLine( height() );
        }
        else {
            QRect bound = fm.boundingRect( ch );
            setWidth( fm.width( ch ) );
            setHeight( bound.height() );
            setBaseLine( -bound.top() );

            // There are some glyphs in TeX that have
            // baseline==0. (\int, \sum, \prod)
            if ( baseLine() == 0 ) {
                //setBaseline( getHeight()/2 + context.axisHeight( tstyle ) );
                setBaseLine( -1 );
            }
        }
    }
}

#warning "Port and remove obsolete code"
#if 0  
/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void TextElement::calcSizes( const ContextStyle& context, 
                             ContextStyle::TextStyle tstyle, 
                             ContextStyle::IndexStyle /*istyle*/,
                             StyleAttributes& style )
{
    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );

    setCharStyle( style.charStyle() );
    setCharFamily( style.charFamily() );

    QFont font = getFont( context, style );
    double fontsize = context.layoutUnitPtToPt( mySize );
    double scriptsize = pow( style.scriptSizeMultiplier(), style.scriptLevel() );
    double size = fontsize * scriptsize;
    font.setPointSizeF( size < style.scriptMinSize() ? style.scriptMinSize() : size );

    QFontMetrics fm( font );
    if ( character() == applyFunctionChar || character() == invisibleTimes || character() == invisibleComma ) {
        setWidth( 0 );
        setHeight( 0 );
        setBaseline( getHeight() );
    }
    else {
        QChar ch = character();
        if ( ch == QChar::null ) {
            setWidth( qRound( context.getEmptyRectWidth( factor ) * 2./3. ) );
            setHeight( qRound( context.getEmptyRectHeight( factor ) * 2./3. ) );
            setBaseline( getHeight() );
        }
        else {
            QRect bound = fm.boundingRect( ch );
            setWidth( context.ptToLayoutUnitPt( fm.width( ch ) ) );
            setHeight( context.ptToLayoutUnitPt( bound.height() ) );
            setBaseline( context.ptToLayoutUnitPt( -bound.top() ) );

            // There are some glyphs in TeX that have
            // baseline==0. (\int, \sum, \prod)
            if ( getBaseline() == 0 ) {
                //setBaseline( getHeight()/2 + context.axisHeight( tstyle ) );
                setBaseline( -1 );
            }
        }
    }
}

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void TextElement::draw( QPainter& painter, const LuPixelRect& /*r*/,
                        const ContextStyle& context,
                        ContextStyle::TextStyle tstyle,
                        ContextStyle::IndexStyle /*istyle*/,
                        StyleAttributes& style,
                        const LuPixelPoint& parentOrigin )
{
    if ( character() == applyFunctionChar || character() == invisibleTimes || character() == invisibleComma ) {
        return;
    }

    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    // Let container set the color, instead of elementType
    //setUpPainter( context, painter );
    painter.setPen( style.color() );

    setCharStyle( style.charStyle() );
    setCharFamily( style.charFamily() );

    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );
    QFont font = getFont( context, style );
    double fontsize = context.layoutUnitPtToPt( mySize );
    double scriptsize = pow( style.scriptSizeMultiplier(), style.scriptLevel() );
    double size = fontsize * scriptsize;
    font.setPointSizeFloat( size < style.scriptMinSize() ? style.scriptMinSize() : size );
    painter.setFont( font );

    // Each starting element draws the whole token
/*    ElementType* token = getElementType();
    if ( ( token != 0 ) && !symbol ) {
        QString text = token->text( static_cast<SequenceElement*>( getParent() ) );
//         kDebug() << "draw text: " << text[0].unicode()
//                   << " " << painter.font().family().latin1()
//                   << endl;
        painter.fillRect( context.layoutUnitToPixelX( parentOrigin.x() ),
                          context.layoutUnitToPixelY( parentOrigin.y() ),
                          context.layoutUnitToPixelX( getParent()->getWidth() ),
                          context.layoutUnitToPixelY( getParent()->getHeight() ),
                          style.background() );
        painter.drawText( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getBaseline() ),
                          text );
    }
    else {
*/
        QChar ch = character();
        if ( ch != QChar::Null ) {
            luPixel bl = getBaseline();
            if ( bl == -1 ) {
                // That's quite hacky and actually not the way it's
                // meant to be. You shouldn't calculate a lot in
                // draw. But I don't see how else to deal with
                // baseline==0 glyphs without yet another flag.
                bl = -( getHeight()/2 + context.axisHeight( tstyle, factor ) );
            }
            painter.drawText( context.layoutUnitToPixelX( myPos.x() ),
                              context.layoutUnitToPixelY( myPos.y()+bl ),
                              ch );
        }
        else {
            painter.setPen( QPen( context.getErrorColor(),
                                  context.layoutUnitToPixelX( context.getLineWidth( factor ) ) ) );
            painter.drawRect( context.layoutUnitToPixelX( myPos.x() ),
                              context.layoutUnitToPixelY( myPos.y() ),
                              context.layoutUnitToPixelX( getWidth() ),
                              context.layoutUnitToPixelY( getHeight() ) );
        }
}
#endif // 0

QFont TextElement::getFont( const AttributeManager* am )
{
    QFont font;
    QVariant mathVariant = am->valueOf( "mathvariant" );
    if ( mathVariant.canConvert( QVariant::Int ) ) {
        MathVariant variant = static_cast<MathVariant>(mathVariant.toInt());
        switch ( variant ) {
        case Normal:
            font.setItalic( false );
            font.setBold( false );
            break;
        case Bold:
            font.setItalic( false );
            font.setBold( true );
            break;
        case Italic:
            font.setItalic( true );
            font.setBold( false );
            break;
        case BoldItalic:
            font.setItalic( true );
            font.setBold( true );
            break;
        }
    }
    else {
        QVariant fontFamily = am->valueOf( "fontfamily" );
        if ( fontFamily.canConvert( QVariant::String ) ) {
            QString family = fontFamily.toString();
            font = QFont( family );
        }
    }

    /*
    if ( style.customFont() ) {
        font = style.font();
    }
    else {
        font = context.getDefaultFont();
    }
    */
//    return fontStyle.symbolTable()->font( character(), font );
    return font;
}

/*
const SymbolTable& TextElement::getSymbolTable() const
{
//    return formula()->getSymbolTable();
    static SymbolTable s;
    return s;
}
*/

} // namespace KFormula
