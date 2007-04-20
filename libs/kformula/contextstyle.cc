/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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
 * Boston, MA 02110-1301, USA.
*/

#include <QFontMetrics>
#include <QPaintDevice>
#include <QString>

#include <kdebug.h>
#include <KoGlobal.h>

#include "fontstyle.h"
#include "contextstyle.h"


KFORMULA_NAMESPACE_BEGIN


ContextStyle::ContextStyle()
    : symbolFont( "Symbol" ),
      defaultColor(Qt::black), numberColor(Qt::blue),
      operatorColor(Qt::darkGreen), errorColor(Qt::darkRed),
      emptyColor(Qt::blue), helpColor( Qt::gray ), m_sizeFactor( 0 )
{
    textStyleValues[ displayStyle      ].setup( 1. );
    textStyleValues[ textStyle         ].setup( 1. );
    textStyleValues[ scriptStyle       ].setup( .7 );
    textStyleValues[ scriptScriptStyle ].setup( .49 );

    m_baseTextStyle = displayStyle;

    lineWidth = 1;
    linearMovement = false;
    centerSymbol = true;
    m_syntaxHighlighting = true;

    m_fontStyle = 0;
}


ContextStyle::~ContextStyle()
{
    delete m_fontStyle;
}


void ContextStyle::init( bool init )
{
    setup();
    setFontStyle( m_fontStyleName, init );
}


void ContextStyle::setFontStyle( const QString& fontStyle, bool init )
{
    delete m_fontStyle;
    m_fontStyleName = fontStyle;
    m_fontStyle = new FontStyle();
    m_fontStyle->init( this, init );
}


const SymbolTable& ContextStyle::symbolTable() const
{
    return *( m_fontStyle->symbolTable() );
}


void ContextStyle::readConfig( KConfig* config, bool init )
{
    config->setGroup( "kformula Font" );
    QString fontName = config->readEntry( "defaultFont", "Times,12,-1,5,50,1,0,0,0,0" );
    defaultFont.fromString( fontName );
    fontName = config->readEntry( "nameFont", "Times,12,-1,5,50,0,0,0,0,0" );
    nameFont.fromString( fontName );
    fontName = config->readEntry( "numberFont", "Times,12,-1,5,50,0,0,0,0,0" );
    numberFont.fromString( fontName );
    fontName = config->readEntry( "operatorFont", "Times,12,-1,5,50,0,0,0,0,0" );
    operatorFont.fromString( fontName );
    QString baseSize = config->readEntry( "baseSize", "20" );
    m_baseSize = baseSize.toInt();

    if ( ! FontStyle::missingFonts( init ).isEmpty() ) {
        kWarning( DEBUGID) << "Not all basic fonts found\n";
    }
    mathFont.fromString("Arev Sans");
    bracketFont.fromString("cmex10");

    // There's no gui right anymore but I'll leave it here...
    config->setGroup( "kformula Color" );
    defaultColor  = config->readEntry( "defaultColor",  defaultColor );
    numberColor   = config->readEntry( "numberColor",   numberColor );
    operatorColor = config->readEntry( "operatorColor", operatorColor );
    emptyColor    = config->readEntry( "emptyColor",    emptyColor );
    errorColor    = config->readEntry( "errorColor",    errorColor );
    helpColor     = config->readEntry( "helpColor",     helpColor );

    m_syntaxHighlighting = config->readEntry( "syntaxHighlighting", true );
}

QColor ContextStyle::getNumberColor()   const
{
    if ( edit() && syntaxHighlighting() ) {
        return numberColor;
    }
    return getDefaultColor();
}

QColor ContextStyle::getOperatorColor() const
{
    if ( edit() && syntaxHighlighting() ) {
        return operatorColor;
    }
    return getDefaultColor();
}

QColor ContextStyle::getErrorColor()    const
{
    if ( edit() && syntaxHighlighting() ) {
        return errorColor;
    }
    return getDefaultColor();
}

QColor ContextStyle::getEmptyColor()    const
{
    if ( edit() && syntaxHighlighting() ) {
        return emptyColor;
    }
    return getDefaultColor();
}

QColor ContextStyle::getHelpColor()     const
{
    if ( edit() && syntaxHighlighting() ) {
        return helpColor;
    }
    return getDefaultColor();
}

