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

#include <q3ptrlist.h>
#include <QPainter>
#include <QPen>
#include <q3pointarray.h>

#include <kdebug.h>
#include <klocale.h>

#include "bracketelement.h"
#include "elementvisitor.h"
#include "fontstyle.h"
#include "formulacursor.h"
#include "formulaelement.h"
#include "sequenceelement.h"

KFORMULA_NAMESPACE_BEGIN

SingleContentElement::SingleContentElement(BasicElement* parent )
    : BasicElement( parent )
{
    content = new SequenceElement( this );
}


SingleContentElement::SingleContentElement( const SingleContentElement& other )
    : BasicElement( other )
{
    content = new SequenceElement( other.content );
    content->setParent( this );
}


SingleContentElement::~SingleContentElement()
{
    delete content;
}


QChar SingleContentElement::getCharacter() const
{
    // This is meant to make the SingleContentElement text only.
    // This "fixes" the parenthesis problem (parenthesis too large).
    // I'm not sure if we really want this. There should be better ways.
    if ( content->isTextOnly() ) {
        return '\\';
    }
    return content->getCharacter();
}

BasicElement* SingleContentElement::goToPos( FormulaCursor* cursor, bool& handled,
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
        return this;
    }
    return 0;
}

void SingleContentElement::dispatchFontCommand( FontCommand* cmd )
{
    content->dispatchFontCommand( cmd );
}

void SingleContentElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        //bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            content->moveLeft(cursor, this);
        }
        else {
            getParent()->moveLeft(cursor, this);
        }
    }
}

void SingleContentElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        //bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            content->moveRight(cursor, this);
        }
        else {
            getParent()->moveRight(cursor, this);
        }
    }
}

void SingleContentElement::moveUp(FormulaCursor* cursor, BasicElement* /*from*/)
{
    getParent()->moveUp(cursor, this);
}

void SingleContentElement::moveDown(FormulaCursor* cursor, BasicElement* /*from*/)
{
    getParent()->moveDown(cursor, this);
}

void SingleContentElement::remove( FormulaCursor* cursor,
                                   Q3PtrList<BasicElement>& removedChildren,
                                   Direction direction )
{
    switch (cursor->getPos()) {
    case contentPos:
        BasicElement* parent = getParent();
        parent->selectChild(cursor, this);
        parent->remove(cursor, removedChildren, direction);
    }
}

void SingleContentElement::normalize( FormulaCursor* cursor, Direction direction )
{
    if (direction == beforeCursor) {
        content->moveLeft(cursor, this);
    }
    else {
        content->moveRight(cursor, this);
    }
}

SequenceElement* SingleContentElement::getMainChild()
{
    return content;
}

void SingleContentElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    if (child == content) {
        cursor->setTo(this, contentPos);
    }
}

void SingleContentElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    QDomDocument doc = element.ownerDocument();

    QDomElement con = doc.createElement("CONTENT");
    con.appendChild(content->getElementDom(doc));
    element.appendChild(con);
}

bool SingleContentElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    if ( !buildChild( content, node, "CONTENT" ) ) {
        kWarning( DEBUGID ) << "Empty content in " << getTagName() << endl;
        return false;
    }
    node = node.nextSibling();

    return true;
}

void SingleContentElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    content->writeMathML( doc, parent, oasisFormat );
}



BracketElement::BracketElement(SymbolType l, SymbolType r, BasicElement* parent)
    : SingleContentElement(parent),
      left( 0 ), right( 0 ),
      leftType( l ), rightType( r )
{
}


BracketElement::~BracketElement()
{
    delete left;
    delete right;
}


BracketElement::BracketElement( const BracketElement& other )
    : SingleContentElement( other ),
      left( 0 ), right( 0 ),
      leftType( other.leftType ), rightType( other.rightType )
{
}


bool BracketElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void BracketElement::entered( SequenceElement* /*child*/ )
{
    formula()->tell( i18n( "Delimited list" ) );
}


