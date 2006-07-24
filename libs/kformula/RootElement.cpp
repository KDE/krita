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
#include <QPen>

#include <kdebug.h>
#include <klocale.h>

#include "elementvisitor.h"
#include "FormulaCursor.h"
#include "FormulaElement.h"
#include "kformulacommand.h"
#include "RootElement.h"
#include "SequenceElement.h"

KFORMULA_NAMESPACE_BEGIN


class RootSequenceElement : public SequenceElement {
    typedef SequenceElement inherited;
public:

    RootSequenceElement( BasicElement* parent = 0 ) : SequenceElement( parent ) {}
    virtual RootSequenceElement* clone() {
        return new RootSequenceElement( *this );
    }

    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
    virtual KCommand* buildCommand( Container*, Request* );
};


KCommand* RootSequenceElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        return 0;
    }

    switch ( *request ) {
    case req_addIndex: {
        FormulaCursor* cursor = container->activeCursor();
        if ( cursor->isSelection() ||
             ( cursor->getPos() > 0 && cursor->getPos() < countChildren() ) ) {
            break;
        }
        IndexRequest* ir = static_cast<IndexRequest*>( request );
        if ( ir->index() == upperLeftPos ) {
            RootElement* element = static_cast<RootElement*>( getParent() );
            ElementIndexPtr index = element->getIndex();
            if ( !index->hasIndex() ) {
                KFCAddGenericIndex* command = new KFCAddGenericIndex( container, index );
                return command;
            }
            else {
                index->moveToIndex( cursor, afterCursor );
                cursor->setSelection( false );
                formula()->cursorHasMoved( cursor );
                return 0;
            }
        }
    }
    default:
        break;
    }
    return inherited::buildCommand( container, request );
}


RootElement::RootElement(BasicElement* parent)
    : BasicElement(parent)
{
    content = new RootSequenceElement( this );
    index = 0;
}

RootElement::~RootElement()
{
    delete index;
    delete content;
}


RootElement::RootElement( const RootElement& other )
    : BasicElement( other )
{
    content = new RootSequenceElement( *dynamic_cast<RootSequenceElement*>( other.content ) );
    if ( other.index ) {
        index = new SequenceElement( *( other.index ) );
        index->setParent( this );
    }
    else {
        index = 0;
    }
}

const QList<BasicElement*>& RootElement::childElements()
{
    return QList<BasicElement*>();
}

void RootElement::drawInternal()
{
}


/*
bool RootElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}
*/

void RootElement::entered( SequenceElement* child )
{
    if ( child == content ) {
        formula()->tell( i18n( "Main list of root" ) );
    }
    else {
        formula()->tell( i18n( "Index" ) );
    }
}

