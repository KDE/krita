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

#include "BasicElement.h"
#include "StyleElement.h"

KFORMULA_NAMESPACE_BEGIN

StyleElement::StyleElement( BasicElement* parent ) : TokenStyleElement( parent ),
                                                     m_scriptMinSizeType( NoSize ),
                                                     m_veryVeryThinMathSpaceType( NoSize ),
                                                     m_veryThinMathSpaceType( NoSize ),
                                                     m_thinMathSpaceType( NoSize ),
                                                     m_mediumMathSpaceType( NoSize ),
                                                     m_thickMathSpaceType( NoSize ),
                                                     m_veryThickMathSpaceType( NoSize ),
                                                     m_veryVeryThickMathSpaceType( NoSize ),
                                                     m_customScriptLevel( false ),
                                                     m_relativeScriptLevel( false ),
                                                     m_customDisplayStyle( false ),
                                                     m_customScriptSizeMultiplier( false ),
                                                     m_customBackground( false )
{
}

/*
bool StyleElement::readAttributesFromMathMLDom( const QDomElement& element )
{
    if ( !BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }
    
    QString scriptlevelStr = element.attribute( "scriptlevel" );
    if ( ! scriptlevelStr.isNull() ) {
        if ( scriptlevelStr[0] == '+' || scriptlevelStr[0] == '-' ) {
            m_relativeScriptLevel = true;
        }
        bool ok;
        m_scriptLevel = scriptlevelStr.toInt( &ok );
        if ( ! ok ) {
            kdWarning( DEBUGID ) << "Invalid scriptlevel attribute value: " 
                                 << scriptlevelStr << endl;
        }
        else {
            m_customScriptLevel = true;
        }
    }
    QString displaystyleStr = element.attribute( "displaystyle" );
    if ( ! displaystyleStr.isNull() ) {
        if ( displaystyleStr.lower() == "true" ) {
            m_displayStyle = true;
        }
        else {
            m_displayStyle = false;
        }
        m_customDisplayStyle = true;
    }
    QString scriptsizemultiplierStr = element.attribute( "scriptsizemultiplier" );
    if ( ! scriptsizemultiplierStr.isNull() ) {
        bool ok;
        m_scriptSizeMultiplier = scriptsizemultiplierStr.toDouble( &ok );
        if ( ! ok ) {
            kdWarning( DEBUGID ) << "Invalid scriptsizemultiplier attribute value: " 
                                 << scriptsizemultiplierStr << endl;
        }
        else {
            m_customScriptSizeMultiplier = true;
        }
    }
    QString scriptminsizeStr = element.attribute( "scriptminsize" );
    if ( ! scriptminsizeStr.isNull() ) {
        readSizeAttribute( scriptminsizeStr, &m_scriptMinSizeType, &m_scriptMinSize );
    }
    QString backgroundStr = element.attribute( "background" );
    if ( ! backgroundStr.isNull() ) {
        // TODO: tranparent background
        m_customBackground = true;
        if ( backgroundStr[0] != '#' ) {
            m_background = QColor( getHtmlColor( backgroundStr ) );
        }
        else {
            m_background = QColor( backgroundStr );
        }
    }
    QString veryverythinmathspaceStr = element.attribute( "veryverythinmathspace" );
    if ( ! veryverythinmathspaceStr.isNull() ) {
        readSizeAttribute( veryverythinmathspaceStr, &m_veryVeryThinMathSpaceType, &m_veryVeryThinMathSpace );
    }
    QString verythinmathspaceStr = element.attribute( "verythinmathspace" );
    if ( ! verythinmathspaceStr.isNull() ) {
        readSizeAttribute( verythinmathspaceStr, &m_veryThinMathSpaceType, &m_veryThinMathSpace );
    }
    QString thinmathspaceStr = element.attribute( "thinmathspace" );
    if ( ! thinmathspaceStr.isNull() ) {
        readSizeAttribute( thinmathspaceStr, &m_thinMathSpaceType, &m_thinMathSpace );
    }
    QString mediummathspaceStr = element.attribute( "mediummathspace" );
    if ( ! mediummathspaceStr.isNull() ) {
        readSizeAttribute( mediummathspaceStr, &m_mediumMathSpaceType, &m_mediumMathSpace );
    }
    QString thickmathspaceStr = element.attribute( "thickmathspace" );
    if ( ! thickmathspaceStr.isNull() ) {
        readSizeAttribute( thickmathspaceStr, &m_thickMathSpaceType, &m_thickMathSpace );
    }
    QString verythickmathspaceStr = element.attribute( "verythickmathspace" );
    if ( ! verythickmathspaceStr.isNull() ) {
        readSizeAttribute( verythickmathspaceStr, &m_veryThickMathSpaceType, &m_veryThickMathSpace );
    }
    QString veryverythickmathspaceStr = element.attribute( "veryverythickmathspace" );
    if ( ! veryverythickmathspaceStr.isNull() ) {
        readSizeAttribute( veryverythickmathspaceStr, &m_veryVeryThickMathSpaceType, &m_veryVeryThickMathSpace );
    }
    return inherited::readAttributesFromMathMLDom( element );
}
*/

