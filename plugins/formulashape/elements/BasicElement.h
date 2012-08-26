/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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

#ifndef BASICELEMENT_H
#define BASICELEMENT_H

#include "kformula_export.h"
#include "ElementFactory.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QRectF>
#include <QLineF>
class QPainter;
class QVariant;
class KoXmlWriter;
#include "KoXmlReaderForward.h"
class AttributeManager;
class FormulaCursor;
class QPainterPath;
class TableDataElement;
#define DEBUGID 40000

/**
 * @short The base class for all elements of a formula
 *
 * The BasicElement class is constructed with a parent and normally an element in a
 * formula has a parent. The only exception is FormulaElement which is the root of
 * the element tree and has no parent element.
 * Most of the elements have children but the number of it can be fixed or variable
 * and the type of child element is not certain. So with the childElements() method you
 * can obtain a list of all direct children of an element. Note that the returned list
 * can be empty when the element is eg a token. This is also the reason why each class
 * inheriting BasicElement has to implement the childElements() method on its own.
 * With the childElementAt method you can test if the given point is in the element.
 * This method is generically implemented for all element types only once in
 * BasicElement.
 * The BasicElement knows its size and position in the formula. This data is normally
 * only used for drawing and stored in the m_boundingRect attribute.
 * To adapt both variables, size and coordinates, to fit in the formula each and every
 * BasicElement derived class has to implement layoutElement().
 *
 * For cursor movement, an element has to implement elementBefore, elementAfter,
 * lastCursorPosition and positionOfChild as well as
 * moveCursor (keyboard navigation) and setCursorTo (cursor placement by clicking).
 */
class KOFORMULA_EXPORT BasicElement {
public:
    /*
     * The standard constructor
     * @param parent pointer to the BasicElement's parent
     */
    BasicElement( BasicElement* parent = 0 );

    /// The standard destructor
    virtual ~BasicElement();

    /**
     * Get the element of the formula at the given point
     * @param p the point to look for 
     * @return a pointer to a BasicElement
     */
    BasicElement* childElementAt( const QPointF& p );

    /**
     * Obtain a list of all child elements of this element - sorted in saving order
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*> childElements() const;

    /**
     * Replace a child element
     * @param oldelement the child to replace
     * @param newelement the child @p oldelement is replaced with
     */ 
    virtual bool replaceChild( BasicElement* oldelement, BasicElement* newelement );

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     * @param am AttributeManager containing style info
     */
    virtual void paint( QPainter& painter, AttributeManager* am );

    /**
     * Render the editing hints of the element to the given QPainter
     * @param painter The QPainter to paint the element to
     * @param am AttributeManager containing style info
     */
    virtual void paintEditingHints( QPainter& painter, AttributeManager* am );

    
    /**
     * Calculate the minimum size of the element and the positions of its children
     *
     * Laying out the items is done in two parts.
     *
     * First layout() is called for the topmost element, which in turn calls
     * layout() for its children, and so on. This sets the minimum size of all elements.
     *
     * Then stretch() is called for the topmost element, which in turn calls
     * stretch() for its children, and so on.  This stretches elements that
     * are stretchable, up to their maximum size.
     *
     * @param am The AttributeManager providing information about attributes values
     */
    virtual void layout( const AttributeManager* am );

    /**
     * Calculate the stretched size of the element.  This is called after layouting.
     */
    virtual void stretch();

    /**
     * Implement the cursor behaviour for the element
     * @param cursor the cursor we test
     * @return true, if the element accepts the cursor
     */
    virtual bool acceptCursor( const FormulaCursor& cursor );
    
    /**
     * Return the coordinates of the line, where the cursor should be drawn
     * in coordinates relative to the formula element (or the flake shape)
     * @param cursor The FormulaCursor specifying the position
     * @return the cursor line
     */
    virtual QLineF cursorLine(int position) const;
    
    virtual QPainterPath selectionRegion(const int pos1, const int pos2) const;
    
    virtual QList<BasicElement*> elementsBetween(int pos1, int pos2) const;
    
