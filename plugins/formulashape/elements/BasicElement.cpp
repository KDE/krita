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

#include "BasicElement.h"
#include "AttributeManager.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <QPainter>
#include <QVariant>

#include <kdebug.h>
#include "FormulaCursor.h"
#include "TableDataElement.h"

BasicElement::BasicElement( BasicElement* p ) : m_parentElement( p )
{
    m_scaleFactor = 1.0;
    m_scaleLevel = 1;
    m_boundingRect.setTopLeft( QPointF( 0.0, 0.0 ) );
    m_boundingRect.setWidth( 7.0 );       // standard values
    m_boundingRect.setHeight( 10.0 );
    m_displayStyle = true;
    setBaseLine( 10.0 );
}

BasicElement::~BasicElement()
{
    m_attributes.clear();
}

void BasicElement::paint( QPainter& painter, AttributeManager* )
{
    painter.save();
    painter.setBrush( QBrush( Qt::blue ) );
    painter.drawRect( QRectF(0.0, 0.0, width(), height()) );
    painter.restore();
}

void BasicElement::paintEditingHints ( QPainter& painter, AttributeManager* am )
{
    Q_UNUSED( painter )
    Q_UNUSED( am )
}

void BasicElement::layout( const AttributeManager* )
{ /* do nothing */ }

void BasicElement::stretch()
{
    foreach( BasicElement* tmpElement, childElements() ) {
        tmpElement->stretch();
    }
}


bool BasicElement::acceptCursor( const FormulaCursor& cursor )
{
    Q_UNUSED( cursor )
    return false;
}

bool BasicElement::moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor)
{
    Q_UNUSED( newcursor )
    Q_UNUSED( oldcursor )
    return false;
}

QLineF BasicElement::cursorLine(int position) const
{
    Q_UNUSED( position )
    QPointF top = absoluteBoundingRect().topLeft();
    QPointF bottom = top + QPointF( 0.0, height() );
    return QLineF(top,bottom);
}

QPainterPath BasicElement::selectionRegion(const int pos1, const int pos2) const
{
	QLineF l1=cursorLine(pos1);
	QLineF l2=cursorLine(pos2);
	//TODO: find out why doesn't work
	//QRectF r1(l1.p1(),l1.p2());
	//QRectF r2(l2.p1(),l2.p2());

	QRectF r1(l1.p1(),l2.p2());
	QRectF r2(l2.p1(),l1.p2());
	QPainterPath temp;
	temp.addRect(r1.unite(r2));
	return temp;
}


const QRectF BasicElement::absoluteBoundingRect() const
{
    QPointF neworigin = origin();
    BasicElement* tmp=parentElement();
    while (tmp) {
	neworigin+=tmp->origin();
	tmp=tmp->parentElement();
    }
    return QRectF(neworigin,QSizeF(width(),height()));
}

bool BasicElement::setCursorTo(FormulaCursor& cursor, QPointF point)
{
    Q_UNUSED( point )
    cursor.setPosition(0);
    cursor.setCurrentElement(this);
    return true;
}

bool BasicElement::replaceChild( BasicElement* oldelement, BasicElement* newelement)
{
    Q_UNUSED( oldelement )
    Q_UNUSED( newelement )
    return false;
}

