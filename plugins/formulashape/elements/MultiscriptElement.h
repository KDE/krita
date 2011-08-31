/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>
                 2009 Jeremias Epperlein <jeeree@web.de>
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

#ifndef MULTISCRIPTELEMENT_H
#define MULTISCRIPTELEMENT_H

#include "FixedElement.h"
#include "kformula_export.h"

/**
 * @short Implementation of the mmultiscript element
 */
class KOFORMULA_EXPORT MultiscriptElement : public FixedElement {
public:
    /// The standard constructor
    MultiscriptElement( BasicElement* parent = 0 );

    /// The destructor
    ~MultiscriptElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements() const;
    
    virtual bool setCursorTo ( FormulaCursor& cursor, QPointF point );

    virtual bool moveCursor ( FormulaCursor& newcursor, FormulaCursor& oldcursor );
    
//     virtual int length() const;
    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     * @param am AttributeManager containing style info
     */
    void paint( QPainter& painter, AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    void layout( const AttributeManager* am );

    /**
     * Implement the cursor behaviour for the element
     * @param cursor The FormulaCursor that is moved around
     * @return A this pointer if the element accepts if not the element to asked instead
     */
    virtual bool acceptCursor ( const FormulaCursor& cursor );

    /// @return The default value of the attribute for this element
    QString attributesDefaultValue( const QString& attribute ) const; 

    /// @return The element's ElementType
    ElementType elementType() const;

protected:
    /// Read all content from the node
    bool readMathMLContent( const KoXmlElement& element );

    /// Write all content to the KoXmlWriter
    void writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const;

    /// Make sure that there are an even number of elements, as the spec says
    /// there must be.
    void ensureEvenNumberElements();

private:
    /// The BasicElement representing the base element of the multiscript
    BasicElement* m_baseElement;

    /// A list of BasicElements representing the sub- and super-scripts left to the base
    /// element.  The first item in the list is subscript, second is superscript, third
    /// subscript and so on.
    /// The first 2 items are drawn closest to the item, then moving increasingly
    /// further away
    QList<BasicElement*> m_preScripts;

    /// A list of BasicElements representing the sub- and super-scripts right to the base
    /// element.  The first item in the list is subscript, second is superscript, third
    /// subscript and so on.
    /// The first 2 items are drawn closest to the item, then moving increasingly
    /// further away
    QList<BasicElement*> m_postScripts;
};


#endif // MULTISCRIPTELEMENT_H
