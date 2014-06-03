/* This file is part of the KDE project
   Copyright (C) 2006-2009 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "kformula_export.h"
#include "TokenElement.h"
#include <QPainterPath>

/**
 * @short Implementation of the MathML mglyph element
 *
 * GlyphElement uses the Qt font database classes to load the additional fonts to
 * display its contents.
 */
class KOFORMULA_EXPORT GlyphElement : public TokenElement {
public:
    /// The standart constructor
    explicit GlyphElement(BasicElement *parent = 0);

    /// @return The element's ElementType
    ElementType elementType() const;

    /// Process @p raw and render it to @p path
    QRectF renderToPath( const QString& raw, QPainterPath& path ) const;

    /// get width of character, for layouting
    qreal getWidth(const AttributeManager *am);


private:
    bool readMathMLAttributes( const KoXmlElement& element );
    void writeMathMLAttributes( KoXmlWriter* writer ) const;
    void writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const;

    QChar m_char;         // Char to be shown
    QString m_fontFamily; // Font family to use
    QString m_alt;        // Alternative text if font family not found
    bool m_hasFont;       // Whether required font is available

};

#endif // GLYPHELEMENT_H