BasicElement* BracketElement::goToPos( FormulaCursor* cursor, bool& handled,
                                       const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());
        e = getContent()->goToPos(cursor, handled, point, myPos);
        if (e != 0) {
            return e;
        }

        // We are in one of those gaps.
        luPixel dx = point.x() - myPos.x();
        luPixel dy = point.y() - myPos.y();

        if ((dx > getContent()->getX()+getContent()->getWidth()) ||
            (dy > getContent()->getY()+getContent()->getHeight())) {
            getContent()->moveEnd(cursor);
            handled = true;
            return getContent();
        }
        return this;
    }
    return 0;
}


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void BracketElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    SequenceElement* content = getContent();
    content->calcSizes(style, tstyle, istyle);

    //if ( left == 0 ) {
    delete left;
    delete right;
    left = style.fontStyle().createArtwork( leftType );
    right = style.fontStyle().createArtwork( rightType );
    //}

    if (content->isTextOnly()) {
        left->calcSizes(style, tstyle);
        right->calcSizes(style, tstyle);

        setBaseline(qMax(content->getBaseline(),
                         qMax(left->getBaseline(), right->getBaseline())));

        content->setY(getBaseline() - content->getBaseline());
        left   ->setY(getBaseline() - left   ->getBaseline());
        right  ->setY(getBaseline() - right  ->getBaseline());

        //setMidline(content->getY() + content->getMidline());
        setHeight(qMax(content->getY() + content->getHeight(),
                       qMax(left ->getY() + left ->getHeight(),
                            right->getY() + right->getHeight())));
    }
    else {
        //kDebug( DEBUGID ) << "BracketElement::calcSizes " << content->axis( style, tstyle ) << " " << content->getHeight() << endl;
        luPixel contentHeight = 2 * qMax( content->axis( style, tstyle ),
                                          content->getHeight() - content->axis( style, tstyle ) );
        left->calcSizes( style, tstyle, contentHeight );
        right->calcSizes( style, tstyle, contentHeight );

        // height
        setHeight(qMax(contentHeight,
                       qMax(left->getHeight(), right->getHeight())));
        //setMidline(getHeight() / 2);

        content->setY(getHeight() / 2 - content->axis( style, tstyle ));
        setBaseline(content->getBaseline() + content->getY());

        if ( left->isNormalChar() ) {
            left->setY(getBaseline() - left->getBaseline());
        }
        else {
            left->setY((getHeight() - left->getHeight())/2);
        }
        if ( right->isNormalChar() ) {
            right->setY(getBaseline() - right->getBaseline());
        }
        else {
            right->setY((getHeight() - right->getHeight())/2);
        }

//         kDebug() << "BracketElement::calcSizes" << endl
//                   << "getHeight(): " << getHeight() << endl
//                   << "left->getHeight():  " << left->getHeight() << endl
//                   << "right->getHeight(): " << right->getHeight() << endl
//                   << "left->getY():  " << left->getY() << endl
//                   << "right->getY(): " << right->getY() << endl
//                   << endl;
    }

    // width
    setWidth(left->getWidth() + content->getWidth() + right->getWidth());
    content->setX(left->getWidth());
    right  ->setX(left->getWidth()+content->getWidth());
}


