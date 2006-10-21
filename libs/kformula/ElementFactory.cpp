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
    if      ( type == "mi" )            return new IdentifierElement( parent );
    else if ( type == "mo" )            return createOperatorElement( parent, element );
    else if ( type == "mn" )            return new NumberElement( parent );
    else if ( type == "mtext" )         return new TokenElement( parent );
    else if ( type == "ms" )            return new StringElement( parent );
    else if ( type == "mspace" )        return new SpaceElement( parent );
    else if ( type == "mglyph" )        return new GlyphElement( parent );

    // General Layout Schemata ( Section 3.1.6.2 )
    else if ( type == "mrow" )          return new SequenceElement( parent );
    else if ( type == "mfrac" )         return new FractionElement( parent );
    else if ( type == "msqrt"
              || type == "mroot" )      return new RootElement( parent );
	else if ( type == "mstyle" )        return new StyleElement( parent );
    else if ( type == "merror" )        return new ErrorElement( parent );
    else if ( type == "mpadded" )       return new PaddedElement( parent );
    else if ( type == "mphantom" )      return new PhantomElement( parent );
    else if ( type == "mfenced" )       return new BracketElement( parent );
    else if ( type == "menclose" )      return new EncloseElement( parent );

    // Script and Limit Schemata ( Section 3.1.6.3 )
    else if ( type == "msub"
              || type == "msup"
              || type == "msubsup" )    return new MultiscriptElement( parent );
    else if ( type == "munder"
              || type == "mover"
              || type == "munderover" ) return new UnderOverElement( parent );
    else if ( type == "mmultiscripts" ) return new MultiscriptElement( parent );

    return 0;
}

TextElement* OasisCreationStrategy::createTextElement( const QChar& ch, bool symbol )
{
    return new TextElement( ch, symbol );
}

EmptyElement* OasisCreationStrategy::createEmptyElement()
{
    return new EmptyElement;
}

NameSequence* OasisCreationStrategy::createNameSequence()
{
    return new NameSequence;
}

BracketElement* OasisCreationStrategy::createBracketElement( SymbolType lhs, SymbolType rhs )
{
    return new BracketElement( lhs, rhs );
}

OverlineElement* OasisCreationStrategy::createOverlineElement()
{
    return new OverlineElement;
}

UnderlineElement* OasisCreationStrategy::createUnderlineElement()
{
    return new UnderlineElement;
}

MultilineElement* OasisCreationStrategy::createMultilineElement()
{
    return new MultilineElement;
}

SpaceElement* OasisCreationStrategy::createSpaceElement( SpaceWidth width )
{
    return new SpaceElement( width );
}

FractionElement* OasisCreationStrategy::createFractionElement()
{
    return new FractionElement;
}

RootElement* OasisCreationStrategy::createRootElement()
{
    return new RootElement;
}

SymbolElement* OasisCreationStrategy::createSymbolElement( SymbolType type )
{
    return new SymbolElement( type );
}

MatrixElement* OasisCreationStrategy::createMatrixElement( uint rows, uint columns )
{
    return new MatrixElement( rows, columns );
}

IndexElement* OasisCreationStrategy::createIndexElement()
{
    return new IndexElement;
}

BasicElement* OasisCreationStrategy::createOperatorElement( const QDomElement& element )
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
            return new BracketElement();
        }
        return new OperatorElement();
    }
    if ( n.isText() ) {
        QString text = n.toText().data();
        if ( text.length() == 1 && QString("()[]{}").contains(text[0]) ) {
            return new BracketElement();
        }
    }
    return new OperatorElement();
}

IdentifierElement* OasisCreationStrategy::createIdentifierElement()
{
    return new IdentifierElement();
}

OperatorElement* OasisCreationStrategy::createOperatorElement()
{
    return new OperatorElement();
}

NumberElement* OasisCreationStrategy::createNumberElement()
{
    return new NumberElement();
}

} // namespace KFormula
