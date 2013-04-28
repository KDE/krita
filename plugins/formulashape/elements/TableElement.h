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

#ifndef TABLEELEMENT_H
#define TABLEELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"
#include <QPainterPath>

class TableRowElement;
	
/**
 * @short A matrix or table element in a formula
 *
 * A table element contains a list of rows which are of class TableRowElement.
 * These rows contain single entries which are of class TableEntryElement. The
 * TableElement takes care that the different TableRowElements are informed how
 * to lay out their children correctly as they need to be synced.
 */
class KOFORMULA_EXPORT TableElement : public BasicElement {
public:
    /// The standard constructor
    explicit TableElement(BasicElement *parent = 0);
    
    /// The standard destructor
    ~TableElement();

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
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements() const;

    /// inherited from BasicElement
    virtual bool acceptCursor( const FormulaCursor& cursor );
    
    /// inherited from BasicElement
    virtual int positionOfChild(BasicElement* child) const;
    
    /// inherited from BasicElement
    virtual int endPosition() const;
    
    /// inherited from BasicElement
    virtual bool setCursorTo(FormulaCursor& cursor, QPointF point);
    
    virtual bool insertChild ( int position, BasicElement* child );
    bool removeChild (BasicElement* child); 
    
    /// inherited from BasicElement
    virtual bool moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor);
    
    /// @return The default value of the attribute for this element
    QString attributesDefaultValue( const QString& attribute ) const;

    /// @return The width of the column with the index @p column
    qreal columnWidth( int column );

    /// @return The height of the @p TableRowElement
    qreal rowHeight( TableRowElement* row );
    
    /// inherited from BasicElement
    virtual QLineF cursorLine ( int position ) const;
    
    /// inherited from BasicElement
    virtual QPainterPath selectionRegion ( const int pos1, const int pos2 ) const;
    
    /// @return The element's ElementType
    virtual ElementType elementType() const;
    
protected:
    /// Read all content from the node - reimplemented by child elements
    bool readMathMLContent( const KoXmlElement& element );

    /// Write all content to the KoXmlWriter - reimplemented by the child elements
    void writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const;

private:
    /// @return The base line computed out of the align attribute
    qreal parseTableAlign() const;

    /// Calculate the dimensions of each row and column at a centralized point
    void determineDimensions();

    /// Storage for heights of each row calculated in determineDimensions()
    QList<qreal> m_rowHeights;

    /// Storage for widths of each column calculated in determineDimensions()
    QList<qreal> m_colWidths;
    
    /// The rows a matrix contains
    QList<TableRowElement*> m_rows;

    /// Buffer for the pen style used for the table's frame
    Qt::PenStyle m_framePenStyle;

    QList<Qt::PenStyle> m_rowLinePenStyles;

    QList<Qt::PenStyle> m_colLinePenStyles;
};

#endif // TABLEELEMENT_H
