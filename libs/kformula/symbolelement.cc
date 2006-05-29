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

#include "elementvisitor.h"
#include "formulacursor.h"
#include "formulaelement.h"
#include "kformulacommand.h"
#include "sequenceelement.h"
#include "symbolelement.h"

KFORMULA_NAMESPACE_BEGIN


class SymbolSequenceElement : public SequenceElement {
    typedef SequenceElement inherited;
public:

    SymbolSequenceElement( BasicElement* parent = 0 ) : SequenceElement( parent ) {}

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


KCommand* SymbolSequenceElement::buildCommand( Container* container, Request* request )
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
        if ( ( ir->index() == upperMiddlePos ) || ( ir->index() == lowerMiddlePos ) ) {
            SymbolElement* element = static_cast<SymbolElement*>( getParent() );
            ElementIndexPtr index = element->getIndex( ir->index() );
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


SymbolElement::SymbolElement(SymbolType type, BasicElement* parent)
    : BasicElement(parent), symbol( 0 ), symbolType( type )
{
    content = new SymbolSequenceElement( this );
    upper = 0;
    lower = 0;
}

SymbolElement::~SymbolElement()
{
    delete lower;
    delete upper;
    delete content;
    delete symbol;
}


SymbolElement::SymbolElement( const SymbolElement& other )
    : BasicElement( other ), symbol( 0 ), symbolType( other.symbolType )
{
    content = new SymbolSequenceElement( *dynamic_cast<SymbolSequenceElement*>( other.content ) );
    content->setParent( this );

    if ( other.upper ) {
        upper = new SequenceElement( *( other.upper ) );
        upper->setParent( this );
    }
    else {
        upper = 0;
    }
    if ( other.lower ) {
        lower = new SequenceElement( *( other.lower ) );
        lower->setParent( this );
    }
    else {
        lower = 0;
    }
}


bool SymbolElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


BasicElement* SymbolElement::goToPos( FormulaCursor* cursor, bool& handled,
                                      const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());

        e = content->goToPos(cursor, handled, point, myPos);
        if (e != 0) {
            return e;
        }
        if (hasLower()) {
            e = lower->goToPos(cursor, handled, point, myPos);
            if (e != 0) {
                return e;
            }
        }
        if (hasUpper()) {
            e = upper->goToPos(cursor, handled, point, myPos);
            if (e != 0) {
                return e;
            }
        }

        // the positions after the indexes.
        luPixel dx = point.x() - myPos.x();
        luPixel dy = point.y() - myPos.y();
        if (dy < symbol->getY()) {
            if (hasUpper() && (dx > upper->getX())) {
                upper->moveLeft(cursor, this);
                handled = true;
                return upper;
            }
        }
        else if (dy > symbol->getY()+symbol->getHeight()) {
            if (hasLower() && (dx > lower->getX())) {
                lower->moveLeft(cursor, this);
                handled = true;
                return lower;
            }
        }

        // Have the cursor jump behind the integral.
        if ( ( dx < symbol->getX()+symbol->getWidth() ) &&
             ( dx > symbol->getX()+symbol->getWidth()/2 ) ) {
            content->moveRight( cursor, this );
            handled = true;
            return content;
        }

        return this;
    }
    return 0;
}


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void SymbolElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle )
{
    luPt mySize = style.getAdjustedSize( tstyle );
    luPixel distX = style.ptToPixelX( style.getThinSpace( tstyle ) );
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );

    //if ( symbol == 0 ) {
    delete symbol;
    symbol = style.fontStyle().createArtwork( symbolType );
    //}

    symbol->calcSizes(style, tstyle, mySize);
    content->calcSizes(style, tstyle, istyle);

    //symbol->scale(((double)parentSize)/symbol->getHeight()*2);

    luPixel upperWidth = 0;
    luPixel upperHeight = 0;
    if (hasUpper()) {
        upper->calcSizes(style, style.convertTextStyleIndex( tstyle ),
			 style.convertIndexStyleUpper( istyle ) );
        upperWidth = upper->getWidth();
        upperHeight = upper->getHeight() + distY;
    }

    luPixel lowerWidth = 0;
    luPixel lowerHeight = 0;
    if (hasLower()) {
        lower->calcSizes(style, style.convertTextStyleIndex( tstyle ),
			 style.convertIndexStyleLower( istyle ) );
        lowerWidth = lower->getWidth();
        lowerHeight = lower->getHeight() + distY;
    }

    // widths
    luPixel xOffset = qMax(symbol->getWidth(), qMax(upperWidth, lowerWidth));
    if (style.getCenterSymbol()) {
        symbol->setX((xOffset - symbol->getWidth()) / 2);
    }
    else {
        symbol->setX(xOffset - symbol->getWidth());
    }
    // ???
    content->setX(xOffset +
                  static_cast<luPixel>( symbol->slant()*symbol->getHeight()/2 ) +
                  distX/2);

    setWidth(qMax(content->getX() + content->getWidth(),
                  qMax(upperWidth, lowerWidth)));

    // heights
    //int toMidline = qMax(content->getHeight() / 2,
    luPixel toMidline = qMax(content->axis( style, tstyle ),
                             upperHeight + symbol->getHeight()/2);
    //int fromMidline = qMax(content->getHeight() / 2,
    luPixel fromMidline = qMax(content->getHeight() - content->axis( style, tstyle ),
                               lowerHeight + symbol->getHeight()/2);
    setHeight(toMidline + fromMidline);
    //setMidline(toMidline);

    symbol->setY(toMidline - symbol->getHeight()/2);
    //content->setY(toMidline - content->getHeight()/2);
    content->setY(toMidline - content->axis( style, tstyle ));

    if (hasUpper()) {
        luPixel slant =
            static_cast<luPixel>( symbol->slant()*( symbol->getHeight()+distY ) );
        if (style.getCenterSymbol()) {
            upper->setX((xOffset - upperWidth) / 2 + slant );
        }
        else {
            if (upperWidth < symbol->getWidth()) {
                upper->setX(symbol->getX() +
                            (symbol->getWidth() - upperWidth) / 2 + slant );
            }
            else {
                upper->setX(xOffset - upperWidth);
            }
        }
        upper->setY(toMidline - upperHeight - symbol->getHeight()/2);
    }
    if (hasLower()) {
        luPixel slant = static_cast<luPixel>( -symbol->slant()*distY );
        if (style.getCenterSymbol()) {
            lower->setX((xOffset - lowerWidth) / 2 + slant);
        }
        else {
            if (lowerWidth < symbol->getWidth()) {
                lower->setX(symbol->getX() +
                            (symbol->getWidth() - lowerWidth) / 2 + slant );
            }
            else {
                lower->setX(xOffset - lowerWidth);
            }
        }
        lower->setY(toMidline + symbol->getHeight()/2 + distY);
    }
    setBaseline(content->getBaseline() + content->getY());
}

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void SymbolElement::draw( QPainter& painter, const LuPixelRect& r,
                          const ContextStyle& style,
                          ContextStyle::TextStyle tstyle,
                          ContextStyle::IndexStyle istyle,
                          const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    luPt mySize = style.getAdjustedSize( tstyle );
    symbol->draw( painter, r, style, tstyle, mySize, myPos );
    content->draw( painter, r, style, tstyle, istyle, myPos );
    if ( hasUpper() ) {
        upper->draw( painter, r, style, style.convertTextStyleIndex( tstyle ),
                     style.convertIndexStyleUpper( istyle ), myPos );
    }
    if ( hasLower() ) {
        lower->draw( painter, r, style, style.convertTextStyleIndex( tstyle ),
                     style.convertIndexStyleLower( istyle ), myPos );
    }

    // Debug
#if 0
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::red);
//     painter.drawRect( style.layoutUnitToPixelX( myPos.x() ),
//                       style.layoutUnitToPixelY( myPos.y() ),
//                       style.layoutUnitToPixelX( getWidth() ),
//                       style.layoutUnitToPixelY( getHeight() ) );
    painter.drawRect( style.layoutUnitToPixelX( myPos.x()+symbol->getX() ),
                      style.layoutUnitToPixelY( myPos.y()+symbol->getY() ),
                      style.layoutUnitToPixelX( symbol->getWidth() ),
                      style.layoutUnitToPixelY( symbol->getHeight() ) );
    painter.setPen(Qt::green);
    painter.drawLine( style.layoutUnitToPixelX( myPos.x() ),
                      style.layoutUnitToPixelY( myPos.y()+axis(style, tstyle) ),
                      style.layoutUnitToPixelX( myPos.x()+getWidth() ),
                      style.layoutUnitToPixelY( myPos.y()+axis(style, tstyle) ) );
