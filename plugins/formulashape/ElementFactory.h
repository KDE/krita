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
#include "kformula_export.h"

class BasicElement;

enum ElementType {
    Basic,
    Formula,
    Row,
    Identifier,
    Number,
    Operator,
    Space,
    Fraction,
    Table,
    TableRow,
    TableData,
    Under,
    Over,
    UnderOver,
    MultiScript,
    SupScript,
    SubScript,
    SubSupScript,
    Root,
    SquareRoot,
    Text,
    Style,
    Padded,
    Error,
    Fenced,
    Glyph,
    String,
    Enclose,
    Phantom,
    Action,
    Annotation,
    Unknown,
    Empty
};


/**
 * @short An implementation of the factory pattern to create element instances
 *
 * The creation of new BasicElement derived classes is an often done task. While
 * loading ElementFactory provides a very simple way to achieve an element by
 * passing its MathML name. Just use the static createElement() method.
 * While saving the elementName() method is used to map the ElementType's to the
 * according MathML names.
 *
 * @author Martin Pfeiffer
 */
class KOFORMULA_EXPORT ElementFactory {
public:
    /// The default constructor
    ElementFactory();

    /**
     * Obtain new instances of elements by passing the MathML tag name
     * @param tagName The MathML tag name of the new element
     * @param parent The parent element of the newly created element
     * @return A pointer to the new BasicElement derived element
     */
    static BasicElement* createElement( const QString& tagName, BasicElement* parent );

    /**
     * Obtain the MathML name of a ElementType.
     * @param type The given ElementType to get the MathML name from
     * @return The MathML name as QString
     */
    static QString elementName( ElementType type );
};

#endif // ELEMENTFACTORY_H
