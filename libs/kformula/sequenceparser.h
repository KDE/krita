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

#ifndef SEQUENCEPARSER_H
#define SEQUENCEPARSER_H

#include <QList>
#include <QString>

#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN

class BasicElement;
class ElementType;
class SymbolTable;


/**
 * The parser that gets the element list and returns a syntax tree.
 */
class SequenceParser {
public:
    SequenceParser(const SymbolTable& table);

    /**
     * @returns a parse tree.
     */
    ElementType* parse( QList<BasicElement*>& elements);

    /**
     * Reads the next token.
     */
    void nextToken();

    int getStart() const { return tokenStart; }
    int getEnd() const { return tokenEnd; }
    TokenType getTokenType() const { return type; }

    /**
     * Tells the element about its new type.
     *
     * @param pos the position of the element
     * @param type the new type
     */
    void setElementType( int pos, ElementType* type);

    /**
     * @returns a new primitive token.
     */
    ElementType* getPrimitive();

    /**
     * @returns the current token's text
     */
    QString text();

private:

    /**
     * Reads the next token which is a number.
     */
    void readNumber();

    /**
     * Reads a sequence of digits.
     */
    void readDigits();

    /**
     * Reads the next token which is some text.
     */
    void readText();

    /**
     * @returns the char at tokenEnd.
     */
    QChar getEndChar();

    /**
     * The elements we want to parse. The parser must not change
     * it!
     */
    QList<BasicElement*> list;

    /**
     * The position up to which we have read. The current
     * token starts here.
     */
    int tokenStart;

    /**
     * The first position after the current token.
     */
    int tokenEnd;

    /**
     * The type of the current token.
     */
    TokenType type;

    /**
     * Whether the next token might be a binary operator.
     */
    bool binOpAllowed;

    /**
     * The table that contains all known symbols.
     */
    const SymbolTable& table;
};

KFORMULA_NAMESPACE_END

#endif // SEQUENCEPARSER_H
