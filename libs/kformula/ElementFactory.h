/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#ifndef ELEMENTFACTORY_H
#define ELEMENTFACTORY_H

#include <QString>

#include "kformuladefs.h"

namespace KFormula {
	
class BasicElement;
class BracketElement;
class EmptyElement;
class FractionElement;
class IdentifierElement;
class IndexElement;
class MatrixElement;
class MultilineElement;
class NameSequence;
class NumberElement;
class OperatorElement;
class OverlineElement;
class RootElement;
class SpaceElement;
class SymbolElement;
class TextElement;
class UnderlineElement;

class ElementFactory {
public:
    ElementFactory();

    static BasicElement* createElement( const QString& tagName, BasicElement* parent );

    static TextElement* createTextElement( const QChar& ch, bool symbol=false );
    static EmptyElement* createEmptyElement();
    static NameSequence* createNameSequence();
    static BracketElement* createBracketElement( SymbolType lhs, SymbolType rhs );
    static OverlineElement* createOverlineElement();
    static UnderlineElement* createUnderlineElement();
    static MultilineElement* createMultilineElement();
    static SpaceElement* createSpaceElement( SpaceWidth width );
    static FractionElement* createFractionElement();
    static RootElement* createRootElement();
    static SymbolElement* createSymbolElement( SymbolType type );
    static MatrixElement* createMatrixElement( uint rows, uint columns );
    static IndexElement* createIndexElement();
    static IdentifierElement* createIdentifierElement();
    static OperatorElement* createOperatorElement();
    static NumberElement* createNumberElement();

    BasicElement* createOperatorElement( const QDomElement& element );
};

} // namespace KFormula

#endif // ELEMENTFACTORY_H