void StyleElement::writeMathMLAttributes( QDomElement& element ) const
{
    if ( m_customScriptLevel ) {
        QString prefix;
        if ( m_relativeScriptLevel && m_scriptLevel >= 0 ) {
            prefix = "+";
        }
        element.setAttribute( "scriptlevel", prefix + QString( "%1" ).arg( m_scriptLevel ) );
    }
    if ( m_customDisplayStyle ) {
        element.setAttribute( "displaystyle", m_displayStyle ? "true" : "false" );
    }
    if ( m_customScriptSizeMultiplier ) {
        element.setAttribute( "scriptsizemultiplier", QString( "%1" ).arg( m_scriptSizeMultiplier ) );
    }
    writeSizeAttribute( element, "scriptminsize", m_scriptMinSizeType, m_scriptMinSize );
    if ( m_customBackground ) {
        element.setAttribute( "background", m_background.name() );
    }
    writeSizeAttribute( element, "veryverythinmathspace", m_veryVeryThinMathSpaceType, m_veryVeryThinMathSpace );
    writeSizeAttribute( element, "verythinmathspace", m_veryThinMathSpaceType, m_veryThinMathSpace );
    writeSizeAttribute( element, "thinmathspace", m_thinMathSpaceType, m_thinMathSpace );
    writeSizeAttribute( element, "mediummathspace", m_mediumMathSpaceType, m_mediumMathSpace );
    writeSizeAttribute( element, "thickmathspace", m_thickMathSpaceType, m_thickMathSpace );
    writeSizeAttribute( element, "verythickmathspace", m_veryThickMathSpaceType, m_veryThickMathSpace );
    writeSizeAttribute( element, "veryverythickmathspace", m_veryVeryThickMathSpaceType, m_veryVeryThickMathSpace );

    inherited::writeMathMLAttributes( element );
}


void StyleElement::setStyleSize( const ContextStyle& context, StyleAttributes& style )
{
    if ( m_customScriptLevel ) {
        if ( m_relativeScriptLevel ) {
            style.setScriptLevel( style.scriptLevel() + m_scriptLevel );
        }
        else {
            style.setScriptLevel( m_scriptLevel );
        }
    }
    else {
        style.setScriptLevel( style.scriptLevel() );
    }
    if ( m_customDisplayStyle || style.customDisplayStyle() ) {
        style.setCustomDisplayStyle( true );
        if ( m_customDisplayStyle ) {
            style.setDisplayStyle( m_displayStyle );
        }
        else {
            style.setDisplayStyle( style.displayStyle() );
        }
    }
    else {
        style.setCustomDisplayStyle( false );
    }
    if ( m_customScriptSizeMultiplier ) {
        style.setScriptSizeMultiplier( m_scriptSizeMultiplier );
    }
    else {
        style.setScriptSizeMultiplier( style.scriptSizeMultiplier() );
    }

    // Get scriptminsize attribute in absolute units, so we don't depend on 
    // context to get the default value
    double basesize = context.layoutUnitPtToPt( context.getBaseSize() );
    double size = style.scriptMinSize();
    switch ( m_scriptMinSizeType ) {
    case AbsoluteSize:
        size = m_scriptMinSize;
    case RelativeSize:
        size = m_scriptMinSize * basesize;
    case PixelSize:
        size = context.pixelXToPt( m_scriptMinSize );
    default:
        break;
    }
    style.setScriptMinSize( size );

    style.setVeryVeryThinMathSpace( sizeFactor( context, 
                                                m_veryVeryThinMathSpaceType,
                                                m_veryVeryThinMathSpace,
                                                style.veryVeryThinMathSpace() ));
    style.setVeryThinMathSpace( sizeFactor( context,  m_veryThinMathSpaceType,
                                            m_veryThinMathSpace,
                                            style.veryThinMathSpace() ));
    style.setThinMathSpace( sizeFactor( context, m_thinMathSpaceType,
                                        m_thinMathSpace, 
                                        style.thinMathSpace() ));
    style.setMediumMathSpace( sizeFactor( context, m_mediumMathSpaceType,
                                          m_mediumMathSpace,
                                          style.mediumMathSpace() ));
    style.setThickMathSpace( sizeFactor( context,  m_thickMathSpaceType,
                                         m_thickMathSpace,
                                         style.thickMathSpace() ));
    style.setVeryThickMathSpace( sizeFactor( context, m_veryThickMathSpaceType,
                                             m_veryThickMathSpace,
                                             style.veryThickMathSpace() ));
    style.setVeryVeryThickMathSpace( sizeFactor( context, 
                                                 m_veryVeryThickMathSpaceType,
                                                 m_veryVeryThickMathSpace,
                                                 style.veryVeryThickMathSpace() ));
    inherited::setStyleSize( context, style );
}