    /**
     * Move the cursor in the direction specified in cursor
     * @param newcursor the cursor we move around
     * @param oldcursor the former cursor position
     * @return true, if we moved the cursor
     */
    virtual bool moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor);
    
    /// @return The element's ElementType
    virtual ElementType elementType() const;

    /// Set the element's width to @p width
    void setWidth( qreal width );

    /// @return The width of the element
    qreal width() const;

    /// Set the element's height to @p height
    void setHeight( qreal height );

    /// @return The height of the element
    qreal height() const;

    /// @return The bounding rectangle of the element
    const QRectF& boundingRect() const;

    /// @return The absoulte bounding rectangle of the element
    const QRectF absoluteBoundingRect() const;

    /**
     * place the cursor at the given point
     * the point should be placed a the position in the element 
     * (or it's child) that is closest to the point
     * in particular the point doesn't have to be within 
     * boundingBox()
     * @param cursor The FormulaCursor to modify
     * @param point The point in coordinates relative to the elements local coordinate system
     * @return true, iff the cursor could be placed
     **/
    virtual bool setCursorTo(FormulaCursor& cursor, QPointF point);

    /// @return The bounding rectangle of the children, relative to the element
    const QRectF& childrenBoundingRect() const;

    /// Set the bounding rectangle of the children, relative to the element
    void setChildrenBoundingRect(const QRectF &rect);

    /// Set the element's baseline to @p baseLine
    void setBaseLine( qreal baseLine );

    /// @return The baseline of the element
    qreal baseLine() const;

    /// Set the element's origin inside the m_parentElement to @p origin
    void setOrigin( QPointF origin );

    /// @return The element's origin 
    QPointF origin() const;

    /// Set the element's m_parentElement to @p parent
    void setParentElement( BasicElement* parent );

    /// @return The parent element of this BasicElement
    BasicElement* parentElement() const;

    /// @return The last cusor position (number of available cursor positions - 1)
    virtual int endPosition() const;
    
    /** 
     * @return the cursor position before the child in this element and -1 if it isn't a child
     * @param child  the childelement we are looking for
     */
    virtual int positionOfChild(BasicElement* child) const;
        
    /// @return the element right before the cursor position @p position and 0 if there is none
    virtual BasicElement* elementBefore(int position) const;
    
    /// @return the element right after the cursor position @p position and 0 if there is none
    virtual BasicElement* elementAfter(int position) const;
    
    /// Set the element's m_scaleFactor to @p scaleFactor
    void setScaleFactor( qreal scaleFactor );

    /// @return The elements scale factor
    qreal scaleFactor() const;

    /// Set the elements scale level and sets the scale factor
    void setScaleLevel( int scaleLevel );

    /// @return The elements scale level
    int scaleLevel() const;

    /**
     * Set an attribute's value
     * @param name The name of the attribute to be set
     * @param value The value to set for the attribute
     */
    void setAttribute( const QString& name, const QVariant& value );

    /// @return The value of the attribute if it is set for this element
    QString attribute( const QString& attribute ) const;

    /// @return The value of the attribute if it is inherited
    virtual QString inheritsAttribute( const QString& attribute ) const;

    /// @return The default value of the attribute for this element
    virtual QString attributesDefaultValue( const QString& attribute ) const;

    /// Whether displaystyle is set
    bool displayStyle() const;
    
    /// Whether displaystyle is set.  This is updated by FormulaRenderer
    void setDisplayStyle(bool displayStyle);

    /// Read the element from MathML
    bool readMathML( const KoXmlElement& element );

    /// Save the element to MathML 
    void writeMathML( KoXmlWriter* writer, const QString& ns = "math" ) const;

    /// @return true, if @p other is a descendant of this element
    bool hasDescendant(BasicElement* other) const;

    /// @return first empty element, that is a descendant of this element, if there is one
    BasicElement* emptyDescendant();

    /// @return true, when the element is empty
    virtual bool isEmpty() const;

    /// @return true, if the element is an inferred mrow
    virtual bool isInferredRow() const;

    /// @return the formula element that is a descendant of this element
    BasicElement* formulaElement();

    /** writes the child element tree to kDebug()
     *  only for debugging purpose
     *  @param wrong indicates, if the parent is set wrong
     *  @param indent indention level
     */
    virtual void writeElementTree(int indent=0, bool wrong=false) const;

    /// return the content of the element to kDebug(), only for debugging
    virtual const QString writeElementContent() const;
    
    /// @return the first TableDataElement among the elements ancestors or 0 if there is none
    TableDataElement* parentTableData();

protected:
    /// Read all attributes loaded and add them to the m_attributes map 
    virtual bool readMathMLAttributes( const KoXmlElement& element );

    /// Read all content from the node - reimplemented by child elements
    virtual bool readMathMLContent( const KoXmlElement& element );

    /// Write all attributes of m_attributes to @p writer
    virtual void writeMathMLAttributes( KoXmlWriter* writer ) const;

    /// Write all content to the KoXmlWriter - reimplemented by the child elements
    virtual void writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const;

    static void cleanElementTree(BasicElement* element);
    
private:
    /// The element's parent element - might not be null except of FormulaElement
    BasicElement* m_parentElement;

    /// A hash map of all attributes where attribute name is assigned to a value
    QHash<QString,QString> m_attributes;

    /// The boundingRect storing the element's width, height, x and y
    QRectF m_boundingRect;
    
    /** The boundingRect storing the childrens element's width, height, x and y
     *  The bottomRight hand corner will always be small that then size of
     *  m_boundingRect
     */
    QRectF m_childrenBoundingRect;

    /// The position of our base line from the upper border
    qreal m_baseLine;

    /// Factor with which this element is scaled down by
    qreal m_scaleFactor;

    /// Scale level with which this element is scaled down by
    qreal m_scaleLevel;

    /// Indicates whether this element has displaystyle set
    bool m_displayStyle;
};

#endif // BASICELEMENT_H