/*
BasicElement* RootElement::goToPos( FormulaCursor* cursor, bool& handled,
                                    const LuPixelPoint& point, const LuPixelPoint& parentOrigin)
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());

        e = content->goToPos(cursor, handled, point, myPos);
        if (e != 0) {
            return e;
        }
        if (hasIndex()) {
            e = index->goToPos(cursor, handled, point, myPos);
            if (e != 0) {
                return e;
            }
        }

        //int dx = point.x() - myPos.x();
        luPixel dy = point.y() - myPos.y();

        // the position after the index
        if (hasIndex()) {
            if (dy < index->getHeight()) {
                index->moveLeft(cursor, this);
                handled = true;
                return index;
            }
        }

        return this;
    }
    return 0;
}*/


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void RootElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    content->calcSizes(style, tstyle,
		       style.convertIndexStyleLower(istyle));

    luPixel indexWidth = 0;
    luPixel indexHeight = 0;
    if (hasIndex()) {
	index->calcSizes(style,
			 style.convertTextStyleIndex(tstyle),
			 style.convertIndexStyleUpper(istyle));
        indexWidth = index->getWidth();
        indexHeight = index->getHeight();
    }

    luPixel distX = style.ptToPixelX( style.getThinSpace( tstyle ) );
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );
    luPixel unit = (content->getHeight() + distY)/ 3;

    if (hasIndex()) {
        if (indexWidth > unit) {
            index->setX(0);
            rootOffset.setX( indexWidth - unit );
        }
        else {
            index->setX( ( unit - indexWidth )/2 );
            rootOffset.setX(0);
        }
        if (indexHeight > unit) {
            index->setY(0);
            rootOffset.setY( indexHeight - unit );
        }
        else {
            index->setY( unit - indexHeight );
            rootOffset.setY(0);
        }
    }
    else {
        rootOffset.setX(0);
        rootOffset.setY(0);
    }

    setWidth( content->getWidth() + unit+unit/3+ rootOffset.x() + distX/2 );
    setHeight( content->getHeight() + distY*2 + rootOffset.y() );

    content->setX( rootOffset.x() + unit+unit/3 );
    content->setY( rootOffset.y() + distY );
    setBaseline(content->getBaseline() + content->getY());
}

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void RootElement::draw( QPainter& painter, const LuPixelRect& r,
                        const ContextStyle& style,
                        ContextStyle::TextStyle tstyle,
                        ContextStyle::IndexStyle istyle,
                        const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    content->draw(painter, r, style, tstyle,
		  style.convertIndexStyleLower(istyle), myPos);
    if (hasIndex()) {
        index->draw(painter, r, style,
		    style.convertTextStyleIndex(tstyle),
		    style.convertIndexStyleUpper(istyle), myPos);
    }

    luPixel x = myPos.x() + rootOffset.x();
    luPixel y = myPos.y() + rootOffset.y();
    //int distX = style.getDistanceX(tstyle);
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );
    luPixel unit = (content->getHeight() + distY)/ 3;

    painter.setPen( QPen( style.getDefaultColor(),
                          style.layoutUnitToPixelX( 2*style.getLineWidth() ) ) );
    painter.drawLine( style.layoutUnitToPixelX( x+unit/3 ),
                      style.layoutUnitToPixelY( y+unit+distY/3 ),
                      style.layoutUnitToPixelX( x+unit/2+unit/3 ),
                      style.layoutUnitToPixelY( myPos.y()+getHeight() ) );

    painter.setPen( QPen( style.getDefaultColor(),
                          style.layoutUnitToPixelY( style.getLineWidth() ) ) );

    painter.drawLine( style.layoutUnitToPixelX( x+unit+unit/3 ),
                      style.layoutUnitToPixelY( y+distY/3 ),
                      style.layoutUnitToPixelX( x+unit/2+unit/3 ),
                      style.layoutUnitToPixelY( myPos.y()+getHeight() ) );
    painter.drawLine( style.layoutUnitToPixelX( x+unit+unit/3 ),
                      style.layoutUnitToPixelY( y+distY/3 ),
                      style.layoutUnitToPixelX( x+unit+unit/3+content->getWidth() ),
                      style.layoutUnitToPixelY( y+distY/3 ) );
    painter.drawLine( style.layoutUnitToPixelX( x+unit/3 ),
                      style.layoutUnitToPixelY( y+unit+distY/2 ),
                      style.layoutUnitToPixelX( x ),
                      style.layoutUnitToPixelY( y+unit+unit/2 ) );
}


void RootElement::dispatchFontCommand( FontCommand* cmd )
{
    content->dispatchFontCommand( cmd );
    if (hasIndex()) {
        index->dispatchFontCommand( cmd );
    }
}

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void RootElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            content->moveLeft(cursor, this);
        }
        else if (from == content) {
            if (linear && hasIndex()) {
                index->moveLeft(cursor, this);
            }
            else {
                getParent()->moveLeft(cursor, this);
            }
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
void RootElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            if (linear && hasIndex()) {
                index->moveRight(cursor, this);
            }
            else {
                content->moveRight(cursor, this);
            }
        }
        else if (from == index) {
            content->moveRight(cursor, this);
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
void RootElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveUp(cursor, this);
    }
    else {
        if (from == getParent()) {
            content->moveRight(cursor, this);
        }
        else if (from == content) {
            if (hasIndex()) {
                index->moveRight(cursor, this);
            }
            else {
                getParent()->moveUp(cursor, this);
            }
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
void RootElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveDown(cursor, this);
    }
    else {
        if (from == getParent()) {
            if (hasIndex()) {
                index->moveRight(cursor, this);
            }
            else {
                content->moveRight(cursor, this);
            }
        }
        else if (from == index) {
            content->moveRight(cursor, this);
        }
        else {
            getParent()->moveDown(cursor, this);
        }
    }
}

/**
 * Reinserts the index if it has been removed.
 */
