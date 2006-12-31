/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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

void ContextStyle::setZoomAndResolution( int zoom, int dpiX, int dpiY )
{
    KoZoomHandler::setZoomAndResolution( zoom, dpiX, dpiY );
}

bool ContextStyle::setZoomAndResolution( int zoom, double zoomX, double zoomY, bool, bool )
{
    bool changes = m_zoom != zoom || m_zoomedResolutionX != zoomX || m_zoomedResolutionY != zoomY;
    m_zoom = zoom;
    m_zoomedResolutionX = zoomX;
    m_zoomedResolutionY = zoomY;
    return changes;
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
    return ptToLayoutUnitPt( m_sizeFactor * m_baseSize 
                             * getReductionFactor( tstyle ) * factor );
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
    return ptToPixelX( m_sizeFactor 
                       * textStyleValues[ tstyle ].thinSpace( quad ) 
                       * factor );
}

luPixel ContextStyle::getMediumSpace( TextStyle tstyle, double factor ) const
{
    return ptToPixelX( m_sizeFactor
                       * textStyleValues[ tstyle ].mediumSpace( quad )
                       * factor );
}

luPixel ContextStyle::getThickSpace( TextStyle tstyle, double factor ) const
{
    return ptToPixelX( m_sizeFactor
                       * textStyleValues[ tstyle ].thickSpace( quad )
                       * factor );
}

luPixel ContextStyle::getQuadSpace( TextStyle tstyle, double factor ) const
{
    return ptToPixelX( m_sizeFactor 
                       * textStyleValues[ tstyle ].quadSpace( quad ) 
                       * factor );
}

luPixel ContextStyle::axisHeight( TextStyle tstyle, double factor ) const
{
    //return ptToPixelY( textStyleValues[ tstyle ].axisHeight( m_axisHeight ) );
    return static_cast<luPixel>( m_sizeFactor
                                 * textStyleValues[ tstyle ].axisHeight( m_axisHeight )
                                 * factor );
}

luPt ContextStyle::getBaseSize() const
{
    return static_cast<luPt>( ptToLayoutUnitPt( m_sizeFactor*m_baseSize ) );
}

void ContextStyle::setBaseSize( int size )
{
    //kDebug( 40000 ) << "ContextStyle::setBaseSize" << endl;
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
    return ptToLayoutUnitPixX( m_sizeFactor * lineWidth * factor );
}

luPixel ContextStyle::getEmptyRectWidth( double factor ) const
{
    return ptToLayoutUnitPixX( m_sizeFactor * m_baseSize * factor / 1.8 );
}

luPixel ContextStyle::getEmptyRectHeight( double factor ) const
{
    return ptToLayoutUnitPixX( m_sizeFactor * m_baseSize * factor / 1.8 );
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
    quad = ptToLayoutUnitPt( fm.width( 'M' ) );

    font = QFont(defaultFont);
    font.setPointSizeF( size );
    QFontMetrics fm2( font );
    //m_axisHeight = ptToLayoutUnitPt( fm2.strikeOutPos() );
    //ptToLayoutUnitPixY
    //m_axisHeight = ptToLayoutUnitPt( pixelYToPt( fm2.strikeOutPos() ) );
    m_axisHeight = ptToLayoutUnitPixY( pixelYToPt( fm2.strikeOutPos() ) );
}

// copied from KoTextZoomHandler.h

double ContextStyle::pixelToLayoutUnitX( double x ) const
{
    // Layout text at 1440 DPI
    // Well, not really always 1440 DPI, but always 20 times the point size
    // This is constant, no need to litterally apply 1440 DPI at all resolutions.
    double m_layoutUnitFactor = 20.0;
	
    return qRound( ( x * m_layoutUnitFactor * m_resolutionX ) / m_zoomedResolutionX );
}

double ContextStyle::pixelToLayoutUnitY( double y ) const
{
    // Layout text at 1440 DPI
    // Well, not really always 1440 DPI, but always 20 times the point size
    // This is constant, no need to litterally apply 1440 DPI at all resolutions.
    double m_layoutUnitFactor = 20.0;
    return qRound( ( y * m_layoutUnitFactor * m_resolutionY ) / m_zoomedResolutionY );
}

double ContextStyle::layoutUnitToPixelX( double lupix ) const
{
    // Layout text at 1440 DPI
    // Well, not really always 1440 DPI, but always 20 times the point size
    // This is constant, no need to litterally apply 1440 DPI at all resolutions.
    double m_layoutUnitFactor = 20.0;
    return ( lupix * m_zoomedResolutionX ) / ( m_layoutUnitFactor * m_resolutionX );
}

double ContextStyle::layoutUnitToPixelY( double lupix ) const
{
    // Layout text at 1440 DPI
    // Well, not really always 1440 DPI, but always 20 times the point size
    // This is constant, no need to litterally apply 1440 DPI at all resolutions.
    double m_layoutUnitFactor = 20.0;
    return ( lupix * m_zoomedResolutionY ) / ( m_layoutUnitFactor * m_resolutionY );
}

