/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#include "SubSupElement.h"
#include "FormulaCursor.h"
#include "AttributeManager.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <QPainter>
#include <kdebug.h>

SubSupElement::SubSupElement( BasicElement* parent, ElementType elementType )
    : FixedElement( parent )
{
    m_baseElement = new RowElement( this );
    if (elementType != SupScript) {
        m_subScript = new RowElement( this );
    } else {
        m_subScript = 0;
    }
    if (elementType != SubScript) {
        m_superScript = new RowElement( this );
    } else {
        m_superScript = 0;
    }
    m_elementType = elementType;
}

SubSupElement::~SubSupElement()
{
    delete m_baseElement;
    if (m_subScript) {
        delete m_subScript;
    }
    if (m_superScript) {
        delete m_superScript;
    }
}

void SubSupElement::paint( QPainter& painter, AttributeManager* am )
{ 
    Q_UNUSED(painter)
    Q_UNUSED(am)
    /*do nothing as this element has no visual representation*/
}

void SubSupElement::layout( const AttributeManager* am )
{
    // Get the minimum amount of shifting
    qreal subscriptshift   = am->doubleOf( "subscriptshift", this ); 
    qreal superscriptshift = am->doubleOf( "superscriptshift", this );
    qreal halfthinSpace   = 0;

    if(m_elementType == SubSupScript) {
        //Add half a thin space between both sup and superscript, so there is a minimum
        //of a whole thin space between them.
        halfthinSpace = am->layoutSpacing( this )/2.0;
    }

    // The yOffset is the amount the base element is moved down to make
    // room for the superscript
    qreal yOffset = 0;
    if(m_superScript) {
        yOffset = m_superScript->height() - m_baseElement->height()/2 + halfthinSpace;
        yOffset = qMax( yOffset, superscriptshift );
    }
    qreal largestWidth = 0;
    if(m_subScript) {
        largestWidth = m_subScript->width();
    }
    if(m_superScript) {
        largestWidth = qMax( largestWidth, m_superScript->width());
        m_superScript->setOrigin( QPointF( m_baseElement->width(), 0) );
    }

    setWidth( m_baseElement->width() + largestWidth );
    setBaseLine( yOffset + m_baseElement->baseLine() );
    m_baseElement->setOrigin( QPointF( 0, yOffset ) );


    if(m_subScript) {
        qreal yPos = yOffset + qMax( m_baseElement->height()/2 + halfthinSpace, 
                                     m_baseElement->height() - m_subScript->baseLine() 
                                     + subscriptshift );
        m_subScript->setOrigin( QPointF( m_baseElement->width(), yPos ) );
	setHeight( yPos + m_subScript->height() );
    } else {
        setHeight( yOffset + m_baseElement->height() );
    }
}

const QList<BasicElement*> SubSupElement::childElements() const
{
    QList<BasicElement*> tmp;
    tmp << m_baseElement;
    if (m_subScript) {
        tmp << m_subScript;
    }
    if (m_superScript) {
        tmp << m_superScript;
    }
    return tmp;
}

bool SubSupElement::replaceChild ( BasicElement* oldelement, BasicElement* newelement )
{
    //TODO: investigate, if we really need this check
    if (newelement->elementType() == Row) {
        RowElement* newrow = static_cast<RowElement*>(newelement);
        if (oldelement == m_baseElement) {
            m_baseElement = newrow;
            return true;
        } else if (oldelement == m_subScript ) {
            m_subScript = newrow;
            return true;
        } else if (oldelement == m_superScript ) {
            m_superScript = newrow;
            return true;
        }
    }
    return false;
}

QString SubSupElement::attributesDefaultValue( const QString& attribute ) const
{
    Q_UNUSED( attribute )
    return QString();
}

ElementType SubSupElement::elementType() const
{
    return m_elementType;
}

bool SubSupElement::readMathMLContent( const KoXmlElement& parent )
{
    KoXmlElement tmp;
    int counter = 0;
    forEachElement( tmp, parent ) {
        switch (counter) {
        case 0:
            loadElement(tmp, &m_baseElement);
            break;
        case 1:
            if (m_elementType == SupScript) {
                loadElement(tmp, &m_superScript);
            }
            else {
                // Valid for both Subscript and Subsupscript
                loadElement(tmp, &m_subScript);
            }
            break;
        case 2:
            if (m_elementType==SubSupScript) {
                loadElement(tmp, &m_superScript);
            }
            else {
                kDebug(39001) << "Too many arguments to "
                              << ElementFactory::elementName(m_elementType);
            }
            break;
        default:
            kDebug(39001) << "Too many arguments to "
                          << ElementFactory::elementName(m_elementType);
        }
        counter++;
    }

    // Check for too few arguments.
    if ((counter < 3 && m_elementType == SubSupScript) || (counter < 2)) {
        kDebug(39001) << "Not enough arguments to "<< ElementFactory::elementName(m_elementType);
        return false;
    }

    return true;
}

void SubSupElement::writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const
{
    // just save the children in the right order
    m_baseElement->writeMathML( writer, ns );

    if (m_elementType != SupScript)
        m_subScript->writeMathML( writer, ns );

    if (m_elementType != SubScript )
        m_superScript->writeMathML( writer, ns );
}


int SubSupElement::endPosition() const
{
    return (m_elementType==SubSupScript ? 5 : 3);
}


bool SubSupElement::setCursorTo(FormulaCursor& cursor, QPointF point)
{
    if (cursor.isSelecting()) {
        return false;
    }
    if (m_subScript && m_subScript->boundingRect().contains(point)) {
        return m_subScript->setCursorTo(cursor,point-m_subScript->origin());
    } else if (m_superScript && m_superScript->boundingRect().contains(point)) {
        return m_superScript->setCursorTo(cursor,point-m_superScript->origin());
    } else {
        return m_baseElement->setCursorTo(cursor,point-m_baseElement->origin());
    }
    return false;
}


bool SubSupElement::moveCursor ( FormulaCursor& newcursor, FormulaCursor& oldcursor )
{
    // 0^1_2
    int childpos=newcursor.position()/2;

    switch( newcursor.direction()) {
    case MoveUp:
    case MoveDown:
        if (m_elementType==SubScript) {
            return moveHorSituation(newcursor,oldcursor,1,0);
        } else if (m_elementType==SupScript) {
            return moveHorSituation(newcursor,oldcursor,0,1);
        } else {
            switch (childpos) {
            case 1:
            case 2:
                return moveVertSituation(newcursor,oldcursor,1,2);
            case 0:
                if (newcursor.direction()==MoveDown) {
                    return moveHorSituation(newcursor,oldcursor,1,0);
                } else {
                    return moveHorSituation(newcursor,oldcursor,0,2);
                }
            }
        }
        break;
    case MoveLeft:
    case MoveRight:
        switch (childpos) {
        case 0:
            return moveHorSituation(newcursor,oldcursor,0,1);
        case 1:
            return moveHorSituation(newcursor,oldcursor,0,1);
        case 2:
            return moveHorSituation(newcursor,oldcursor,0,2);
        }
        break;
    default:
        break;
    }
    return false;
}