void RootElement::insert(FormulaCursor* cursor,
                         QList<BasicElement*>& newChildren,
                         Direction direction)
{
    if (cursor->getPos() == upperLeftPos) {
        index = static_cast<SequenceElement*>(newChildren.takeAt(0));
        index->setParent(this);

        if (direction == beforeCursor) {
            index->moveLeft(cursor, this);
        }
        else {
            index->moveRight(cursor, this);
        }
        cursor->setSelection(false);
        formula()->changed();
    }
}

/**
 * Removes all selected children and returns them. Places the
 * cursor to where the children have been.
 *
 * We remove ourselve if we are requested to remove our content.
 */
void RootElement::remove(FormulaCursor* cursor,
                         QList<BasicElement*>& removedChildren,
                         Direction direction)
{
    switch (cursor->getPos()) {
    case contentPos:
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
        break;
    case upperLeftPos:
        removedChildren.append(index);
        formula()->elementRemoval(index);
        index = 0;
        cursor->setTo(this, upperLeftPos);
        formula()->changed();
        break;
    }
}


/**
 * Moves the cursor to a normal place where new elements
 * might be inserted.
 */
void RootElement::normalize(FormulaCursor* cursor, Direction direction)
{
    if (direction == beforeCursor) {
        content->moveLeft(cursor, this);
    }
    else {
        content->moveRight(cursor, this);
    }
}


// main child
//
// If an element has children one has to become the main one.

SequenceElement* RootElement::getMainChild()
{
    return content;
}

// void RootElement::setMainChild(SequenceElement* child)
// {
//     formula()->elementRemoval(content);
//     content = child;
//     content->setParent(this);
//     formula()->changed();
// }


/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void RootElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    if (child == content) {
        cursor->setTo(this, contentPos);
    }
    else if (child == index) {
        cursor->setTo(this, upperLeftPos);
    }
}


void RootElement::moveToIndex(FormulaCursor* cursor, Direction direction)
{
    if (hasIndex()) {
        if (direction == beforeCursor) {
            index->moveLeft(cursor, this);
        }
        else {
            index->moveRight(cursor, this);
        }
    }
}

void RootElement::setToIndex(FormulaCursor* cursor)
{
    cursor->setTo(this, upperLeftPos);
}


/**
 * Appends our attributes to the dom element.
 */
void RootElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    QDomDocument doc = element.ownerDocument();

    QDomElement con = doc.createElement("CONTENT");
    con.appendChild(content->getElementDom(doc));
    element.appendChild(con);

    if(hasIndex()) {
        QDomElement ind = doc.createElement("ROOTINDEX");
        ind.appendChild(index->getElementDom(doc));
        element.appendChild(ind);
    }
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool RootElement::readAttributesFromDom(QDomElement element)
{
    return BasicElement::readAttributesFromDom(element);
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool RootElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    if ( !buildChild( content, node, "CONTENT" ) ) {
        kWarning( DEBUGID ) << "Empty content in RootElement." << endl;
        return false;
    }
    node = node.nextSibling();

    if ( node.nodeName().toUpper() == "ROOTINDEX" ) {
        if ( !buildChild( index=new SequenceElement( this ), node, "ROOTINDEX" ) ) {
            return false;
        }
    }
    // backward compatibility
    else if ( node.nodeName().toUpper() == "INDEX" ) {
        if ( !buildChild( index=new SequenceElement( this ), node, "INDEX" ) ) {
            return false;
        }
    }
    node = node.nextSibling();

    return true;
}
/*
QString RootElement::toLatex()
{
    QString root;
    root="\\sqrt";
    if(hasIndex()) {
        root+="[";
	root+=index->toLatex();
	root+="]";
    }
    root+="{";
    root+=content->toLatex();
    root+="}";

    return root;
}

QString RootElement::formulaString()
{
    if ( hasIndex() ) {
        return "(" + content->formulaString() + ")**(1.0/(" + index->formulaString() + "))";
    }
    return "sqrt(" + content->formulaString() + ")";
}
*/
void RootElement::writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat )
{
    QDomElement de;

    if( hasIndex() )
        de = doc.createElement( oasisFormat ? "math:mroot" : "mroot" );
    else
        de = doc.createElement( oasisFormat ? "math:msqrt" : "msqrt" );

    content->writeMathML( doc, de, oasisFormat );

    if( hasIndex() )
    {
        index->writeMathML( doc, de, oasisFormat );
    }

    parent.appendChild( de );
}

KFORMULA_NAMESPACE_END
