/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#ifndef BASICELEMENT_H
#define BASICELEMENT_H

#include <KoXmlReader.h>
#include <QHash>
#include <QList>
#include <QString>
#include <QRectF>
#include <QVariant>
class QPainter;
class KoXmlWriter;

namespace FormulaShape {

class AttributeManager;
class FormulaCursor;

enum ElementType {
    Basic,
    Formula,
    Row,
    Space,
    Fraction,
    Matrix,
    MatrixRow,
    MatrixEntry,
    UnderOver,
    MultiScript,
    Root
};


/**
 * @short The base class for all elements of a formula
 *
 * The BasicElement class is constructed with a parent and normally an element in a
 * formula has a parent. The only exception is the @see FormulaElement which is the
 * root of the element tree and has no parent element.
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
 * BasicElement derived class has to implement layoutElement() and calculateSize()
 * methods. The former adaptes the position, means the coordinates, when the formula
 * changes and the latter calculates the size of the element. After a formula change
 * first calculateSize is called for all elements then layoutElement().
 */
class BasicElement {
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
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*> childElements();

    /**
     * Insert a new child at the cursor position
     * @param cursor The cursor holding the position where to inser
     * @param child A BasicElement to insert
     */
    virtual void insertChild( FormulaCursor* cursor, BasicElement* child );
   
    /**
     * Remove a child element
     * @param element The BasicElement to remove
     */ 
    virtual void removeChild( BasicElement* element );

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    virtual void paint( QPainter& painter, const AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    virtual void layout( const AttributeManager* am );
    
    /**
     * Move the FormulaCursor left
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveLeft( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor right 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveRight( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor up 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveUp( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor down 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveDown( FormulaCursor* cursor, BasicElement* from );

    /// @return The element's ElementType
    virtual ElementType elementType() const;
    
    /// @return The height of the element
    double height() const;

    /// @return The width of the element
    double width() const;

    /// @return The baseline of the element
    double baseLine() const;

    /// @return The element's origin 
    QPointF origin() const;

    /// @return The bounding rectangle of the element
    const QRectF& boundingRect() const;

    /// Set the element's width to @p width
    void setWidth( double width );

    /// Set the element's height to @p height
    void setHeight( double height );
    
    /// Set the element's baseline to @p baseLine
    void setBaseLine( double baseLine );

    /// Set the element's origin inside the m_parentElement to @p origin
    void setOrigin( QPointF origin );

    /// Set the element's m_parentElement to @p parent
    void setParentElement( BasicElement* parent );

    /// @return The parent element of this BasicElement
    BasicElement* parentElement() const;

    /**
     * Set an attribute's value
     * @param name The name of the attribute to be set
     * @param value The value to set for the attribute
     */
    void setAttribute( const QString& name, QVariant value );

    /// @return The value of the attribute if it is set for this element
    QString attribute( const QString& attribute ) const;

    /// @return The value of the attribute if it is inherited
    virtual QString inheritsAttribute( const QString& attribute ) const;

    /// @return The default value of the attribute for this element
    virtual QVariant attributesDefaultValue( const QString& attribute ) const;
    
    /// Read the element from MathML
    virtual void readMathML( const KoXmlElement& element );

    /// Save the element to MathML 
    virtual void writeMathML( KoXmlWriter* writer, bool oasisFormat = false ) const;

    /// @returns MathML element tag name
    virtual QString getElementName() const { return "mrow"; }

protected:
    /// Read all attributes loaded and add them to the m_attributes map 
    void readMathMLAttributes( const KoXmlElement& element );

    /// Write all attributes of m_attributes to @p writer
    void writeMathMLAttributes( KoXmlWriter* writer ) const;

    virtual int readMathMLContent( KoXmlNode& node );
    virtual void writeMathMLContent( KoXmlWriter* , bool ) const {};

private:
    /// The element's parent element - might not be null except of FormulaElement
    BasicElement* m_parentElement;

    /// A hash map of all attributes where attribute name is assigned to a value
    QHash<QString,QString> m_attributes;

    /// The boundingRect storing the element's width, height, x and y
    QRectF m_boundingRect;
   
    /// The position of our base line from the upper border
    double m_baseLine;
};

} // namespace FormulaShape

#endif // BASICELEMENT_H