double ContextStyle::layoutUnitToFontSize( double luSize, bool /*forPrint*/ ) const
{
    // Qt will use QPaintDevice::x11AppDpiY() to go from pt to pixel for fonts
    return layoutUnitPtToPt( luSize ) * m_zoomedResolutionY;
/*    #ifdef Q_WS_X11
        / POINT_TO_INCH(QPaintDevice::x11AppDpiY())
    #endif
      ;*/
}

double StyleAttributes::sizeFactor() const
{
    if ( m_size.empty() ) {
//        kWarning( DEBUGID ) << "SizeFactor stack is empty.\n";
        return 1.0;
    }
    return m_size.top();
}

bool StyleAttributes::customMathVariant() const
{
    if ( m_customMathVariant.empty() ) {
        return false;
    }
    return m_customMathVariant.top();
}

CharStyle StyleAttributes::charStyle() const
{
    if ( m_charStyle.empty() ) {
//        kWarning( DEBUGID ) << "CharStyle stack is empty.\n";
        return anyChar;
    }
    return m_charStyle.top();
}

CharFamily StyleAttributes::charFamily() const
{
    if ( m_charFamily.empty() ) {
//        kWarning( DEBUGID ) << "CharFamily stack is empty.\n";
        return anyFamily;
    }
    return m_charFamily.top();
}

QColor StyleAttributes::color() const
{
    if ( m_color.empty() ) {
//        kWarning( DEBUGID ) << "Color stack is empty.\n";
        return QColor( Qt::black );
        //return getDefaultColor();
    }
    return m_color.top();
}

QColor StyleAttributes::background() const
{
    if ( m_background.empty() ) {
//        kWarning( DEBUGID ) << "Background stack is empty.\n";
        return QColor( Qt::color0 );
    }
    return m_background.top();
}

QFont StyleAttributes::font() const
{
    if ( m_font.empty() ) {
        return QFont();
    }
    return m_font.top();
}

bool StyleAttributes::fontWeight() const
{
    if ( m_fontWeight.empty() ) {
        return false;
    }
    return m_fontWeight.top();
}

bool StyleAttributes::customFontWeight() const
{
    if ( m_customFontWeight.empty() ) {
        return false;
    }
    return m_customFontWeight.top();
}

bool StyleAttributes::fontStyle() const
{
    if ( m_fontStyle.empty() ) {
        return false;
    }
    return m_fontStyle.top();
}

bool StyleAttributes::customFontStyle() const
{
    if ( m_customFontStyle.empty() ) {
        return false;
    }
    return m_customFontStyle.top();
}

bool StyleAttributes::customFont() const
{
    if ( m_customFontFamily.empty() ) {
        return false;
    }
    return m_customFontFamily.top();
}

int StyleAttributes::scriptLevel() const
{
    if ( m_scriptLevel.empty() ) {
        return 0;
    }
    return m_scriptLevel.top();
}

double StyleAttributes::scriptSizeMultiplier() const
{
    if ( m_scriptSizeMultiplier.empty() ) {
        return scriptsizemultiplier;
    }
    return m_scriptSizeMultiplier.top();
}

double StyleAttributes::scriptMinSize() const
{
    if ( m_scriptMinSize.empty() ) {
        return scriptminsize;
    }
    return m_scriptMinSize.top();
}

double StyleAttributes::veryVeryThinMathSpace() const
{
    if ( m_veryVeryThinMathSpace.empty() ) {
        return veryverythinmathspace;
    }
    return m_veryVeryThinMathSpace.top();
}

double StyleAttributes::veryThinMathSpace() const
{
    if ( m_veryThinMathSpace.empty() ) {
        return verythinmathspace;
    }
    return m_veryThinMathSpace.top();
}

double StyleAttributes::thinMathSpace() const
{
    if ( m_thinMathSpace.empty() ) {
        return thinmathspace;
    }
    return m_thinMathSpace.top();
}

double StyleAttributes::mediumMathSpace() const
{
    if ( m_mediumMathSpace.empty() ) {
        return mediummathspace;
    }
    return m_mediumMathSpace.top();
}

double StyleAttributes::thickMathSpace() const
{
    if ( m_thickMathSpace.empty() ) {
        return thickmathspace;
    }
    return m_thickMathSpace.top();
}

double StyleAttributes::veryThickMathSpace() const
{
    if ( m_veryThickMathSpace.empty() ) {
        return verythickmathspace;
    }
    return m_veryThickMathSpace.top();
}

double StyleAttributes::veryVeryThickMathSpace() const
{
    if ( m_veryVeryThickMathSpace.empty() ) {
        return veryverythickmathspace;
    }
    return m_veryVeryThickMathSpace.top();
}

bool StyleAttributes::displayStyle() const
{
    if ( m_displayStyle.empty() ) {
        return true;
    }
    return m_displayStyle.top();
}

