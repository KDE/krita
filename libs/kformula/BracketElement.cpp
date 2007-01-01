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
                                                         rightType( EmptyBracket ),
                                                         m_operator( false ),
                                                         m_customLeft( false ),
                                                         m_customRight( false )
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

/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void BracketElement::calcSizes( const ContextStyle& context, 
                                ContextStyle::TextStyle tstyle, 
                                ContextStyle::IndexStyle istyle,
                                StyleAttributes& style )
{
/*
    SequenceElement* content = getContent();
    content->calcSizes( context, tstyle, istyle, style );

    //if ( left == 0 ) {
    delete left;
    delete right;
    left = context.fontStyle().createArtwork( leftType );
    right = context.fontStyle().createArtwork( rightType );
    //}

    double factor = style.sizeFactor();
    if (content->isTextOnly()) {
        left->calcSizes( context, tstyle, factor );
        right->calcSizes( context, tstyle, factor );

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
        //kDebug( DEBUGID ) << "BracketElement::calcSizes " << content->axis( context, tstyle ) << " " << content->getHeight() << endl;
        luPixel contentHeight = 2 * qMax( content->axis( context, tstyle, factor ),
                                          content->getHeight() - content->axis( context, tstyle, factor ) );
        left->calcSizes( context, tstyle, factor, contentHeight );
        right->calcSizes( context, tstyle, factor, contentHeight );

        // height
        setHeight(qMax(contentHeight,
                       qMax(left->getHeight(), right->getHeight())));
        //setMidline(getHeight() / 2);

        content->setY(getHeight() / 2 - content->axis( context, tstyle, factor ));
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
                           const ContextStyle& context,
                           ContextStyle::TextStyle tstyle,
                           ContextStyle::IndexStyle istyle,
                           StyleAttributes& style,
                           const LuPixelPoint& parentOrigin )
{
/*    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    SequenceElement* content = getContent();
    content->draw(painter, r, context, tstyle, istyle, style, myPos);

    if (content->isTextOnly()) {
        left->draw(painter, r, context, tstyle, style, myPos);
        right->draw(painter, r, context, tstyle, style, myPos);
    }
    else {
        luPixel contentHeight = 2 * qMax(content->axis( context, tstyle, factor ),
                                         content->getHeight() - content->axis( context, tstyle, factor ));
        left->draw( painter, r, context, tstyle, style, contentHeight, myPos);
        right->draw(painter, r, context, tstyle, style, contentHeight, myPos);
    }
*/
    // Debug
#if 0
    painter.setBrush( Qt::NoBrush );
    painter.setPen( Qt::red );
    painter.drawRect( context.layoutUnitToPixelX( myPos.x()+left->getX() ),
                      context.layoutUnitToPixelY( myPos.y()+left->getY() ),
                      context.layoutUnitToPixelX( left->getWidth() ),
                      context.layoutUnitToPixelY( left->getHeight() ) );
    painter.drawRect( context.layoutUnitToPixelX( myPos.x()+right->getX() ),
                      context.layoutUnitToPixelY( myPos.y()+right->getY() ),
                      context.layoutUnitToPixelX( right->getWidth() ),
                      context.layoutUnitToPixelY( right->getHeight() ) );
#endif
}

void BracketElement::readMathMLContent( const KoXmlElement& element )
{
}
    