const QList<BasicElement*> BasicElement::childElements() const
{
    kWarning( 39001) << "Returning no elements from BasicElement";
    return QList<BasicElement*>();
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

void BasicElement::setAttribute( const QString& name, const QVariant& value )
{
    if( name.isEmpty() || !value.canConvert( QVariant::String ) )
        return;

    if( value.isNull() )
        m_attributes.remove( name );
    else
        m_attributes.insert( name, value.toString() );
}

QString BasicElement::attribute( const QString& attribute ) const
{
    QString tmp = m_attributes.value( attribute );
    if( tmp.isEmpty() )
        return QString();

    return tmp;
}

QString BasicElement::inheritsAttribute( const QString& ) const
{
    return QString();   // do nothing
}

QString BasicElement::attributesDefaultValue( const QString& ) const
{
    return QString();  // do nothing
}

bool BasicElement::readMathML( const KoXmlElement& element )
{
    readMathMLAttributes( element );
    return readMathMLContent( element );
}

bool BasicElement::readMathMLAttributes( const KoXmlElement& element )
{
    QStringList attributeList = KoXml::attributeNames( element );
    foreach( const QString &attributeName, attributeList ) {
        m_attributes.insert( attributeName.toLower(),
                             element.attribute( attributeName ).toLower() );
    }
    return true;
}

bool BasicElement::readMathMLContent( const KoXmlElement& parent )
{
    Q_UNUSED( parent )
    return true;
}

void BasicElement::writeMathML( KoXmlWriter* writer, const QString& ns ) const
{
    if (elementType() == Basic || elementType() == Unknown) {
        return;
    }

    // Collapse a an <mrow> with only one child element to the child element itself.
    if ((elementType() == Row) && (childElements().count()==1)) {
        foreach( BasicElement* tmp, childElements() ) {
            tmp->writeMathML( writer, ns );
        }
    } else {
        const QByteArray name = ns.isEmpty() ? ElementFactory::elementName( elementType() ).toLatin1()
            : ns.toLatin1() + ":" + ElementFactory::elementName( elementType() ).toLatin1();
        writer->startElement( name );
        writeMathMLAttributes( writer );
        if ( elementType() == Formula ) {
            /*
             * This is a hack to make OOo compatible. It's mandatory
             * for OOo requires a semantics element as math element's
             * child
             */
            writer->startElement( "math:semantics" );
        }
        writeMathMLContent( writer, ns );
        if ( elementType() == Formula ) {
            writer->endElement();
        }
        writer->endElement();
    }
}

void BasicElement::writeMathMLAttributes( KoXmlWriter* writer ) const
{
    foreach( const QString &value, m_attributes )
        writer->addAttribute( m_attributes.key( value ).toLatin1(), value );
}

void BasicElement::writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const
{
    Q_UNUSED( writer )   // this is just to be reimplemented
    Q_UNUSED( ns )   // this is just to be reimplemented
}

ElementType BasicElement::elementType() const
{
    return Basic;
}

const QRectF& BasicElement::boundingRect() const
{
    return m_boundingRect;
}
const QRectF& BasicElement::childrenBoundingRect() const
{
    return m_childrenBoundingRect;
}
void BasicElement::setChildrenBoundingRect(const QRectF &rect)
{
    m_childrenBoundingRect = rect;
    Q_ASSERT(m_childrenBoundingRect.bottom() <= m_boundingRect.height());
    Q_ASSERT(m_childrenBoundingRect.right() <= m_boundingRect.width());
}
qreal BasicElement::height() const
{
    return m_boundingRect.height();
}

qreal BasicElement::width() const
{
    return m_boundingRect.width();
}

qreal BasicElement::baseLine() const
{
    return m_baseLine;
}

QPointF BasicElement::origin() const
{
    return m_boundingRect.topLeft();
}

BasicElement* BasicElement::parentElement() const
{
    return m_parentElement;
}

qreal BasicElement::scaleFactor() const
{
    return m_scaleFactor;
}
int BasicElement::scaleLevel() const
{
    return m_scaleLevel;
}

void BasicElement::setWidth( qreal width )
{
    m_boundingRect.setWidth( width );
}

void BasicElement::setHeight( qreal height )
{
    m_boundingRect.setHeight( height );
}

void BasicElement::setOrigin( QPointF origin )
{
    m_boundingRect.moveTopLeft( origin );
}

void BasicElement::setBaseLine( qreal baseLine )
{
    m_baseLine = baseLine;
}

int BasicElement::endPosition() const
{
    return 0;
}

int BasicElement::positionOfChild(BasicElement* child) const
{
    Q_UNUSED( child )
    return -1;
}

void BasicElement::setParentElement( BasicElement* parent )
{
    m_parentElement = parent;
}

void BasicElement::setScaleLevel( int scaleLevel )
{
    if(scaleLevel == m_scaleLevel) {
        return;
    }
    m_scaleLevel =  qMax(scaleLevel, 0);
    int level = scaleLevel;
    m_scaleFactor = 1.9;
    while(level-- > 0)  { //raise multiplier to the power of level
        m_scaleFactor *= 0.71;
    }
}
BasicElement* BasicElement::elementBefore ( int position ) const
{
    Q_UNUSED( position )
    return 0;
}

BasicElement* BasicElement::elementAfter ( int position ) const
{
    Q_UNUSED( position )
    return 0;
}

QList< BasicElement* > BasicElement::elementsBetween ( int pos1, int pos2 ) const
{
    Q_UNUSED( pos1 )
    Q_UNUSED( pos2 )
    QList<BasicElement*> tmp;
    return tmp;
}


bool BasicElement::displayStyle() const
{
    return m_displayStyle;
}
void BasicElement::setDisplayStyle(bool displayStyle)
{
    m_displayStyle = displayStyle;
}


bool BasicElement::hasDescendant ( BasicElement* other ) const
{
    if (other==this) {
        return true;
    }
    foreach (BasicElement* tmp, childElements()) {
        if (tmp->hasDescendant(other)) {
            return true;
        }
    }
    return false;
}


BasicElement* BasicElement::emptyDescendant()
{
    BasicElement* tmp;
    if (isEmpty() && parentElement() && parentElement()->isInferredRow()) {
        return this;
    }
    foreach (BasicElement* child, childElements()) {
        if ( (tmp=child->emptyDescendant()) ) {
            return tmp;
        }
    }
    return 0;
}

//TODO: This should be cached
BasicElement* BasicElement::formulaElement()
{
    if (parentElement()==0) {
        return this;
    } else {
        return parentElement()->formulaElement();
    }
}

bool BasicElement::isEmpty() const
{
    return false;
}


void BasicElement::setScaleFactor ( qreal scaleFactor )
{
    m_scaleFactor=scaleFactor;
}

void BasicElement::writeElementTree(int indent, bool wrong) const
{
    QString s;
    for (int i=0; i<indent; ++i) {
        s+="   ";
    }
    s+=ElementFactory::elementName(elementType());
    s+=' ';
    s+=writeElementContent();
/*    s+="        [scale level ";
    s+=QString::number(m_scaleFactor)+","+QString::number(m_scaleLevel)+"] ";*/
    s+=QString(" [")+QString::number(baseLine())+" ; " + QString::number(height())+']';
    s+=QString(" [")+ QString::number(origin().y())+']';
    if (wrong) {
        s+=" -> wrong parent !!!";
    }
    kDebug()<<s;
    foreach (BasicElement* tmp, childElements()) {
        if (tmp->parentElement()!=this) {
            tmp->writeElementTree(indent+1,true);
        } else {
            tmp->writeElementTree(indent+1,false);
        }
    }
}


const QString BasicElement::writeElementContent() const
{
    return "";
}


bool BasicElement::isInferredRow() const
{
    return false;
}

void BasicElement::cleanElementTree ( BasicElement* element )
{
    foreach (BasicElement* tmp,element->childElements()) {
        cleanElementTree(tmp);
    }
    if (element->elementType()==Row && element->parentElement() && element->parentElement()->isInferredRow()) {
        if ( element->childElements().count()==1) {
            BasicElement* parent=element->parentElement();
            parent->replaceChild(element,element->childElements()[0]);
        } else if ( element->isEmpty()) {
            RowElement* parent=static_cast<RowElement*>(element->parentElement());
            parent->removeChild(element);
        }
    }
}

TableDataElement* BasicElement::parentTableData()
{
    if (elementType()==TableData) {
        return static_cast<TableDataElement*>(this);
    } else if (parentElement()) {
        return parentElement()->parentTableData();
    } else {
        return 0;
    }
}