#endif
}


void SymbolElement::dispatchFontCommand( FontCommand* cmd )
{
    content->dispatchFontCommand( cmd );
    if ( hasUpper() ) {
        upper->dispatchFontCommand( cmd );
    }
    if ( hasLower() ) {
        lower->dispatchFontCommand( cmd );
    }
}

// navigation
//
// The elements are responsible to handle cursor movement themselves.
// To do this they need to know the direction the cursor moves and
// the element it comes from.
//
// The cursor might be in normal or in selection mode.

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void SymbolElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
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
            if (linear && hasLower()) {
                lower->moveLeft(cursor, this);
            }
            else if (linear && hasUpper()) {
                upper->moveLeft(cursor, this);
            }
            else {
                getParent()->moveLeft(cursor, this);
            }
        }
        else if (from == lower) {
            if (linear && hasUpper()) {
                upper->moveLeft(cursor, this);
            }
            else {
                getParent()->moveLeft(cursor, this);
            }
        }
        else if (from == upper) {
            getParent()->moveLeft(cursor, this);
        }
    }
}

/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void SymbolElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            if (linear && hasUpper()) {
                upper->moveRight(cursor, this);
            }
            else if (linear && hasLower()) {
                lower->moveRight(cursor, this);
            }
            else {
                content->moveRight(cursor, this);
            }
        }
        else if (from == upper) {
            if (linear && hasLower()) {
                lower->moveRight(cursor, this);
            }
            else {
                content->moveRight(cursor, this);
            }
        }
        else if (from == lower) {
            content->moveRight(cursor, this);
        }
        else if (from == content) {
            getParent()->moveRight(cursor, this);
        }
    }
}

