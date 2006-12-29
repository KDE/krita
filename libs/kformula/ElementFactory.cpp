/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
                 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "ElementFactory.h"

#include "ActionElement.h"
#include "BracketElement.h"
#include "EncloseElement.h"
#include "ErrorElement.h"
#include "FractionElement.h"
#include "GlyphElement.h"
#include "IdentifierElement.h"
#include "MatrixElement.h"
#include "MatrixEntryElement.h"
#include "MatrixRowElement.h"
#include "MultiscriptElement.h"
#include "NumberElement.h"
#include "OperatorElement.h"
#include "PaddedElement.h"
#include "PhantomElement.h"
#include "RootElement.h"
#include "SequenceElement.h"
#include "SpaceElement.h"
#include "StringElement.h"
#include "StyleElement.h"
#include "TextElement.h"
#include "UnderOverElement.h"


namespace KFormula {

BasicElement* ElementFactory::createElement( const QString& tagName,
                                             BasicElement* parent )
{
    // TODO
    // mlabeledtr
    // maligngroup
    // malignmark
    // Content elements
    // mtr and mtd are currently managed inside MatrixElement

    // Token Elements ( Section 3.1.6.1 )
    if      ( tagName == "mi" )            ;//return new IdentifierElement( parent );
//    else if ( tagName == "mo" )            return createOperatorElement( parent, element );
    //else if ( tagName == "mn" )            return new NumberElement( parent );
    //else if ( tagName == "mtext" )         return new TokenElement( parent );
    //else if ( tagName == "ms" )            return new StringElement( parent );
    //else if ( tagName == "mspace" )        return new SpaceElement( parent );
    //else if ( tagName == "mglyph" )        return new GlyphElement( parent );

    // General Layout Schemata ( Section 3.1.6.2 )
    else if ( tagName == "mrow" )          return new SequenceElement( parent );
    //else if ( tagName == "mfrac" )         return new FractionElement( parent );
    //else if ( tagName == "msqrt"
    //          || tagName == "mroot" )      return new RootElement( parent );
	//else if ( tagName == "mstyle" )        return new StyleElement( parent );
    //else if ( tagName == "merror" )        return new ErrorElement( parent );
    //else if ( tagName == "mpadded" )       return new PaddedElement( parent );
    //else if ( tagName == "mphantom" )      return new PhantomElement( parent );
    //else if ( tagName == "mfenced" )       return new BracketElement( parent );
    //else if ( tagName == "menclose" )      return new EncloseElement( parent );

    // Script and Limit Schemata ( Section 3.1.6.3 )
    //else if ( tagName == "msub"
    //        || tagName == "msup"
    //        || tagName == "msubsup" )    return new MultiscriptElement( parent );
    //else if ( tagName == "munder"
    //        || tagName == "mover"
    //        || tagName == "munderover" ) return new UnderOverElement( parent );
    //else if ( tagName == "mmultiscripts" ) return new MultiscriptElement( parent );

    return 0;
}

TextElement* ElementFactory::createTextElement( const QChar& ch, bool symbol )
{
    return new TextElement( ch, symbol );
}

/*
EmptyElement* ElementFactory::createEmptyElement()
{
    return new EmptyElement;
}

NameSequence* ElementFactory::createNameSequence()
{
    return new NameSequence;
}

BracketElement* ElementFactory::createBracketElement( SymbolType lhs, SymbolType rhs )
{
    return new BracketElement( lhs, rhs );
}

OverlineElement* ElementFactory::createOverlineElement()
{
    return new OverlineElement;
}

UnderlineElement* ElementFactory::createUnderlineElement()
{
    return new UnderlineElement;
}

MultilineElement* ElementFactory::createMultilineElement()
{
    return new MultilineElement;
}

SpaceElement* ElementFactory::createSpaceElement( SpaceWidth width )
{
    return new SpaceElement( width );
}
*/

FractionElement* ElementFactory::createFractionElement()
{
    //return new FractionElement;
}

RootElement* ElementFactory::createRootElement()
{
    //return new RootElement;
}

/*
SymbolElement* ElementFactory::createSymbolElement( SymbolType type )
{
    return new SymbolElement( type );
}

MatrixElement* ElementFactory::createMatrixElement( uint rows, uint columns )
{
    return new MatrixElement( rows, columns );
}

IndexElement* ElementFactory::createIndexElement()
{
    return new IndexElement;
}
*/

BasicElement* ElementFactory::createOperatorElement( const QDomElement& element )
{
    QDomNode n = element.firstChild();
    if ( n.isNull() )
        return 0;
    if ( n.isEntityReference() ) {
        QString name = n.nodeName();
        if ( name == "CloseCurlyDoubleQuote"
             || name == "CloseCurlyQuote"
             || name == "LeftAngleBracket"
             || name == "LeftCeiling"
             || name == "LeftDoubleBracket"
             || name == "LeftFloor"
             || name == "OpenCurlyDoubleQuote"
             || name == "OpenCurlyQuote"
             || name == "RightAngleBracket"
             || name == "RightCeiling"
             || name == "RightDoubleBracket"
             || name == "RightFloor" ) {
            //return new BracketElement();
        }
        //return new OperatorElement();
    }
    if ( n.isText() ) {
        QString text = n.toText().data();
        if ( text.length() == 1 && QString("()[]{}").contains(text[0]) ) {
            //return new BracketElement();
        }
    }
    //return new OperatorElement();
}

IdentifierElement* ElementFactory::createIdentifierElement()
{
    //return new IdentifierElement();
}

OperatorElement* ElementFactory::createOperatorElement()
{
    //return new OperatorElement();
}

NumberElement* ElementFactory::createNumberElement()
{
    //return new NumberElement();
}

} // namespace KFormula
