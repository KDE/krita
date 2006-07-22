/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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
 * Boston, MA 02110-1301, USA.
*/

#include <QPainter>

#include <kdebug.h>
#include <klocale.h>

#include "elementvisitor.h"
#include "FormulaElement.h"
#include "FormulaCursor.h"
#include "FractionElement.h"
#include "SequenceElement.h"

KFORMULA_NAMESPACE_BEGIN
using namespace std;

FractionElement::FractionElement(BasicElement* parent)
        : BasicElement(parent), withLine(true)
{
    m_numerator = 0;
    m_denominator = 0;
}

FractionElement::~FractionElement()
{
    m_denominator = 0;
    m_numerator = 0;
}

FractionElement::FractionElement( const FractionElement& other )
    : BasicElement( other ), withLine( other.withLine )
{
/*    numerator = new SequenceElement( *( other.numerator ) );
    denominator = new SequenceElement( *( other.denominator ) );
    numerator->setParent( this );
    denominator->setParent( this );*/
}

const QList<BasicElement*>& FractionElement::childElements()
{
    QList<BasicElement*> list;
    list << m_denominator << m_numerator;
    return list;
}

void FractionElement::entered( SequenceElement* child )
{
    if ( child == m_numerator ) {
        formula()->tell( i18n( "Numerator" ) );
    }
    else {
        formula()->tell( i18n( "Denominator" ) );
    }
}

void FractionElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    ContextStyle::TextStyle i_tstyle = style.convertTextStyleFraction( tstyle );
    ContextStyle::IndexStyle u_istyle = style.convertIndexStyleUpper( istyle );
    ContextStyle::IndexStyle l_istyle = style.convertIndexStyleLower( istyle );

    m_numerator->calcSizes( style, i_tstyle, u_istyle );
    m_denominator->calcSizes( style, i_tstyle, l_istyle );

    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );

    setWidth( qMax( m_numerator->getWidth(), m_denominator->getWidth() ) );
    setHeight( m_numerator->getHeight() + m_denominator->getHeight() +
               2*distY + style.getLineWidth() );
    setBaseline( qRound( m_numerator->getHeight() + distY + .5*style.getLineWidth() +
                 style.axisHeight( tstyle ) ) );

    m_numerator->setX( ( getWidth() - m_numerator->getWidth() ) / 2 );
    m_denominator->setX( ( getWidth() - m_denominator->getWidth() ) / 2 );

    m_numerator->setY( 0 );
    m_denominator->setY( getHeight() - m_denominator->getHeight() );
}


/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void FractionElement::draw( QPainter& painter, const LuPixelRect& r,
                            const ContextStyle& style,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    m_numerator->draw(painter, r, style,
		    style.convertTextStyleFraction( tstyle ),
		    style.convertIndexStyleUpper( istyle ), myPos);
    if (m_denominator) { // Can be temporarily 0 see FractionElement::remove
        m_denominator->draw(painter, r, style,
		      style.convertTextStyleFraction( tstyle ),
		      style.convertIndexStyleLower( istyle ), myPos);
    }

    if ( withLine ) {
        painter.setPen( QPen( style.getDefaultColor(),
                              style.layoutUnitToPixelY( style.getLineWidth() ) ) );
        painter.drawLine( style.layoutUnitToPixelX( myPos.x() ),
                          style.layoutUnitToPixelY( myPos.y() + axis( style, tstyle ) ),
                          style.layoutUnitToPixelX( myPos.x() + getWidth() ),
                          style.layoutUnitToPixelY( myPos.y() + axis( style, tstyle ) ) );
    }
}


void FractionElement::dispatchFontCommand( FontCommand* cmd )
{
    m_numerator->dispatchFontCommand( cmd );
    m_denominator->dispatchFontCommand( cmd );
}

void FractionElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            if (linear) {
                m_denominator->moveLeft(cursor, this);
            }
            else {
                m_numerator->moveLeft(cursor, this);
            }
        }
        else if (from == m_denominator) {
            m_numerator->moveLeft(cursor, this);
        }
        else {
            getParent()->moveLeft(cursor, this);
        }
    }
}

void FractionElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            m_numerator->moveRight(cursor, this);
        }
        else if (from == m_numerator) {
            if (linear) {
                m_denominator->moveRight(cursor, this);
            }
            else {
                getParent()->moveRight(cursor, this);
            }
        }
        else {
            getParent()->moveRight(cursor, this);
        }
    }
}

void FractionElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveUp(cursor, this);
    }
    else {
        if (from == getParent()) {
            m_denominator->moveRight(cursor, this);
        }
        else if (from == m_denominator) {
            m_numerator->moveRight(cursor, this);
        }
        else {
            getParent()->moveUp(cursor, this);
        }
    }
}

void FractionElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveDown(cursor, this);
    }
    else {
        if (from == getParent())
            m_numerator->moveRight(cursor, this);
        else if (from == m_numerator) 
            m_denominator->moveRight(cursor, this);
        else
            getParent()->moveDown(cursor, this);
    }
}

void FractionElement::insert(FormulaCursor* cursor,
                             QList<BasicElement*>& newChildren,
                             Direction direction)
{
    if (cursor->getPos() == denominatorPos) {
        m_denominator = static_cast<SequenceElement*>(newChildren.takeAt(0));
        m_denominator->setParent(this);

        if (direction == beforeCursor) {
            m_denominator->moveLeft(cursor, this);
        }
        else {
            m_denominator->moveRight(cursor, this);
        }
        cursor->setSelection(false);
        formula()->changed();
    }
}

void FractionElement::remove(FormulaCursor* cursor,
                             QList<BasicElement*>& removedChildren,
                             Direction direction)
{
    switch (cursor->getPos()) {
    case numeratorPos:
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
        break;
    case denominatorPos:
        removedChildren.append(m_denominator);
        formula()->elementRemoval(m_denominator);
        m_denominator = 0;
        cursor->setTo(this, denominatorPos);
        formula()->changed();
        break;
    }
}

bool FractionElement::isSenseless()
{
    return m_denominator == 0;
}

SequenceElement* FractionElement::getMainChild()
{
    return m_numerator;
}

/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void FractionElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    if (child == m_numerator) {
        cursor->setTo(this, numeratorPos);
    }
    else if (child == m_denominator) {
        cursor->setTo(this, denominatorPos);
    }
}


/**
 * Appends our attributes to the dom element.
 */
void FractionElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    QDomDocument doc = element.ownerDocument();
    if (!withLine) element.setAttribute("NOLINE", 1);

    QDomElement num = doc.createElement("NUMERATOR");
    num.appendChild(m_numerator->getElementDom(doc));
    element.appendChild(num);

    QDomElement den = doc.createElement("DENOMINATOR");
    den.appendChild(m_denominator->getElementDom(doc));
    element.appendChild(den);
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool FractionElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    QString lineStr = element.attribute("NOLINE");
    if(!lineStr.isNull()) {
        withLine = lineStr.toInt() == 0;
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool FractionElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    if ( !buildChild( m_numerator, node, "NUMERATOR" ) ) {
        kWarning( DEBUGID ) << "Empty numerator in FractionElement." << endl;
        return false;
    }
    node = node.nextSibling();

    if ( !buildChild( m_denominator, node, "DENOMINATOR" ) ) {
        kWarning( DEBUGID ) << "Empty denominator in FractionElement." << endl;
        return false;
    }
    node = node.nextSibling();

    return true;
}

void FractionElement::writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mfrac": "mfrac" );
    if ( !withLine ) // why is this no function?
        de.setAttribute( "linethickness", 0 );
    m_numerator->writeMathML( doc, de, oasisFormat );
    m_denominator->writeMathML( doc, de, oasisFormat );
    parent.appendChild( de );
}

/*
QString FractionElement::toLatex()
{
    if ( withLine ) {
        return "\\frac{" + numerator->toLatex() +"}{" + denominator->toLatex() + "}";
    }
    else {
        return "{" + numerator->toLatex() + "\\atop " + denominator->toLatex() + "}";
    }
}

QString FractionElement::formulaString()
{
    return "(" + numerator->formulaString() + ")/(" + denominator->formulaString() + ")";
}
*/
/*
BasicElement* FractionElement::goToPos( FormulaCursor* cursor, bool& handled,
                                        const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());
        e = numerator->goToPos(cursor, handled, point, myPos);
        if (e != 0) {
            return e;
        }
        e = denominator->goToPos(cursor, handled, point, myPos);
        if (e != 0) {
            return e;
        }

        luPixel dx = point.x() - myPos.x();
        luPixel dy = point.y() - myPos.y();

        // the positions after the numerator / denominator
        if ((dx > numerator->getX()) &&
            (dy < numerator->getHeight())) {
            numerator->moveLeft(cursor, this);
            handled = true;
            return numerator;
        }
        else if ((dx > denominator->getX()) &&
                 (dy > denominator->getY())) {
            denominator->moveLeft(cursor, this);
            handled = true;
            return denominator;
        }

        return this;
    }
    return 0;
}*/


}  // namespace KFormula
