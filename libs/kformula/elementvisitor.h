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

#ifndef ELEMENTVISITOR_H
#define ELEMENTVISITOR_H

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
class SequenceElement;
class SpaceElement;
class SymbolElement;
class TextElement;
class UnderlineElement;


/**
 * Visitor. Provides a way to add new functionality to the element tree.
 */
class ElementVisitor {
public:

    virtual ~ElementVisitor() {}

    virtual bool visit( BracketElement* ) { return true; }
    virtual bool visit( EmptyElement* ) { return true; }
    virtual bool visit( FractionElement* ) { return true; }
    virtual bool visit( IndexElement* ) { return true; }
    virtual bool visit( MatrixElement* ) { return true; }
    virtual bool visit( MultilineElement* ) { return true; }
    virtual bool visit( NameSequence* ) { return true; }
    virtual bool visit( OverlineElement* ) { return true; }
    virtual bool visit( RootElement* ) { return true; }
    virtual bool visit( SequenceElement* ) { return true; }
    virtual bool visit( SpaceElement* ) { return true; }
    virtual bool visit( SymbolElement* ) { return true; }
    virtual bool visit( TextElement* ) { return true; }
    virtual bool visit( UnderlineElement* ) { return true; }
};

KFORMULA_NAMESPACE_END

#endif // ELEMENTVISITOR_H
