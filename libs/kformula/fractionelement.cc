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
//Added by qt3to4:
#include <Q3PtrList>

#include <kdebug.h>
#include <klocale.h>

#include "elementvisitor.h"
#include "formulaelement.h"
#include "formulacursor.h"
#include "fractionelement.h"
#include "sequenceelement.h"

KFORMULA_NAMESPACE_BEGIN
using namespace std;

FractionElement::FractionElement(BasicElement* parent)
        : BasicElement(parent), withLine(true)
{
    numerator = new SequenceElement(this);
    denominator = new SequenceElement(this);
}

FractionElement::~FractionElement()
{
    delete denominator;
    delete numerator;
}

FractionElement::FractionElement( const FractionElement& other )
    : BasicElement( other ), withLine( other.withLine )
{
    numerator = new SequenceElement( *( other.numerator ) );
    denominator = new SequenceElement( *( other.denominator ) );
    numerator->setParent( this );
    denominator->setParent( this );
}


bool FractionElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}

void FractionElement::entered( SequenceElement* child )
{
    if ( child == numerator ) {
        formula()->tell( i18n( "Numerator" ) );
    }
    else {
        formula()->tell( i18n( "Denominator" ) );
    }
}


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
}


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void FractionElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    ContextStyle::TextStyle i_tstyle = style.convertTextStyleFraction( tstyle );
    ContextStyle::IndexStyle u_istyle = style.convertIndexStyleUpper( istyle );
    ContextStyle::IndexStyle l_istyle = style.convertIndexStyleLower( istyle );

    numerator->calcSizes( style, i_tstyle, u_istyle );
    denominator->calcSizes( style, i_tstyle, l_istyle );

    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );

    setWidth( qMax( numerator->getWidth(), denominator->getWidth() ) );
    setHeight( numerator->getHeight() + denominator->getHeight() +
               2*distY + style.getLineWidth() );
    setBaseline( qRound( numerator->getHeight() + distY + .5*style.getLineWidth() +
                 style.axisHeight( tstyle ) ) );

    numerator->setX( ( getWidth() - numerator->getWidth() ) / 2 );
    denominator->setX( ( getWidth() - denominator->getWidth() ) / 2 );

    numerator->setY( 0 );
    denominator->setY( getHeight() - denominator->getHeight() );
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

    numerator->draw(painter, r, style,
		    style.convertTextStyleFraction( tstyle ),
		    style.convertIndexStyleUpper( istyle ), myPos);
    if (denominator) { // Can be temporarily 0 see FractionElement::remove
        denominator->draw(painter, r, style,
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
    numerator->dispatchFontCommand( cmd );
    denominator->dispatchFontCommand( cmd );
}

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void FractionElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            if (linear) {
                denominator->moveLeft(cursor, this);
            }
            else {
                numerator->moveLeft(cursor, this);
            }
        }
        else if (from == denominator) {
            numerator->moveLeft(cursor, this);
        }
        else {
            getParent()->moveLeft(cursor, this);
        }
    }
}


/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void FractionElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            numerator->moveRight(cursor, this);
        }
        else if (from == numerator) {
            if (linear) {
                denominator->moveRight(cursor, this);
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


/**
 * Enters this element while moving up starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or above it.
 */
void FractionElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveUp(cursor, this);
    }
    else {
        if (from == getParent()) {
            denominator->moveRight(cursor, this);
        }
        else if (from == denominator) {
            numerator->moveRight(cursor, this);
        }
        else {
            getParent()->moveUp(cursor, this);
        }
    }
}


/**
 * Enters this element while moving down starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or below it.
 */
void FractionElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveDown(cursor, this);
    }
    else {
        if (from == getParent()) {
            numerator->moveRight(cursor, this);
        }
        else if (from == numerator) {
            denominator->moveRight(cursor, this);
        }
        else {
            getParent()->moveDown(cursor, this);
        }
    }
}


/**
 * Reinserts the denominator if it has been removed.
 */
void FractionElement::insert(FormulaCursor* cursor,
                             Q3PtrList<BasicElement>& newChildren,
                             Direction direction)
{
    if (cursor->getPos() == denominatorPos) {
        denominator = static_cast<SequenceElement*>(newChildren.take(0));
        denominator->setParent(this);

        if (direction == beforeCursor) {
            denominator->moveLeft(cursor, this);
        }
        else {
            denominator->moveRight(cursor, this);
        }
        cursor->setSelection(false);
        formula()->changed();
    }
}


/**
 * Removes all selected children and returns them. Places the
 * cursor to where the children have been.
 *
 * We remove ourselve if we are requested to remove our numerator.
 *
 * It is possible to remove the denominator. But after this we
 * are senseless and the caller is required to replace us.
 */
void FractionElement::remove(FormulaCursor* cursor,
                             Q3PtrList<BasicElement>& removedChildren,
                             Direction direction)
{
    switch (cursor->getPos()) {
    case numeratorPos:
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
        break;
    case denominatorPos:
        removedChildren.append(denominator);
        formula()->elementRemoval(denominator);
        denominator = 0;
        cursor->setTo(this, denominatorPos);
        formula()->changed();
        break;
    }
}


/**
 * Returns wether the element has no more useful
 * children (except its main child) and should therefore
 * be replaced by its main child's content.
 */
bool FractionElement::isSenseless()
{
    return denominator == 0;
}


// main child
//
// If an element has children one has to become the main one.

SequenceElement* FractionElement::getMainChild()
{
    return numerator;
}

// void FractionElement::setMainChild(SequenceElement* child)
// {
//     formula()->elementRemoval(numerator);
//     numerator = child;
//     numerator->setParent(this);
//     formula()->changed();
// }


/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void FractionElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    if (child == numerator) {
        cursor->setTo(this, numeratorPos);
    }
    else if (child == denominator) {
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
    num.appendChild(numerator->getElementDom(doc));
    element.appendChild(num);

    QDomElement den = doc.createElement("DENOMINATOR");
    den.appendChild(denominator->getElementDom(doc));
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

    if ( !buildChild( numerator, node, "NUMERATOR" ) ) {
        kWarning( DEBUGID ) << "Empty numerator in FractionElement." << endl;
        return false;
    }
    node = node.nextSibling();

    if ( !buildChild( denominator, node, "DENOMINATOR" ) ) {
        kWarning( DEBUGID ) << "Empty denominator in FractionElement." << endl;
        return false;
    }
    node = node.nextSibling();

    return true;
}

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

void FractionElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mfrac": "mfrac" );
    if ( !withLine ) // why is this no function?
        de.setAttribute( "linethickness", 0 );
    numerator->writeMathML( doc, de, oasisFormat );
    denominator->writeMathML( doc, de, oasisFormat );
    parent.appendChild( de );
}

KFORMULA_NAMESPACE_END
