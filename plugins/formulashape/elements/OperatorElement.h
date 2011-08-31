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

#ifndef OPERATORELEMENT_H
#define OPERATORELEMENT_H

#include "TokenElement.h"
#include "kformula_export.h"
#include "Dictionary.h"

/**
 * @short Implementation of the MathML mo element
 *
 * The mo element uses the Dictionary class to look up the attributes of the single
 * operator. Processed in the renderToPath method the operator element renders its
 * contents to the path. It also respects the right and left spaces. 
 */
class KOFORMULA_EXPORT OperatorElement : public TokenElement {
public:
    /// The standart constructor
    OperatorElement( BasicElement* parent = 0 );

    /**
     * Used by FenceElement to render its open, close fences as well as the separators
     * @param raw The raw string which is supposed to be rendered - might be entity
     * @param form Indicates whether raw is interpreted as fence or separator
     * @return The painter path with the rendered content
     */
    QPainterPath renderForFence( const QString& raw, Form form );

    /// @return The element's ElementType
    ElementType elementType() const;

    /// Process @p raw and render it to @p path
    QRectF renderToPath( const QString& raw, QPainterPath& path ) const;
    
    /// Inherited from TokenElement
    virtual bool insertText ( int position, const QString& text );
    
    /// Inherited from TokenElement
    virtual bool readMathMLContent ( const KoXmlElement& parent );
    
    /** Reimplemented from BaseElement
     *  Sets the height() and baseLine() of the element based on the parent size
     */
    virtual void stretch();
private:
    /// @return The Form value that was passed as QString @p value
    Form parseForm( const QString& value ) const;
    Dictionary m_dict;

private:
    Form determineOperatorForm() const;
};

#endif // OPERATORELEMENT_H
