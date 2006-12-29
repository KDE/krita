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

#include "BasicElement.h"
#include "FormulaCursor.h"
#include <KoXmlReader.h>
#include <KoXmlWriter.h>

#include <kdebug.h>
#include "contextstyle.h"
#include "SequenceElement.h"

namespace KFormula {

BasicElement::BasicElement( BasicElement* p ) : m_baseLine( 0.0 ),
						m_parentElement( p )
{
}

BasicElement::~BasicElement()
{
    // TODO delete m_attributes 
}

void BasicElement::paint( QPainter& painter ) const
{
    // TODO paint a blue rectangle with boundingRect
    // painter.setBrush( Qt::NoBrush );
    // painter.setPen( QPen(  ) );
    // painter.drawRect( m_boundingRect );
}

void BasicElement::calculateSize()
{
    // set the bounding rect to a default value
    // m_boundingRect.setWidth( );
    // m_boundingRect.setHeight( ); 
}

void BasicElement::insertChild( FormulaCursor* cursor, BasicElement* element )
{
    m_parentElement->insertChild( cursor, element );
}

void BasicElement::removeChild( BasicElement* )
{   // do nothing a BasicElement has no children
}

const QList<BasicElement*> BasicElement::childElements() 
{
    return QList<BasicElement*>();
}

const QRectF& BasicElement::boundingRect() const
{
    return m_boundingRect;
}

double BasicElement::height() const
{
    return m_boundingRect.height();
}

double BasicElement::width() const
{
    return m_boundingRect.width();
}

double BasicElement::baseLine() const
{
    return m_baseLine;
}

QPointF BasicElement::origin() const
{
    return m_boundingRect.topLeft();
}

void BasicElement::setWidth( double width )
{
    m_boundingRect.setWidth( width );
}

void BasicElement::setHeight( double height )
{
    m_boundingRect.setHeight( height );
}

void BasicElement::setOrigin( QPointF origin )
{
    m_boundingRect.setTopLeft( origin );
}

void BasicElement::setBaseLine( double baseLine )
{
    m_baseLine = baseLine;
}

void BasicElement::setParentElement( BasicElement* parent )
{
    m_parentElement = parent;
}

ElementType BasicElement::elementType() const
{
    return m_elementType;
}

BasicElement* BasicElement::parentElement() const
{
    return m_parentElement;
}

BasicElement* BasicElement::childElementAt( const QPointF& p )
{
    if( !m_boundingRect.contains( p ) )
        return 0;
	  	
    if( childElements().isEmpty() ) 
        return this;
	      
    BasicElement* ownerElement = 0;
    foreach( BasicElement* tmpElement, childElements() )  
    {
        ownerElement = tmpElement->childElementAt( p );
	
        if( ownerElement )
            return ownerElement;
    }
    
    return this;    // if no child contains the point, it's the FormulaElement itsself
}

QString BasicElement::inheritAttribute( const QString& attribute ) const
{
    if( !m_attributes.contains( attribute ) )
        return QString();
    return m_attributes.value( attribute );
}

void BasicElement::moveLeft( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveLeft( cursor, this );
    else
        cursor->setCursorTo( this, 0 );
}

void BasicElement::moveRight( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveRight( cursor, this );
    else
        cursor->setCursorTo( this, 0 );
}

void BasicElement::moveUp( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveUp( cursor, this );
    else
        cursor->setCursorTo( this, 0 );
}

void BasicElement::moveDown( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveDown( cursor, this );
    else
        cursor->setCursorTo( this, 0 );
}

void BasicElement::moveHome( FormulaCursor* cursor )
{
    parentElement()->moveHome( cursor );
}

void BasicElement::moveEnd( FormulaCursor* cursor )
{
    parentElement()->moveEnd( cursor );
}

void BasicElement::readMathML( const KoXmlElement& element )
{
    readMathMLAttributes( element );
    KoXmlNode node = element.firstChild();
    readMathMLContent( node );
}

void BasicElement::readMathMLAttributes( const KoXmlElement& element )
{
    QStringList attributeList = KoXml::attributeNames( element );
    foreach( QString attributeName, attributeList ) {
        m_attributes.insert( attributeName, element.attribute( attributeName ) );
    }
}

int BasicElement::readMathMLContent( KoXmlNode &node )
{
    return 1;
}

void BasicElement::writeMathML( KoXmlWriter* writer, bool oasisFormat ) const
{
    QString name = oasisFormat ? "math:"  + getElementName() : getElementName();
    writer->startElement( name.toLatin1() );
    writeMathMLAttributes( writer );
    writeMathMLContent( writer, oasisFormat );
    writer->endElement();
}

void BasicElement::writeMathMLAttributes( KoXmlWriter* writer ) const
{
    foreach( QString value, m_attributes )
        writer->addAttribute( m_attributes.key( value ).toLatin1(), value );
}













void BasicElement::calcSizes( const ContextStyle& context, 
                              ContextStyle::TextStyle tstyle,
                              ContextStyle::IndexStyle istyle,
                              StyleAttributes& style )
{
}

void BasicElement::draw( QPainter& painter, const LuPixelRect& r,
                         const ContextStyle& context,
                         ContextStyle::TextStyle tstyle,
                         ContextStyle::IndexStyle istyle,
                         StyleAttributes& style,
                         const LuPixelPoint& parentOrigin )

{}

bool BasicElement::readOnly( const BasicElement* /*child*/ ) const
{
    return m_parentElement->readOnly( this );
}

/**
 * Returns our position inside the widget.
 */
LuPixelPoint BasicElement::widgetPos()
{
    luPixel x = 0;
    luPixel y = 0;
    for (BasicElement* element = this; element != 0; element = element->getParent()) {
        x += element->getX();
        y += element->getY();
    }
    return LuPixelPoint(x, y);
}

/**
  * - * Sets the cursor inside this element to its start position
  *   - * For most elements that is the main child.
  *   - */
void BasicElement::goInside(FormulaCursor* cursor)
{
    BasicElement* mainChild = getMainChild();
    if (mainChild != 0) {
      mainChild->goInside(cursor);
    }
}

/**
 * Moves the cursor to a normal place where new elements
 * might be inserted.
 */
void BasicElement::normalize(FormulaCursor* cursor, Direction direction)
{
    BasicElement* element = getMainChild();
    if (element != 0) {
        if (direction == beforeCursor) {
            element->moveLeft(cursor, this);
        }
        else {
            element->moveRight(cursor, this);
        }
    }
}

QDomElement BasicElement::getElementDom( QDomDocument& doc)
{
    QDomElement de = doc.createElement(getTagName());
    writeDom(de);
    return de;
}

bool BasicElement::buildFromDom(QDomElement element)
{
    if (element.tagName() != getTagName()) {
        kWarning( DEBUGID ) << "Wrong tag name " << element.tagName().toLatin1() << " for " << getTagName().toLatin1() << ".\n";
        return false;
    }
    if (!readAttributesFromDom(element)) {
        return false;
    }
    QDomNode node = element.firstChild();
    return readContentFromDom(node);
}

/**
 * Appends our attributes to the dom element.
 */
void BasicElement::writeDom(QDomElement)
{
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool BasicElement::readAttributesFromDom(QDomElement)
{
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool BasicElement::readContentFromDom(QDomNode&)
{
    return true;
}


/**
 * Returns a SequenceElement constructed from the nodes first child
 * if the nodes name matches the given name.
 */
bool BasicElement::buildChild( SequenceElement* child, QDomNode node, const QString & name )
{
    if (node.isElement()) {
        QDomElement e = node.toElement();
        if (e.tagName().toUpper() == name) {
            QDomNode nodeInner = e.firstChild();
            if (nodeInner.isElement()) {
                QDomElement element = nodeInner.toElement();
                return child->buildFromDom( element );
            }
        }
    }
    return false;
}



void BasicElement::setX( double x )
{
    m_boundingRect.setX( x );
}

void BasicElement::setY( double y )
{
    m_boundingRect.setY( y );
}

double BasicElement::getHeight() const
{
    return m_boundingRect.height();
}

double BasicElement::getWidth() const
{
    return m_boundingRect.width();
}

double BasicElement::getY() const
{
    return m_boundingRect.y();
}

double BasicElement::getX() const
{
    return m_boundingRect.x();
}


/**
 * Utility function that sets the size type and returns the size value from
 * a MathML attribute string with unit as defined in Section 2.4.4.2
 *
 * @returns the size value
 *
 * @param str the attribute string.
 * @param st size type container. It will be properly assigned to its size
 * type or NoSize if str is invalid
 */
double BasicElement::getSize( const QString& str, SizeType* st )
{
    int index = str.find( "%" );
    if ( index != -1 ) {
        return str2size( str, st, index, RelativeSize ) / 100.0;
    }
    index = str.find( "pt", 0, false );
    if ( index != -1 ) {
        return str2size( str, st, index, AbsoluteSize );
    }
    index = str.find( "mm", 0, false );
    if ( index != -1 ) {
        return str2size( str, st, index, AbsoluteSize ) * 72.0 / 20.54;
    }
    index = str.find( "cm", 0, false );
    if ( index != -1 ) {
        return str2size( str, st, index, AbsoluteSize ) * 72.0 / 2.54;
    }
    index = str.find( "in", 0, false );
    if ( index != -1 ) {
        return str2size( str, st, index, AbsoluteSize ) * 72.0;
    }
    index = str.find( "em", 0, false );
    if ( index != -1 ) {
        return str2size( str, st, index, RelativeSize );
    }
    index = str.find( "ex", 0, false );
    if ( index != -1 ) {
        return str2size( str, st, index, RelativeSize );
    }
    index = str.find( "pc", 0, false );
    if ( index != -1 ) {
        return str2size( str, st, index, AbsoluteSize ) * 12.0;
    }
    index = str.find( "px", 0, false );
    if ( index != -1 ) {
        return str2size( str, st, index, PixelSize );
    }
    // If there's no unit, assume 'pt'
    return str2size( str, st, str.length(),AbsoluteSize );
}

SizeType BasicElement::getSpace( const QString& str )
{
    if ( str == "negativeveryverythinmathspace" ) {
        return NegativeVeryVeryThinMathSpace;
    }
    if ( str == "negativeverythinmathspace" ) {
        return NegativeVeryThinMathSpace;
    }
    if ( str == "negativethinmathspace" ) {
        return NegativeThinMathSpace;
    }
    if ( str == "negativemediummathspace" ) {
        return NegativeMediumMathSpace;
    }
    if ( str == "negativethickmathspace" ) {
        return NegativeThickMathSpace;
    }
    if ( str == "negativeverythickmathspace" ) {
        return NegativeVeryThickMathSpace;
    }
    if ( str == "negativeveryverythickmathspace" ) {
        return NegativeVeryVeryThickMathSpace;
    }
    if ( str == "veryverythinmathspace" ) {
        return VeryVeryThinMathSpace;
    }
    if ( str == "verythinmathspace" ) {
        return VeryThinMathSpace;
    }
    if ( str == "thinmathspace" ) {
        return ThinMathSpace;
    }
    if ( str == "mediummathspace" ) {
        return MediumMathSpace;
    }
    if ( str == "thickmathspace" ) {
        return ThickMathSpace;
    }
    if ( str == "verythickmathspace" ) {
        return VeryThickMathSpace;
    }
    if ( str == "veryverythickmathspace" ) {
        return VeryVeryThickMathSpace;
    }
    return NoSize;
}


/**
 * Used internally by getSize()
 */
double BasicElement::str2size( const QString& str, SizeType *st, uint index, SizeType type )
{
    QString num = str.left( index );
    bool ok;
    double size = num.toDouble( &ok );
    if ( ok ) {
        if ( st ) {
            *st = type;
        }
        return size;
    }
    if ( st ) {
        *st = NoSize;
    }
    return -1;
}

} // namespace KFormula