/**
 * Enters this element while moving up starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or above it.
 */
void SymbolElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveUp(cursor, this);
    }
    else {
        if (from == content) {
            if (hasUpper()) {
                upper->moveLeft(cursor, this);
            }
            else {
                getParent()->moveUp(cursor, this);
            }
        }
        else if (from == upper) {
            getParent()->moveUp(cursor, this);
        }
        else if ((from == getParent()) || (from == lower)) {
            content->moveRight(cursor, this);
        }
    }
}

/**
 * Enters this element while moving down starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or below it.
 */
void SymbolElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveDown(cursor, this);
    }
    else {
        if (from == content) {
            if (hasLower()) {
                lower->moveLeft(cursor, this);
            }
            else {
                getParent()->moveDown(cursor, this);
            }
        }
        else if (from == lower) {
            getParent()->moveDown(cursor, this);
        }
        else if ((from == getParent()) || (from == upper)) {
            content->moveRight(cursor, this);
        }
    }
}

// children

// main child
//
// If an element has children one has to become the main one.

// void SymbolElement::setMainChild(SequenceElement* child)
// {
//     formula()->elementRemoval(content);
//     content = child;
//     content->setParent(this);
//     formula()->changed();
// }


/**
 * Inserts all new children at the cursor position. Places the
 * cursor according to the direction.
 *
 * You only can insert one index at a time. So the list must contain
 * exactly on SequenceElement. And the index you want to insert
 * must not exist already.
 *
 * The list will be emptied but stays the property of the caller.
 */
