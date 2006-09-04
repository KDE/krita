/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "BracketElement.h"
#include <KoXmlWriter.h>

#include <QPainter>
#include <QPen>

#include <kdebug.h>
#include <klocale.h>

#include "fontstyle.h"
#include "FormulaCursor.h"
#include "FormulaElement.h"
#include "SequenceElement.h"

namespace KFormula {

BracketElement::BracketElement( BasicElement* parent ) : BasicElement( parent ),
                                                         left( 0 ),
							 right( 0 ),
							 leftType( EmptyBracket ),
							 rightType( EmptyBracket )
{
}


BracketElement::~BracketElement()
{
    delete left;
    delete right;
}


const QList<BasicElement*> BracketElement::childElements()
{
    return QList<BasicElement*>();
}

void BracketElement::readMathML( const QDomElement& element )
{
}

void BracketElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    writer->startElement( oasisFormat ? "math:mfenced" : "mfenced" );
    writeMathMLAttributes( writer );

    // save child elements that will come
    
    writer->endElement();
}

/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void BracketElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
/*    SequenceElement* content = getContent();
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
    right  ->setX(left->getWidth()+content->getWidth());*/
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
/*    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
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
*/
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
/*    SingleContentElement::writeDom(element);
    element.setAttribute("LEFT", leftType);
    element.setAttribute("RIGHT", rightType);*/
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



} // namespace KFormula