bool StyleAttributes::customDisplayStyle() const
{
    if ( m_customDisplayStyle.empty() ) {
        return false;
    }
    return m_customDisplayStyle.top();
}

double StyleAttributes::getSpace( SizeType type, double length ) const
{
    switch ( type ) {
    case NegativeVeryVeryThinMathSpace:
        return - veryVeryThinMathSpace();
    case NegativeVeryThinMathSpace:
        return - veryThinMathSpace();
    case NegativeThinMathSpace:
        return - thinMathSpace();
    case NegativeMediumMathSpace:
        return - mediumMathSpace();
    case NegativeThickMathSpace:
        return - thickMathSpace();
    case NegativeVeryThickMathSpace:
        return - veryThickMathSpace();
    case NegativeVeryVeryThickMathSpace:
        return - veryVeryThickMathSpace();
    case VeryVeryThinMathSpace:
        return veryVeryThinMathSpace();
    case VeryThinMathSpace:
        return veryThinMathSpace();
    case ThinMathSpace:
        return thinMathSpace();
    case MediumMathSpace:
        return mediumMathSpace();
    case ThickMathSpace:
        return thickMathSpace();
    case VeryThickMathSpace:
        return veryThickMathSpace();
    case VeryVeryThickMathSpace:
        return veryVeryThickMathSpace();
    default:
        break;
    }
    return length;
}

void StyleAttributes::resetSize()
{
    if ( ! m_size.empty() ) {
        m_size.pop();
    }
}

void StyleAttributes::resetCharStyle()
{
    if ( ! m_charStyle.empty() ) {
        m_charStyle.pop();
    }
}

void StyleAttributes::resetCharFamily()
{
    if ( ! m_charFamily.empty() ) {
        m_charFamily.pop();
    }
}

void StyleAttributes::resetColor()
{
    if ( ! m_color.empty() ) {
        m_color.pop();
    }
}

void StyleAttributes::resetBackground()
{
    if ( ! m_background.empty() ) {
        m_background.pop();
    }
}

void StyleAttributes::resetFontFamily()
{
    if ( ! m_customFontFamily.empty() ) {
        if ( m_customFontFamily.pop() ) {
            if ( ! m_font.empty() ) {
                m_font.pop();
            }
        }
    }
}

void StyleAttributes::resetFontWeight()
{
    if ( ! m_customFontWeight.empty() ) {
        if ( m_customFontWeight.pop() ) {
            if ( ! m_fontWeight.empty() ) {
                m_fontWeight.pop();
            }
        }
    }
}

void StyleAttributes::resetFontStyle()
{
    if ( ! m_customFontStyle.empty() ) {
        if ( m_customFontStyle.pop() ) {
            if ( ! m_fontStyle.empty() ) {
                m_fontStyle.pop();
            }
        }
    }
}

void StyleAttributes::resetScriptLevel()
{
    if ( ! m_scriptLevel.empty() ) {
        m_scriptLevel.pop();
    }
}

void StyleAttributes::resetScriptSizeMultiplier()
{
    if ( ! m_scriptSizeMultiplier.empty() ) {
        m_scriptSizeMultiplier.pop();
    }
}

void StyleAttributes::resetScriptMinSize()
{
    if ( ! m_scriptMinSize.empty() ) {
        m_scriptMinSize.pop();
    }
}

void StyleAttributes::resetVeryVeryThinMathSpace()
{
    if ( ! m_veryVeryThinMathSpace.empty() ) {
        m_veryVeryThinMathSpace.pop();
    }
}

void StyleAttributes::resetVeryThinMathSpace()
{
    if ( ! m_veryThinMathSpace.empty() ) {
        m_veryThinMathSpace.pop();
    }
}

void StyleAttributes::resetThinMathSpace()
{
    if ( ! m_thinMathSpace.empty() ) {
        m_thinMathSpace.pop();
    }
}

void StyleAttributes::resetMediumMathSpace()
{
    if ( ! m_mediumMathSpace.empty() ) {
        m_mediumMathSpace.pop();
    }
}

void StyleAttributes::resetThickMathSpace()
{
    if ( ! m_thickMathSpace.empty() ) {
        m_thickMathSpace.pop();
    }
}

void StyleAttributes::resetVeryThickMathSpace()
{
    if ( ! m_veryThickMathSpace.empty() ) {
        m_veryThickMathSpace.pop();
    }
}

void StyleAttributes::resetVeryVeryThickMathSpace()
{
    if ( ! m_veryVeryThickMathSpace.empty() ) {
        m_veryVeryThickMathSpace.pop();
    }
}

void StyleAttributes::resetDisplayStyle()
{
    if ( ! m_customDisplayStyle.empty() ) {
        if ( m_customDisplayStyle.pop() ) {
            if ( ! m_displayStyle.empty() ) {
                m_displayStyle.pop();
            }
        }
    }
}


KFORMULA_NAMESPACE_END