/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void BracketElement::draw( QPainter& painter, const LuPixelRect& r,
                           const ContextStyle& style,
                           ContextStyle::TextStyle tstyle,
                           ContextStyle::IndexStyle istyle,
                           const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    SequenceElement* content = getContent();
    content->draw(painter, r, style, tstyle, istyle, myPos);

    if (content->isTextOnly()) {
        left->draw(painter, r, style, tstyle, myPos);
        right->draw(painter, r, style, tstyle, myPos);
    }
    else {
        luPixel contentHeight = 2 * qMax(content->axis( style, tstyle ),
                                         content->getHeight() - content->axis( style, tstyle ));
        left->draw(painter, r, style, tstyle, contentHeight, myPos);
        right->draw(painter, r, style, tstyle, contentHeight, myPos);
    }

    // Debug
#if 0
    painter.setBrush( Qt::NoBrush );
    painter.setPen( Qt::red );
    painter.drawRect( style.layoutUnitToPixelX( myPos.x()+left->getX() ),
                      style.layoutUnitToPixelY( myPos.y()+left->getY() ),
                      style.layoutUnitToPixelX( left->getWidth() ),
                      style.layoutUnitToPixelY( left->getHeight() ) );
    painter.drawRect( style.layoutUnitToPixelX( myPos.x()+right->getX() ),
                      style.layoutUnitToPixelY( myPos.y()+right->getY() ),
                      style.layoutUnitToPixelX( right->getWidth() ),
                      style.layoutUnitToPixelY( right->getHeight() ) );
#endif
}


/**
 * Appends our attributes to the dom element.
 */
void BracketElement::writeDom(QDomElement element)
{
    SingleContentElement::writeDom(element);
    element.setAttribute("LEFT", leftType);
    element.setAttribute("RIGHT", rightType);
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool BracketElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    QString leftStr = element.attribute("LEFT");
    if(!leftStr.isNull()) {
        leftType = static_cast<SymbolType>(leftStr.toInt());
    }
    QString rightStr = element.attribute("RIGHT");
    if(!rightStr.isNull()) {
        rightType = static_cast<SymbolType>(rightStr.toInt());
    }
    return true;
}

QString BracketElement::toLatex()
{
    QString ls,rs,cs;
    cs=getContent()->toLatex();
    ls="\\left"+latexString(leftType) + " ";
    rs=" \\right"+latexString(rightType);

    return ls+cs+rs;
}

QString BracketElement::latexString(char type)
{
    switch (type) {
	case ']':
	    return "]";
	case '[':
	    return "[";
	case '{':
	    return "\\{";
	case '}':
	    return "\\}";
	case '(':
	    return "(";
	case ')':
	    return ")";
	case '|':
	    return "|";
        case '<':
            return "\\langle";
        case '>':
            return "\\rangle";
        case '/':
            return "/";
        case '\\':
            return "\\backslash";
    }
    return ".";
}

QString BracketElement::formulaString()
{
    return "(" + getContent()->formulaString() + ")";
}

void BracketElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mfenced" : "mfenced" );
    if ( left->getType() != LeftRoundBracket ||
         right->getType() != RightRoundBracket )
    {
        de.setAttribute( "open",  QString( QChar( leftType ) ) );
        de.setAttribute( "close", QString( QChar( rightType ) ) );
    }
    SingleContentElement::writeMathML( doc, de, oasisFormat );
    parent.appendChild( de );
}


OverlineElement::OverlineElement( BasicElement* parent )
    : SingleContentElement( parent )
{
}

OverlineElement::~OverlineElement()
{
}

OverlineElement::OverlineElement( const OverlineElement& other )
    : SingleContentElement( other )
{
}


bool OverlineElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void OverlineElement::entered( SequenceElement* /*child*/ )
{
    formula()->tell( i18n( "Overline" ) );
}


void OverlineElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    SequenceElement* content = getContent();
    content->calcSizes(style, tstyle,
		       style.convertIndexStyleLower(istyle));

    //luPixel distX = style.ptToPixelX( style.getThinSpace( tstyle ) );
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );
    //luPixel unit = (content->getHeight() + distY)/ 3;

    setWidth( content->getWidth() );
    setHeight( content->getHeight() + distY );

    content->setX( 0 );
    content->setY( distY );
    setBaseline(content->getBaseline() + content->getY());
}

