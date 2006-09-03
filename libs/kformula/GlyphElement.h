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

#ifndef GLYPHELEMENT_H
#define GLYPHELEMENT_H

#include "TextElement.h"

namespace KFormula {

class GlyphElement : public TextElement {
    typedef TextElement inherited;
public:
    GlyphElement( BasicElement* parent = 0 );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& style,
						    ContextStyle::TextStyle tstyle,
						    ContextStyle::IndexStyle istyle,
							StyleAttributes& style );

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

protected:
    virtual bool readAttributesFromMathMLDom( const QDomElement &element );

private:
    virtual QString getElementName() const { return "mglyph"; }
    virtual void writeMathMLAttributes( QDomElement& element ) const ;

    QChar m_char;         // Char to be shown
    QString m_fontFamily; // Font family to use
    QString m_alt;        // Alternative text if font family not found
    bool m_hasFont;       // Whether required font is available
};

} // namespace KFormula

#endif // GLYPHELEMENT_H
