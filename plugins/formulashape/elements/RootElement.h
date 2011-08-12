/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>
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

#ifndef ROOTELEMENT_H
#define ROOTELEMENT_H

#include "BasicElement.h"
#include "FixedElement.h"
#include "kformula_export.h"

#include <QPainterPath>

/**
 * @short Implementation of the MathML mroot and msqrt elements 
 */
class KOFORMULA_EXPORT RootElement : public FixedElement {
public:
    /// The standard constructor
    RootElement( BasicElement* parent = 0 );

    /// The standard destructor
    ~RootElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements() const;
    
//     QList<BasicElement*> elementsBetween(int pos1, int pos2) const;

    /// inherited from BasicElement
    virtual bool replaceChild ( BasicElement* oldelement, BasicElement* newelement );

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    void paint( QPainter& painter, AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    void layout( const AttributeManager* am );

    /// inherited from BasicElement
    virtual bool setCursorTo(FormulaCursor& cursor, QPointF point);

    /// inherited from BasicElement
    virtual bool moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor);

//     virtual QLineF cursorLine(int position) const;

    /// inherited from BasicElement
//     virtual int positionOfChild(BasicElement* child) const;

    /// @return The element's ElementType
    ElementType elementType() const;

    /// @return The element's length
    virtual int endPosition() const;

protected:
    ///update the selection in cursor so that a proper range is selected
//     void fixSelection (FormulaCursor& cursor);
    
    /// Read root contents - reimplemented from BasicElement
    bool readMathMLContent( const KoXmlElement& element );

    /// Write root contents - reimplemented from BasicElement
    void writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const;

private:
    /// The element that is the radicand of the root
    RowElement* m_radicand;

    /// The element that is the exponent of the root
    RowElement* m_exponent;

    /// The point the artwork relates to.
    QPointF m_rootOffset;

    /// The QPainterPath that holds the lines for the root sign   
    QPainterPath m_rootSymbol;

    /// Line thickness, in pixels
    qreal m_lineThickness;
};

#endif // ROOTELEMENT_H