void ContextStyle::setDefaultColor( const QColor& color )
{
    defaultColor = color;
}
void ContextStyle::setNumberColor( const QColor& color )
{
    numberColor = color;
}
void ContextStyle::setOperatorColor( const QColor& color )
{
    operatorColor = color;
}
void ContextStyle::setErrorColor( const QColor& color )
{
    errorColor = color;
}
void ContextStyle::setEmptyColor( const QColor& color )
{
    emptyColor = color;
}
void ContextStyle::setHelpColor( const QColor& color )
{
    helpColor = color;
}

#if 0
const QStringList& ContextStyle::requestedFonts() const
{
    return m_requestedFonts;
}

void ContextStyle::setRequestedFonts( const QStringList& list )
{
    m_requestedFonts = list;
    //table.init( this );
}
#endif

double ContextStyle::getReductionFactor( TextStyle tstyle ) const
{
    return textStyleValues[ tstyle ].reductionFactor;
}

luPt ContextStyle::getAdjustedSize( TextStyle tstyle, double factor ) const
{
    return m_sizeFactor * m_baseSize * getReductionFactor( tstyle ) * factor;
}

luPixel ContextStyle::getSpace( TextStyle tstyle, SpaceWidth space, double factor ) const
{
    switch ( space ) {
    case NEGTHIN: return -getThinSpace( tstyle, factor );
    case THIN:    return getThinSpace( tstyle, factor );
    case MEDIUM:  return getMediumSpace( tstyle, factor );
    case THICK:   return getThickSpace( tstyle, factor );
    case QUAD:    return getQuadSpace( tstyle, factor );
    }
    return 0;
}

luPixel ContextStyle::getThinSpace( TextStyle tstyle, double factor ) const
{
    return m_sizeFactor * textStyleValues[ tstyle ].thinSpace( quad ) * factor;
}

luPixel ContextStyle::getMediumSpace( TextStyle tstyle, double factor ) const
{
    return m_sizeFactor * textStyleValues[ tstyle ].mediumSpace( quad ) * factor;
}

luPixel ContextStyle::getThickSpace( TextStyle tstyle, double factor ) const
{
    return m_sizeFactor * textStyleValues[ tstyle ].thickSpace( quad ) * factor;
}

luPixel ContextStyle::getQuadSpace( TextStyle tstyle, double factor ) const
{
    return m_sizeFactor  * textStyleValues[ tstyle ].quadSpace( quad )  * factor;
}

luPixel ContextStyle::axisHeight( TextStyle tstyle, double factor ) const
{
    return static_cast<luPixel>( m_sizeFactor
                                 * textStyleValues[ tstyle ].axisHeight( m_axisHeight )
                                 * factor );
}

luPt ContextStyle::getBaseSize() const
{
    return static_cast<luPt>( m_sizeFactor * m_baseSize );
}

void ContextStyle::setBaseSize( int size )
{
    if ( size != m_baseSize ) {
        m_baseSize = size;
        setup();
    }
}

void ContextStyle::setSizeFactor( double factor )
{
    m_sizeFactor = factor;
}


luPixel ContextStyle::getLineWidth( double factor ) const
{
    return m_sizeFactor * lineWidth * factor;
}

double ContextStyle::getEmptyRectWidth( double factor ) const
{
    return m_sizeFactor * m_baseSize * factor / 1.8;
}

double ContextStyle::getEmptyRectHeight( double factor ) const
{
    return m_sizeFactor * m_baseSize * factor / 1.8;
}


ContextStyle::TextStyle ContextStyle::convertTextStyleFraction( TextStyle tstyle ) const
{
    TextStyle result;

    switch ( tstyle ){
    case displayStyle:
	result = textStyle;
	break;
    case textStyle:
	result = scriptStyle;
	break;
    default:
	result = scriptScriptStyle;
	break;
    }

    return result;
}


ContextStyle::TextStyle ContextStyle::convertTextStyleIndex( TextStyle tstyle ) const
{
    TextStyle result;

    switch ( tstyle ){
    case displayStyle:
	result = scriptStyle;
	break;
    case textStyle:
	result = scriptStyle;
	break;
    default:
	result = scriptScriptStyle;
	break;
    }

    return result;
}


void ContextStyle::setup()
{
    luPt size = static_cast<luPt>( m_baseSize );
    QFont font = symbolFont;
    font.setPointSizeF( size );
    QFontMetrics fm( font );

    // Or better the real space required? ( boundingRect )
    quad = fm.width( 'M' );

    font = QFont(defaultFont);
    font.setPointSizeF( size );
    QFontMetrics fm2( font );
    m_axisHeight = fm2.strikeOutPos();
}

KFORMULA_NAMESPACE_END
