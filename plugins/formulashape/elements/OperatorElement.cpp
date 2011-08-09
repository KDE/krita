/* This file is part of the KDE project
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "OperatorElement.h"
#include "RowElement.h"
#include "AttributeManager.h"
#include "kdebug.h"
#include <QFontMetricsF>

OperatorElement::OperatorElement( BasicElement* parent ) : TokenElement( parent )
{}

QPainterPath OperatorElement::renderForFence( const QString& raw, Form form )
{
    Q_UNUSED( raw )
    Q_UNUSED( form )
    return QPainterPath();
}

QRectF OperatorElement::renderToPath( const QString& raw, QPainterPath& path ) const
{
    AttributeManager manager;

    qreal rSpace = manager.parseMathSpace(m_dict.rSpace(), this);
    qreal lSpace = manager.parseMathSpace(m_dict.lSpace(), this);
    path.moveTo( path.currentPosition() + QPointF( lSpace, 0.0 ) );
    QFont font = manager.font(this);
    path.addText( path.currentPosition(), font, raw );
    QFontMetricsF fm(font);
    QRectF rect = path.boundingRect().adjusted(0,0,lSpace+rSpace,0);
    return rect;
//    return fm.boundingRect(QRect(), Qt::TextIncludeTrailingSpaces, raw).adjusted(0,0,lSpace+rSpace,0).adjusted(0,-fm.ascent(), 0, -fm.ascent());
}

/* stretching rules are:
 * If a stretchy operator or fence is a direct subexpression of an <MROW>, or is the sole direct subexpression of an <MTD> in an <MTR>, then it should stretch to cover the height and depth (above and below the axis) of the non-stretchy subexpressions in the <MROW> or <MTR> element, given the constraints mentioned above. This applies even if the <MTD> and/or <MTR> were inferred, as described in the <MTABLE> section.
 
 * If a stretchy operator is a direct subexpression of an <MUNDER>, <MOVER>, <MUNDEROVER>, or if it is the sole direct subexpression of an <MTD>, then it should stretch to cover the width of the other subexpressions in the given element (or in the same matrix column, in the case of a matrix), given the constraints mentioned above. This applies even if the <MTD> was inferred, as described in the <MTABLE> section.
 */
void OperatorElement::stretch()
{
    m_stretchVertically = m_stretchHorizontally = false;

    if(!parentElement())
        return;
 
    if(!m_dict.stretchy())
        return;
 
    switch(parentElement()->elementType()) {
    case TableData:  //MTD
        if(parentElement()->childElements().count() == 1) {
            m_stretchHorizontally = true;
            if(parentElement()->parentElement() &&
               parentElement()->parentElement()->elementType() == TableRow)
                m_stretchVertically = true;
        }
        break;
    case Under:
    case Over:
    case UnderOver:
        m_stretchHorizontally = true;
        break;
    default:
        // There are many element types that inherit Row, so just try casting to
        // a row to see if it inherits from a row
        if( dynamic_cast<RowElement*>( parentElement() ) != 0 )
            m_stretchVertically = true;
        else
            return;
        break;
    }
    if(m_stretchVertically) {
        //FIXME - take into account maximum stretch size
        qreal newHeight = parentElement()->childrenBoundingRect().height();
        qreal newBaseLine = baseLine() * newHeight / height();
        setBaseLine( newBaseLine );
        setHeight( newHeight );
    }
    if(m_stretchHorizontally) {
        setWidth( parentElement()->width() );
    }
}
 
Form OperatorElement::determineOperatorForm() const
{
    // a bit of a hack - determine the operator's form with its position inside the
    // parent's element list. This is with the assumption that the parent is an 
    // ( inferred ) row element. If that is not the case return standard Prefix ( ? )
 
    if( dynamic_cast<RowElement*>( parentElement() ) == 0 )
        return Prefix;
    if( parentElement()->childElements().isEmpty() )
        return Prefix;
    if( parentElement()->childElements().first() == this )
        return Prefix;
    if( parentElement()->childElements().last() == this )
        return Postfix;
    return Infix;
}


Form OperatorElement::parseForm( const QString& value ) const
{
    if( value == "prefix" )
        return Prefix;
    else if( value == "infix" )
        return Infix;
    else if( value == "postfix" )
        return Postfix;
    else
        return InvalidForm;
}

ElementType OperatorElement::elementType() const
{
    return Operator;
}


bool OperatorElement::insertText ( int position, const QString& text )
{   
    if (m_rawString.isEmpty()) {
        m_dict.queryOperator( text, determineOperatorForm() );
    }
    return TokenElement::insertText ( position, text );
}

bool OperatorElement::readMathMLContent ( const KoXmlElement& parent )
{
    bool tmp = TokenElement::readMathMLContent ( parent );
    m_dict.queryOperator( m_rawString, determineOperatorForm() );
    return tmp;
}