void OverlineElement::draw( QPainter& painter, const LuPixelRect& r,
                            const ContextStyle& style,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    SequenceElement* content = getContent();
    content->draw( painter, r, style, tstyle,
                   style.convertIndexStyleLower( istyle ), myPos );

    luPixel x = myPos.x();
    luPixel y = myPos.y();
    //int distX = style.getDistanceX(tstyle);
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );
    //luPixel unit = (content->getHeight() + distY)/ 3;

    painter.setPen( QPen( style.getDefaultColor(),
                          style.layoutUnitToPixelY( style.getLineWidth() ) ) );

    painter.drawLine( style.layoutUnitToPixelX( x ),
                      style.layoutUnitToPixelY( y+distY/3 ),
                      style.layoutUnitToPixelX( x+content->getWidth() ),
                      style.layoutUnitToPixelY( y+distY/3 ) );
}


QString OverlineElement::toLatex()
{
    return "\\overline{" + getContent()->toLatex() + "}";
}

QString OverlineElement::formulaString()
{
    return getContent()->formulaString();
}

void OverlineElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mover" : "mover" );
    SingleContentElement::writeMathML( doc, de, oasisFormat );
    QDomElement op = doc.createElement( oasisFormat ? "math:mo" : "mo" );
    // is this the right entity? Mozilla renders it correctly.
    op.appendChild( doc.createEntityReference( "OverBar" ) );
    de.appendChild( op );
    parent.appendChild( de );
}


UnderlineElement::UnderlineElement( BasicElement* parent )
    : SingleContentElement( parent )
{
}

UnderlineElement::~UnderlineElement()
{
}


UnderlineElement::UnderlineElement( const UnderlineElement& other )
    : SingleContentElement( other )
{
}


bool UnderlineElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void UnderlineElement::entered( SequenceElement* /*child*/ )
{
    formula()->tell( i18n( "Underline" ) );
}


void UnderlineElement::calcSizes(const ContextStyle& style,
                                 ContextStyle::TextStyle tstyle,
                                 ContextStyle::IndexStyle istyle)
{
    SequenceElement* content = getContent();
    content->calcSizes(style, tstyle,
		       style.convertIndexStyleLower(istyle));

    //luPixel distX = style.ptToPixelX( style.getThinSpace( tstyle ) );
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );
    //luPixel unit = (content->getHeight() + distY)/ 3;

    setWidth( content->getWidth() );
    setHeight( content->getHeight() + distY );

    content->setX( 0 );
    content->setY( 0 );
    setBaseline(content->getBaseline() + content->getY());
}

void UnderlineElement::draw( QPainter& painter, const LuPixelRect& r,
                             const ContextStyle& style,
                             ContextStyle::TextStyle tstyle,
                             ContextStyle::IndexStyle istyle,
                             const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    SequenceElement* content = getContent();
    content->draw( painter, r, style, tstyle,
                   style.convertIndexStyleLower( istyle ), myPos );

    luPixel x = myPos.x();
    luPixel y = myPos.y();
    //int distX = style.getDistanceX(tstyle);
    //luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );
    //luPixel unit = (content->getHeight() + distY)/ 3;

    painter.setPen( QPen( style.getDefaultColor(),
                          style.layoutUnitToPixelY( style.getLineWidth() ) ) );

    painter.drawLine( style.layoutUnitToPixelX( x ),
                      style.layoutUnitToPixelY( y+getHeight()-style.getLineWidth() ),
                      style.layoutUnitToPixelX( x+content->getWidth() ),
                      style.layoutUnitToPixelY( y+getHeight()-style.getLineWidth() ) );
}


QString UnderlineElement::toLatex()
{
    return "\\underline{" + getContent()->toLatex() + "}";
}

QString UnderlineElement::formulaString()
{
    return getContent()->formulaString();
}

void UnderlineElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:munder" : "munder" );
    SingleContentElement::writeMathML( doc, de, oasisFormat );
    QDomElement op = doc.createElement( oasisFormat ? "math:mo" : "mo" );
    // is this the right entity? Mozilla renders it correctly.
    op.appendChild( doc.createEntityReference( "UnderBar" ) );
    de.appendChild( op );
    parent.appendChild( de );
}

KFORMULA_NAMESPACE_END