double StyleElement::sizeFactor( const ContextStyle& context, SizeType st, 
                                 double length, double defvalue )
{
    double basesize = context.layoutUnitPtToPt( context.getBaseSize() );
    switch ( st ) {
    case AbsoluteSize:
        return length / basesize;
    case RelativeSize:
        return length;
    case PixelSize:
        return context.pixelXToPt( length ) / basesize;
    default:
        break;
    }
    return defvalue;
}

void StyleElement::setStyleVariant( StyleAttributes& style )
{
    if ( customMathVariant() ) {
        style.setCustomMathVariant ( true );
        style.setCustomFontWeight( false );
        style.setCustomFontStyle( false );
        style.setCustomFont( false );
        if ( customMathVariant() ) {
            style.setCharFamily ( charFamily() );
            style.setCharStyle( charStyle() );
        }
        else {
            style.setCharFamily( style.charFamily() );
            style.setCharStyle( style.charStyle() );
        }
    }
    else {
        style.setCustomMathVariant( false );
        if ( customFontFamily() ) {
            style.setCustomFont( true );
            style.setFont( QFont(fontFamily()) );
        }

        bool fontweight = false;
        if ( customFontWeight() || style.customFontWeight() ) {
            style.setCustomFontWeight( true );
            if ( customFontWeight() ) {
                fontweight = fontWeight();
            }
            else {
                fontweight = style.fontWeight();
            }
            style.setFontWeight( fontweight );
        }
        else {
            style.setCustomFontWeight( false );
        }

        bool fontstyle = false;
        if ( customFontStyle() || style.customFontStyle() ) {
            style.setCustomFontStyle( true );
            if ( customFontStyle() ) {
                fontstyle = fontStyle();
            }
            else {
                fontstyle = style.fontStyle();
            }
            style.setFontStyle( fontstyle );
        }
        else {
            style.setCustomFontStyle( false );
        }

        if ( fontweight && fontstyle ) {
            style.setCharStyle( boldItalicChar );
        }
        else if ( fontweight && ! fontstyle ) {
            style.setCharStyle( boldChar );
        }
        else if ( ! fontweight && fontstyle ) {
            style.setCharStyle( italicChar );
        }
        else {
            style.setCharStyle( normalChar );
        }
    }
}

void StyleElement::setStyleBackground( StyleAttributes& style )
{
    if ( customMathBackground() ) {
        style.setBackground( mathBackground() );
    }
    else if ( m_customBackground ) {
        style.setBackground( m_background );
    }
    else {
        style.setBackground( style.background() );
    }
}

void StyleElement::resetStyle( StyleAttributes& style )
{
    inherited::resetStyle( style );
    style.resetScriptLevel();
    style.resetScriptSizeMultiplier();
    style.resetScriptMinSize();
    style.resetVeryVeryThinMathSpace();
    style.resetVeryThinMathSpace();
    style.resetThinMathSpace();
    style.resetMediumMathSpace();
    style.resetThickMathSpace();
    style.resetVeryThickMathSpace();
    style.resetVeryVeryThickMathSpace();
    style.resetDisplayStyle();
}

void StyleElement::readSizeAttribute( const QString& str, SizeType* st, double* s )
{
    if ( st == 0 || s == 0 ){
        return;
    }
    if ( str == "small" ) {
        *st = RelativeSize;
        *s = 0.8; // ### Arbitrary size
    }
    else if ( str == "normal" ) {
        *st = RelativeSize;
        *s = 1.0;
    }
    else if ( str == "big" ) {
        *st = RelativeSize;
        *s = 1.2; // ### Arbitrary size
    }
    else {
        *s = getSize( str, st );
    }
}

void StyleElement::writeSizeAttribute( QDomElement element, const QString& str, SizeType st, double s ) const
{
    switch ( st ) {
    case AbsoluteSize:
        element.setAttribute( str, QString( "%1pt" ).arg( s ) );
        break;
    case RelativeSize:
        element.setAttribute( str, QString( "%1%" ).arg( s * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( str, QString( "%1px" ).arg( s ) );
        break;
    default:
        break;
    }
}


KFORMULA_NAMESPACE_END