void SymbolElement::insert(FormulaCursor* cursor,
                           QList<BasicElement*>& newChildren,
                           Direction direction)
{
    SequenceElement* index = static_cast<SequenceElement*>(newChildren.takeAt(0));
    index->setParent(this);

    switch (cursor->getPos()) {
    case upperMiddlePos:
        upper = index;
        break;
    case lowerMiddlePos:
        lower = index;
        break;
    default:
        // this is an error!
        return;
    }

    if (direction == beforeCursor) {
        index->moveLeft(cursor, this);
    }
    else {
        index->moveRight(cursor, this);
    }
    cursor->setSelection(false);
    formula()->changed();
}

/**
 * Removes all selected children and returns them. Places the
 * cursor to where the children have been.
 *
 * The cursor has to be inside one of our indexes which is supposed
 * to be empty. The index will be removed and the cursor will
 * be placed to the removed index so it can be inserted again.
 * This methode is called by SequenceElement::remove only.
 *
 * The ownership of the list is passed to the caller.
 */
void SymbolElement::remove(FormulaCursor* cursor,
                           QList<BasicElement*>& removedChildren,
                           Direction direction)
{
    int pos = cursor->getPos();
    switch (pos) {
    case upperMiddlePos:
        removedChildren.append(upper);
        formula()->elementRemoval(upper);
        upper = 0;
        setToUpper(cursor);
        break;
    case lowerMiddlePos:
        removedChildren.append(lower);
        formula()->elementRemoval(lower);
        lower = 0;
        setToLower(cursor);
        break;
    case contentPos: {
        BasicElement* parent = getParent();
        parent->selectChild(cursor, this);
        parent->remove(cursor, removedChildren, direction);
        break;
    }
    }
    formula()->changed();
}

/**
 * Moves the cursor to a normal place where new elements
 * might be inserted.
 */
void SymbolElement::normalize(FormulaCursor* cursor, Direction direction)
{
    if (direction == beforeCursor) {
        content->moveLeft(cursor, this);
    }
    else {
        content->moveRight(cursor, this);
    }
}

/**
 * Returns the child at the cursor.
 */
BasicElement* SymbolElement::getChild(FormulaCursor* cursor, Direction)
{
    int pos = cursor->getPos();
    switch (pos) {
    case contentPos:
        return content;
    case upperMiddlePos:
        return upper;
    case lowerMiddlePos:
        return lower;
    }
    return 0;
}

/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void SymbolElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    if (child == content) {
        setToContent(cursor);
    }
    else if (child == upper) {
        setToUpper(cursor);
    }
    else if (child == lower) {
        setToLower(cursor);
    }
}

void SymbolElement::setToUpper(FormulaCursor* cursor)
{
    cursor->setTo(this, upperMiddlePos);
}

void SymbolElement::setToLower(FormulaCursor* cursor)
{
    cursor->setTo(this, lowerMiddlePos);
}

/**
 * Sets the cursor to point to the place where the content is.
 * There always is a content so this is not a useful place.
 * No insertion or removal will succeed as long as the cursor is
 * there.
 */
void SymbolElement::setToContent(FormulaCursor* cursor)
{
    cursor->setTo(this, contentPos);
}


void SymbolElement::moveToUpper(FormulaCursor* cursor, Direction direction)
{
    if (hasUpper()) {
        if (direction == beforeCursor) {
            upper->moveLeft(cursor, this);
        }
        else {
            upper->moveRight(cursor, this);
        }
    }
}

void SymbolElement::moveToLower(FormulaCursor* cursor, Direction direction)
{
    if (hasLower()) {
        if (direction == beforeCursor) {
            lower->moveLeft(cursor, this);
        }
        else {
            lower->moveRight(cursor, this);
        }
    }
}


ElementIndexPtr SymbolElement::getIndex( int position )
{
    switch ( position ) {
	case lowerMiddlePos:
	    return getLowerIndex();
	case upperMiddlePos:
	    return getUpperIndex();
    }
    return getUpperIndex();
}


/**
 * Appends our attributes to the dom element.
 */
void SymbolElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    element.setAttribute("TYPE", symbolType);

    QDomDocument doc = element.ownerDocument();

    QDomElement con = doc.createElement("CONTENT");
    con.appendChild(content->getElementDom(doc));
    element.appendChild(con);

    if(hasLower()) {
        QDomElement ind = doc.createElement("LOWER");
        ind.appendChild(lower->getElementDom(doc));
        element.appendChild(ind);
    }
    if(hasUpper()) {
        QDomElement ind = doc.createElement("UPPER");
        ind.appendChild(upper->getElementDom(doc));
        element.appendChild(ind);
    }
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool SymbolElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }

    QString typeStr = element.attribute("TYPE");
    if(!typeStr.isNull()) {
        symbolType = static_cast<SymbolType>(typeStr.toInt());
    }

    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool SymbolElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    if ( !buildChild( content, node, "CONTENT" ) ) {
        kWarning( DEBUGID ) << "Empty content in SymbolElement." << endl;
        return false;
    }
    node = node.nextSibling();

    bool lowerRead = false;
    bool upperRead = false;

    while (!node.isNull() && !(upperRead && lowerRead)) {

        if (!lowerRead && (node.nodeName().toUpper() == "LOWER")) {
            lowerRead = buildChild( lower=new SequenceElement( this ), node, "LOWER" );
            if ( !lowerRead ) return false;
        }

        if (!upperRead && (node.nodeName().toUpper() == "UPPER")) {
            upperRead = buildChild( upper=new SequenceElement( this ), node, "UPPER" );
            if ( !upperRead ) return false;
        }

        node = node.nextSibling();
    }
    return true;
}

QString SymbolElement::toLatex()
{
    QString sym;

    switch(symbolType) {

	case 1001:
	 sym="\\int";
	break;
	case 1002:
	 sym="\\sum";
	break;
	case 1003:
	 sym="\\prod";
	break;

	default:
	 sym=" ";

    }


    if(hasLower()) {
        sym+="_{";
	sym+=lower->toLatex();
	sym+="}";
    }

    if(hasUpper()) {
        sym+="^{";
	sym+=upper->toLatex();
	sym+="} ";
    }

    sym += " ";

    sym+=content->toLatex();


    return sym;
}

QString SymbolElement::formulaString()
{
    QString sym;
    switch ( symbolType ) {
    case 1001:
        sym="int(";
	break;
    case 1002:
        sym="sum(";
	break;
    case 1003:
        sym="prod(";
	break;
    default:
        sym="(";
    }
    sym += content->formulaString();
    if ( hasLower() ) {
        sym += ", " + lower->formulaString();
    }
    if ( hasUpper() ) {
        sym += ", " + upper->formulaString();
    }
    return sym + ")";
}

void SymbolElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat  )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mrow" : "mrow" );
    QDomElement mo = doc.createElement( oasisFormat ? "math:mo" : "mo" );

    QString value;

    switch( symbolType )
    {
    case EmptyBracket: break;
    case LeftLineBracket: case RightLineBracket:
        mo.appendChild( doc.createTextNode( "|" ) ); break;
    case Integral:
        mo.appendChild( doc.createEntityReference( "int" ) ); break;
    case Sum:
        mo.appendChild( doc.createEntityReference( "sum" ) ); break;
    case Product:
        mo.appendChild( doc.createEntityReference( "prod" ) ); break;
    default:
        mo.appendChild( doc.createTextNode( QChar( symbolType ) ) );
    }

    QDomElement between;
    if ( hasUpper() && hasLower() )
    {
        between = doc.createElement( oasisFormat ? "math:msubsup" : "msubsup" );
        between.appendChild( mo );
        lower->writeMathML( doc, between, oasisFormat );
        upper->writeMathML( doc, between, oasisFormat );
    }
    else if ( hasUpper() )
    {
        between = doc.createElement( oasisFormat ? "math:msup" : "msup" );
        between.appendChild( mo );
        upper->writeMathML( doc, between, oasisFormat );
    }
    else if ( hasLower() )
    {
        between = doc.createElement( oasisFormat ? "math:msub" : "msub" );
        between.appendChild( mo );
        lower->writeMathML( doc, between, oasisFormat );
    }
    else
        between = mo;

    de.appendChild( between );
    content->writeMathML( doc, de, oasisFormat );

    parent.appendChild( de );
}

KFORMULA_NAMESPACE_END
