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

#ifndef TOKENSTYLEELEMENT_H
#define TOKENSTYLEELEMENT_H

#include "kformuladefs.h"
#include "SequenceElement.h"

KFORMULA_NAMESPACE_BEGIN

/**
 * This class handles mathematical style attributes common to token elements,
 * as explained in MathML Spec, Section 3.2.2.
 *
 * It is in charge of reading and saving elements' attributes and setting
 * rendering information according to these attributes.
 */
class TokenStyleElement : public SequenceElement {
    typedef SequenceElement inherited;
public:

    TokenStyleElement( BasicElement* parent = 0 );

    virtual void calcSizes( const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            StyleAttributes& style );

    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

protected:
//    virtual bool readAttributesFromMathMLDom( const QDomElement &element );
    virtual void writeMathMLAttributes( QDomElement& element ) const ;

    void setAbsoluteSize( double s, bool fontsize = false );
    void setRelativeSize( double s, bool fontsize = false );
    void setPixelSize( double s, bool fontsize = false );

    void setCharStyle( CharStyle cs ) { 
        m_charStyle = cs;
        m_customMathVariant = true; 
    }
    CharStyle charStyle() const { return m_charStyle; }

    void setCharFamily( CharFamily cf ) { 
        m_charFamily = cf; 
        m_customMathVariant = true;
    }
    CharFamily charFamily() const { return m_charFamily; }

    void setMathColor( const QColor& c ) {
        m_mathColor = c; 
        m_customMathColor = true;
    }
    QColor mathColor() const { return m_mathColor; }

    void setMathBackground( const QColor& bg ) { 
        m_mathBackground = bg; 
        m_customMathBackground = true;
    }
    QColor mathBackground() const { return m_mathBackground; }

    void setFontWeight( bool w ) { 
        m_fontWeight = w;
        m_customFontWeight = true; 
    }
    bool fontWeight() const { return m_fontWeight; }

    void setFontStyle( bool s ) { 
        m_fontStyle = s;
        m_customFontStyle = true;
    }
    bool fontStyle() const { return m_fontStyle; }

    void setFontFamily( const QString& s ) { 
        m_fontFamily = s;
        m_customFontFamily = true;
    }
    QString fontFamily() const { return m_fontFamily; }

    void setColor( const QColor& c ) { 
        m_color = c; 
        m_customColor = true;
    }
    QColor color() const { return m_color; }

    bool customMathVariant() const { return m_customMathVariant; }
    bool customMathColor() const { return m_customMathColor; }
    bool customMathBackground() const { return m_customMathBackground; }
    bool customFontWeight() const { return m_customFontWeight; }
    bool customFontStyle() const { return m_customFontStyle; }
    bool customFontFamily() const { return m_customFontFamily; }
    bool customColor() const { return m_customColor; }

    virtual void setStyleSize( const ContextStyle& context, StyleAttributes& style );
    /**
     * Set the mathvariant related info in style stacks, including info for
     * deprecated attributes. It may be redefined by token elements whose
     * behaviour differs from default one (e.g. identifiers)
     */
    virtual void setStyleVariant( StyleAttributes& style );

    void setStyleColor( StyleAttributes& style );
    virtual void setStyleBackground( StyleAttributes& style );

    virtual void resetStyle( StyleAttributes& style );
    /**
     * Return RGB string from HTML Colors. See HTML Spec, section 6.5
     */
    QString getHtmlColor( const QString& colorStr );

private:

    double sizeFactor( const ContextStyle& context, double factor );

    // MathML 2.0 attributes
    SizeType m_mathSizeType;
    double m_mathSize;
    CharStyle m_charStyle;
    CharFamily m_charFamily;
    QColor m_mathColor;
    QColor m_mathBackground;

    // Deprecated MathML 1.01 attributes
    SizeType m_fontSizeType;
    double m_fontSize;
    QString m_fontFamily;
    QColor m_color;
    bool m_fontWeight;
    bool m_fontStyle;

    // MathML 2.0 attributes set ?
    bool m_customMathVariant;
    bool m_customMathColor;
    bool m_customMathBackground;

    // Deprecated MathML 1.01 attributes set ?
    bool m_customFontWeight;
    bool m_customFontStyle;
    bool m_customFontFamily;
    bool m_customColor;
};

KFORMULA_NAMESPACE_END

#endif // TOKENSTYLEELEMENT_H
