/* This file is part of thShapee KDE project
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

#include "UnderOverElement.h"
#include "FormulaCursor.h"
#include "AttributeManager.h"
#include <KoXmlReader.h>
#include <kdebug.h>
#include <QPainter>


UnderOverElement::UnderOverElement( BasicElement* parent, ElementType elementType )
    : FixedElement( parent )
{
    if (elementType!=Under) {
        m_overElement = new RowElement( this );
    } else {
        m_overElement = 0;
    }
    if (elementType!=Over) {
        m_underElement = new RowElement( this );
    } else {
        m_underElement = 0;
    }
    m_baseElement=new RowElement( this );
    m_elementType = elementType;
}

UnderOverElement::~UnderOverElement()
{
    delete m_baseElement;
    if (m_underElement) {
        delete m_underElement;
    }
    if (m_overElement) {
        delete m_overElement;
    }
}

const QList<BasicElement*> UnderOverElement::childElements() const
{
    QList<BasicElement*> tmp;
    tmp << m_baseElement;
    if(m_overElement) {
        tmp << m_overElement;
    }
    if(m_underElement) {
        tmp << m_underElement;
    }
    return tmp;
}

void UnderOverElement::paint( QPainter& painter, AttributeManager* am)
{
    /*do nothing as UnderOverElement has no visual representance*/
    Q_UNUSED(painter)
    Q_UNUSED(am)
}

void UnderOverElement::layout( const AttributeManager* am )
{
//     qreal thinSpace   = am->layoutSpacing( this );
//     qreal accent      = m_elementType != Under && am->boolOf( "accent", this );     //Whether to add a space above
//     qreal accentUnder = m_elementType != Over && am->boolOf( "accentunder", this );//Whether to add a space below

    // Set whether to stretch the element.  Set it to true if it doesn't exist to make it easy to check if any are non-stretchy
    bool underStretchy = m_elementType == Over || am->boolOf( "stretchy", m_underElement );
    bool overStretchy  = m_elementType == Under || am->boolOf( "stretchy", m_overElement );
    bool baseStretchy  = (underStretchy && overStretchy) || am->boolOf( "stretchy", m_baseElement );  //For sanity, make sure at least one is not stretchy

    qreal largestWidth = 0;
    if(!baseStretchy)
        largestWidth = m_baseElement->width();

    if(m_elementType != Over && !underStretchy)
        largestWidth = qMax( m_underElement->width(), largestWidth );
    if(m_elementType != Under && !overStretchy)
        largestWidth = qMax( m_overElement->width(), largestWidth );

    QPointF origin(0.0,0.0);
    if(m_elementType != Under) {
        origin.setX(( largestWidth - m_overElement->width() ) / 2.0 ) ;
        m_overElement->setOrigin( origin );
        origin.setY( m_overElement->height() );
    }

    origin.setX( ( largestWidth - m_baseElement->width() ) / 2.0 );
    m_baseElement->setOrigin( origin );
    setBaseLine( origin.y() + m_baseElement->baseLine() );

    if(m_elementType != Over) {
        origin.setX( ( largestWidth - m_underElement->width())/2.0 );
	/* Try to be smart about where to place the under */
//	if(m_baseElement->baseLine() + 1.5*thinSpace > m_baseElement->height())
//          origin.setY( origin.y() + m_baseElement->baseLine() );
//	else 
          origin.setY( origin.y() + m_baseElement->height()); 
        m_underElement->setOrigin( origin );
        setHeight( origin.y() + m_underElement->height() );
    } else {
        setHeight( origin.y() + m_baseElement->height() );
    }

    setWidth( largestWidth );
}

QString UnderOverElement::attributesDefaultValue( const QString& attribute ) const
{
    Q_UNUSED( attribute )
/*    if( m_overElement->elementType() == Operator )
    else if( m_underElement->elementType() == Operator )*/
    return "false";  // the default for accent and
}

bool UnderOverElement::readMathMLContent( const KoXmlElement& parent )
{
    KoXmlElement tmp;
    int counter=0;
    forEachElement( tmp, parent ) {
        if (counter==0) {
            loadElement(tmp,&m_baseElement);
        } else if (counter==1 && m_elementType != Over) {
            loadElement(tmp,&m_underElement);
        } else if ((counter==2 && m_elementType==UnderOver) || (counter==1 && m_elementType==Over)) {
            loadElement(tmp,&m_overElement);
        } else if ((counter==3 && m_elementType==UnderOver) || (counter==2)) {
            kDebug(39001) << "Too many arguments to "
                          << ElementFactory::elementName(m_elementType)
                          << "counter =" << counter;
            return false;
        }
        counter++;
    }
    
    if ((counter<3 && m_elementType==UnderOver) || (counter<2)) {
        kDebug(39001) << "Not enough arguments to "

                      << ElementFactory::elementName(m_elementType)
                      << "counter =" << counter
                      << "type = " << (m_elementType == Under ? "Under"
                                       : m_elementType == Over ? "Over" : "UnderOver");
        return false;
    }
    return true;
} 

void UnderOverElement::writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const
{
    m_baseElement->writeMathML( writer, ns );   // Just save the children in
    if(m_elementType != Over) {
        m_underElement->writeMathML( writer, ns );  // the right order
    }
    if(m_elementType != Under) {
        m_overElement->writeMathML( writer, ns );
    }
}

ElementType UnderOverElement::elementType() const
{
    return m_elementType;
}

bool UnderOverElement::moveCursor ( FormulaCursor& newcursor, FormulaCursor& oldcursor )
{
    int childpos=newcursor.position()/2;
    if (m_elementType==Over) {
        return moveVertSituation(newcursor,oldcursor,1,0);
    } else if (m_elementType==Under) {
        return moveVertSituation(newcursor,oldcursor,0,1);
    } else {
        switch (childpos) {
            case 1:
                return moveVertSituation(newcursor,oldcursor,1,0);
            case 0:
                if (newcursor.direction()==MoveDown) {
                    return moveVertSituation(newcursor,oldcursor,0,2);
                } else if (newcursor.direction()==MoveUp) {
                    return moveVertSituation(newcursor,oldcursor,1,0);
                } else {
                    return moveVertSituation(newcursor,oldcursor,0,1);
                }
                break;
            case 2:
                return moveVertSituation(newcursor,oldcursor,0,2);
            default:
                return false;
        }
    }
    return false;
}

int UnderOverElement::endPosition() const
{
    return m_elementType==UnderOver ? 5 : 3;
}

bool UnderOverElement::setCursorTo ( FormulaCursor& cursor, QPointF point )
{
    if (cursor.isSelecting()) {
        return false;
    }
    if (m_underElement && m_underElement->boundingRect().contains(point)) {
        return m_underElement->setCursorTo(cursor, point-m_underElement->origin());
    } else if (m_overElement && m_overElement->boundingRect().contains(point)) {
        return m_overElement->setCursorTo(cursor, point-m_overElement->origin());
    } else {
        return m_baseElement->setCursorTo(cursor, point-m_baseElement->origin());
    }
}
