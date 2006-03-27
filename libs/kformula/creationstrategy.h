/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>

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


#ifndef CREATIONSTRATEGY_H
#define CREATIONSTRATEGY_H


#include <qstring.h>

#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN

class BasicElement;
class BracketElement;
class EmptyElement;
class FractionElement;
class IndexElement;
class MatrixElement;
class MultilineElement;
class NameSequence;
class OverlineElement;
class RootElement;
class SpaceElement;
class SymbolElement;
class TextElement;
class UnderlineElement;

/**
 * An object of this type needs to be known by the SequenceElement.
 * It decides what elements can be constructed.
 */
class ElementCreationStrategy {
public:
    virtual ~ElementCreationStrategy() {}

    virtual BasicElement* createElement( QString type ) = 0;

    /// there must always be a TextElement, so this can never return 0
    virtual TextElement* createTextElement( const QChar& ch, bool symbol=false ) = 0;

    /// when this gets called the user has seen the matrix dialog and expects a matrix!
    virtual MatrixElement* createMatrixElement( uint rows, uint columns ) = 0;

    virtual EmptyElement* createEmptyElement() = 0;
    virtual NameSequence* createNameSequence() = 0;
    virtual BracketElement* createBracketElement( SymbolType lhs, SymbolType rhs ) = 0;
    virtual OverlineElement* createOverlineElement() = 0;
    virtual UnderlineElement* createUnderlineElement() = 0;
    virtual MultilineElement* createMultilineElement() = 0;
    virtual SpaceElement* createSpaceElement( SpaceWidth width ) = 0;
    virtual FractionElement* createFractionElement() = 0;
    virtual RootElement* createRootElement() = 0;
    virtual SymbolElement* createSymbolElement( SymbolType type ) = 0;
    virtual IndexElement* createIndexElement() = 0;
};


/**
 * The ordinary strategy to be used for plain kformula.
 */
class OrdinaryCreationStrategy : public ElementCreationStrategy {
public:
    virtual BasicElement* createElement( QString type );

    virtual TextElement* createTextElement( const QChar& ch, bool symbol=false );
    virtual EmptyElement* createEmptyElement();
    virtual NameSequence* createNameSequence();
    virtual BracketElement* createBracketElement( SymbolType lhs, SymbolType rhs );
    virtual OverlineElement* createOverlineElement();
    virtual UnderlineElement* createUnderlineElement();
    virtual MultilineElement* createMultilineElement();
    virtual SpaceElement* createSpaceElement( SpaceWidth width );
    virtual FractionElement* createFractionElement();
    virtual RootElement* createRootElement();
    virtual SymbolElement* createSymbolElement( SymbolType type );
    virtual MatrixElement* createMatrixElement( uint rows, uint columns );
    virtual IndexElement* createIndexElement();
};


KFORMULA_NAMESPACE_END

#endif // CREATIONSTRATEGY_H
