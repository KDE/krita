/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#ifndef ROWELEMENT_H
#define ROWELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"

class FormulaCursor;

/**
 * @short Implementation of the MathML mrow element
 *
 * The mrow element is specified in the MathML spec section 3.3.1. It primarily
 * serves as container for other elements that are grouped together and aligned
 * in a row.
 * mrow has no own visual representance that is why the paint() method is empty.
 * The handling of background and other global attributes is done generically
 * inside FormulaRenderer.
 * The layout() method implements the layouting and size calculation for the mrow
 * element. The calculations assume that spacing is done per element and therefore
 * do not handle it.
 * At the moment there is no linebreaking implementation in RowElement.
 */
class KOFORMULA_EXPORT RowElement : public BasicElement {
public:
    /// The standard constructor
    RowElement( BasicElement* parent = 0 );

    /// The standard destructor
    virtual ~RowElement();

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    virtual void layout( const AttributeManager* am );

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     * @param am AttributeManager containing style info
     */
    virtual void paint( QPainter& painter, AttributeManager* am );
    
    /// inherited from BasicElement
    virtual void paintEditingHints ( QPainter& painter, AttributeManager* am );
    
    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements() const;

    /// inherited from BasicElement
    virtual QList< BasicElement* > elementsBetween ( int pos1, int pos2 ) const;

    /// inherited from BasicElement
    virtual bool insertChild( int position, BasicElement* child );

    /**
     * Remove a child element
     * @param element The BasicElement to remove
     */
    bool removeChild( BasicElement* child);

    ///inherited form BasicElement
    virtual bool replaceChild ( BasicElement* oldelement, BasicElement* newelement );

    /**
     * Implement the cursor behaviour for the element
     * @param direction Indicates whether the cursor moves up, down, right or left
     * @return A this pointer if the element accepts if not the element to asked instead
     */
    bool acceptCursor( const FormulaCursor& cursor );

    /// inherited from BasicElement
    virtual bool moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor);

    /// inherited from BasicElement
    virtual bool setCursorTo(FormulaCursor& cursor, QPointF point);

    /// @return The element's ElementType
    ElementType elementType() const;

    /** Reimplemented from parent class
     *
     *  This stretches the children inside, then readjusts their vertical offsets
     */
    virtual void stretch();

    /// inherited from BasicElement
    virtual int endPosition() const;

    /// inherited from BasicElement
    virtual int positionOfChild(BasicElement* child) const;

    /// inherited from BasicElement
    virtual QLineF cursorLine(int position) const;

    /// inherited from BasicElement
    virtual BasicElement* elementAfter ( int position ) const;

    /// inherited from BasicElement
    virtual BasicElement* elementBefore ( int position ) const;

    /// inherited from BasicElement
    virtual bool isEmpty() const;

    /// inherited from Basic
    virtual bool isInferredRow() const;

protected:
    /// Read contents of the row element
    bool readMathMLContent( const KoXmlElement& parent );

    /// Write all content to the KoXmlWriter - reimplemented by the child elements
    void writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const;

    /// A list of the child elements
    QList<BasicElement*> m_childElements;
};

#endif // ROWELEMENT_H