void BracketElement::writeMathMLContent( KoXmlWriter* writer, bool oasisFormat )
{
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


/**
 * Reads our attributes from the MathML element.
 * Returns false if it failed.
 */
/*
bool BracketElement::readAttributesFromMathMLDom(const QDomElement& element)
{
    if ( !BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    if ( element.tagName().lower() == "mo" ) {
        m_operator = true;
        // TODO: parse attributes in section 3.2.5.2
    }
    else { // mfenced, see attributes in section 3.3.8.2
        leftType = LeftRoundBracket;
        rightType = RightRoundBracket;
        QString openStr = element.attribute( "open" ).stripWhiteSpace();
        if ( !openStr.isNull() ) {
            m_customLeft = true;
            if ( openStr == "[" )
                leftType = LeftSquareBracket;
            else if ( openStr == "]" )
                leftType = RightSquareBracket;
            else if ( openStr == "{" )
                leftType = LeftCurlyBracket;
            else if ( openStr == "}" )
                leftType = RightCurlyBracket;
            else if ( openStr == "<" )
                leftType = LeftCornerBracket;
            else if ( openStr == ">" )
                leftType = RightCornerBracket;
            else if ( openStr == "(" )
                leftType = LeftRoundBracket;
            else if ( openStr == ")" )
                leftType = RightRoundBracket;
            else if ( openStr == "/" )
                leftType = SlashBracket;
            else if ( openStr == "\\" )
                leftType = BackSlashBracket;
            else // TODO: Check for entity references
                leftType = LeftRoundBracket;
        }
        QString closeStr = element.attribute( "close" ).stripWhiteSpace();
        if ( !closeStr.isNull() ) {
            m_customRight = true;
            if ( closeStr == "[" )
                rightType = LeftSquareBracket;
            else if ( closeStr == "]" )
                rightType = RightSquareBracket;
            else if ( closeStr == "{" )
                rightType = LeftCurlyBracket;
            else if ( closeStr == "}" )
                rightType = RightCurlyBracket;
            else if ( closeStr == "<" )
                rightType = LeftCornerBracket;
            else if ( closeStr == ">" )
                rightType = RightCornerBracket;
            else if ( closeStr == "(" )
                rightType = LeftRoundBracket;
            else if ( closeStr == ")" )
                rightType = RightRoundBracket;
            else if ( closeStr == "/" )
                rightType = SlashBracket;
            else if ( closeStr == "\\" )
                rightType = BackSlashBracket;
            else // TODO: Check for entity references
                rightType = LeftRoundBracket;
        }
        m_separators = element.attribute( "separators" ).simplifyWhiteSpace();
    }
    return true;
}
*/

/**
 * Reads our content from the MathML node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
/*
int BracketElement::readContentFromMathMLDom(QDomNode& node)
{
    bool empty = false;
    if ( m_operator ) {
        node = node.parentNode();
        QDomNode open = node;
        QDomNode parent = node.parentNode();
        if ( ! operatorType( node, true ) )
            return -1;
        int nodeNum = searchOperator( node );
        if ( nodeNum == -1 ) // Closing bracket not found
            return -1;
        if ( nodeNum == 0 ) { // Empty content
            empty = true;
        }
        if ( nodeNum > 1 ) { // More than two elements inside, infer a mrow
            kdWarning() << "NodeNum: " << nodeNum << endl;
            QDomDocument doc = node.ownerDocument();
            QDomElement de = doc.createElement( "mrow" );
            int i = 0;
            do {
                QDomNode n = node.nextSibling();
                de.appendChild( node.toElement() );
                node = n;
            } while ( ++i < nodeNum );
            parent.insertAfter( de, open );
            node = de;
            kdWarning() << doc.toString() << endl;
        }
    }
    else {
        // if it's a mfence tag, we need to convert to equivalent expanded form.
        // See section 3.3.8
        while ( ! node.isNull() && ! node.isElement() )
            node = node.nextSibling();
        QDomNode next = node.nextSibling();
        while ( ! next.isNull() && ! next.isElement() )
            next = next.nextSibling();
        if ( ! next.isNull()) {
            QDomDocument doc = node.ownerDocument();
            QDomNode parent = node.parentNode();
            QString ns = parent.prefix();
            QDomElement de = doc.createElementNS( ns, "mrow" );
            uint pos = 0;
            while ( ! node.isNull() ) {
                QDomNode no = node.nextSibling();
                while ( ! no.isNull() && ! no.isElement() )
                    no = no.nextSibling();
                de.appendChild( node.toElement() );
                if ( ! no.isNull() && ( m_separators.isNull() || ! m_separators.isEmpty() ) ) {
                    QDomElement sep = doc.createElementNS( ns, "mo" );
                    de.appendChild( sep );
                    if ( m_separators.isNull() ) {
                        sep.appendChild( doc.createTextNode( "," ) );
                    }
                    else {
                        if ( m_separators.at( pos ).isSpace() ) {
                            pos++;
                        }
                        sep.appendChild( doc.createTextNode( QString ( m_separators.at( pos ) ) ) );
                    }
                    if ( pos < m_separators.length() - 1 ) {
                        pos++;
                    }
                }
                node = no;
            }
            parent.appendChild( de );
            node = parent.firstChild();
            while ( ! node.isElement() )
                node = node.nextSibling();
        }
    }
    if ( ! empty ) {
        inherited::readContentFromMathMLDom( node );
    }
    if ( m_operator ) {
        if ( ! operatorType( node, false ) ) {
            return -1;
        }
        if ( empty )
            return 2;
        return 3;
    }
    return 1;
}


bool BracketElement::operatorType( QDomNode& node, bool open )
{
    SymbolType* type = open ? &leftType : &rightType;
    if ( node.isElement() ) {
        QDomElement e = node.toElement();
        QString s =  e.text();
        if ( s.isNull() )
            return false;
        *type = static_cast<SymbolType>( QString::number( s.at( 0 ).latin1() ).toInt() );
        node = node.nextSibling();
    }
    else if ( node.isEntityReference() ) {
        QString name = node.nodeName();
        // TODO: To fully support these, SymbolType has to be extended,
        //       and better Unicode support is a must
        // CloseCurlyDoubleQuote 0x201D
        // CloseCurlyQoute       0x2019
        // LeftCeiling           0x2308
        // LeftDoubleBracket     0x301A
        // LeftFloor             0x230A
        // OpenCurlyDoubleQuote  0x201C
        // OpenCurlyQuote        0x2018
        // RightCeiling          0x2309
        // RightDoubleBracket    0x301B
        // RightFloor            0x230B
        if ( name == "LeftAngleBracket" ) {
            *type = LeftCornerBracket;
        }
        else if ( name == "RightAngleBracket" ) {
            *type = RightCornerBracket; 
        }
        else {
            if ( open ) {
                *type = LeftRoundBracket;
            }
            else
                *type = RightRoundBracket;
        }
        node = node.nextSibling();
    }
    else {
        return false;
    }
    return true;
}

int BracketElement::searchOperator( const QDomNode& node )
{
    QDomNode n = node;
    for ( int i = 0; ! n.isNull(); n = n.nextSibling(), i++ ) {
        if ( n.isElement() ) {
            QDomElement e = n.toElement();
            if ( e.tagName().lower() == "mo" ) {
                // Try to guess looking at attributes
                QString form = e.attribute( "form" );
                QString f;
                if ( ! form.isNull() ) {
                    f = form.stripWhiteSpace().lower();
                }
                QString fence = e.attribute( "fence" );
                if ( ! fence.isNull() ) {
                    if ( fence.stripWhiteSpace().lower() == "false" ) {
                        continue;
                    }
                    if ( ! f.isNull() ) {
                        if ( f == "postfix" ) {
                            return i;
                        }
                        else {
                            continue;
                        }
                    }
                }
                
                // Guess looking at contents
                QDomNode child = e.firstChild();
                QString name;
                if ( child.isText() )
                    name = child.toText().data().stripWhiteSpace();
                else if ( child.isEntityReference() )
                    name = child.nodeName();
                else 
                    continue;
                if ( name == ")"
                     || name == "]"
                     || name == "}"
                     || name == "CloseCurlyDoubleQuote"
                     || name == "CloseCurlyQuote"
                     || name == "RightAngleBracket"
                     || name == "RightCeiling"
                     || name == "RightDoubleBracket"
                     || name == "RightFloor" ) {
                    if ( f.isNull() || f == "postfix" )
                        return i;
                }
                if ( name == "("
                     || name == "["
                     || name == "{"
                     || name == "LeftAngleBracket"
                     || name == "LeftCeiling"
                     || name == "LeftDoubleBracket"
                     || name == "LeftFloor"
                     || name == "OpenCurlyQuote" ) {
                    if ( ! f.isNull() && f == "postfix" )
                        return i;
                }
            }
        }
    }
    return -1;
}


void BracketElement::writeMathMLAttributes( QDomElement& element ) const
{
    if ( left->getType() != LeftRoundBracket ||
         right->getType() != RightRoundBracket )
    {
        element.setAttribute( "open",  QString( QChar( leftType ) ) );
        element.setAttribute( "close", QString( QChar( rightType ) ) );
    }
    if ( ! m_separators.isNull() ) {
        element.setAttribute( "separators", m_separators );
    }
}
*/

} // namespace KFormula
