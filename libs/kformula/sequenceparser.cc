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

#include "BasicElement.h"
#include "elementtype.h"
#include "sequenceparser.h"
#include "symboltable.h"
#include "textelement.h"


KFORMULA_NAMESPACE_BEGIN


SequenceParser::SequenceParser( const SymbolTable& t )
        : tokenStart( 0 ), tokenEnd( 0 ), type( SEQUENCE ),
          binOpAllowed( false ), table( t )
{
}


void SequenceParser::setElementType( int pos, ElementType* type )
{
//    list.at( pos )->setElementType( type );
}


ElementType* SequenceParser::parse( QList<BasicElement*>& elements )
{
    list = elements;
    return new SequenceType( this );
}


void SequenceParser::nextToken()
{
    tokenStart = tokenEnd;
    if ( tokenStart >= list.count() ) {
        type = END;
        return;
    }
    tokenEnd++;
    BasicElement* element = list.at( tokenStart );
    type = element->getTokenType();
    if ( type == SEPARATOR ) {
        if ( tokenEnd < list.count() ) {
            QChar ch = getEndChar();
			if(ch == ',' | ch == '>' | ch == ';')
			{
				type = NAME;
				readText();
			}
			else
				readText();
        }
    }
    else if ( type == ORDINARY ) {
        readText();
    }
    else if ( type == NUMBER ) {
        readNumber();
    }
    if ( !binOpAllowed && ( type == BINOP ) ) {
        type = ORDINARY;
    }
    binOpAllowed = ( type == ORDINARY ) || ( type == NUMBER ) || ( type == NAME ) ||
          ( type == ELEMENT ) || ( type == BRACKET ) || ( type == INNER );

    //cerr << "SequenceParser::nextToken(): " << type << " "
    //     << tokenStart << " " << tokenEnd << endl;
}


void SequenceParser::readNumber()
{
    type = NUMBER;
    readDigits();
    if ( tokenEnd < list.count()-1 ) {
        QChar ch = getEndChar();

        // Look for a dot.
        if ( ch == '.' ) {
            tokenEnd++;
            ch = getEndChar();
            if ( ch.isNumber() ) {
                readDigits();
            }
//             else {
//                 tokenEnd--;
//                 return;
//             }
        }

        // there might as well be an exponent
        if ( tokenEnd < list.count()-1 ) {
            BasicElement* element = list.at(tokenEnd);
            ch = getEndChar();
            if ( ( element->getTokenType() == ORDINARY ) &&
                 ( ( ch == 'E' ) || ( ch == 'e' ) ) ) {
                tokenEnd++;
                ch = getEndChar();

                // signs are allowed after the exponent
                if ( ( ( ch == '+' ) || ( ch == '-' ) ) &&
                     ( tokenEnd < list.count()-1 ) ) {
                    tokenEnd++;
                    ch = getEndChar();
                    if ( ch.isNumber() ) {
                        readDigits();
                    }
                    else {
                        tokenEnd -= 2;
                        return;
                    }
                }
                else if ( ch.isNumber() ) {
                    readDigits();
                }
                else {
                    tokenEnd--;
                }
            }
        }
    }
}


void SequenceParser::readDigits()
{
    for ( ; tokenEnd < list.count(); tokenEnd++ ) {
        QChar ch = getEndChar();
        if ( !ch.isNumber() ) {
            break;
        }
    }
}


void SequenceParser::readText()
{
    BasicElement* element = list.at( tokenStart );
    TextElement* beginText = static_cast<TextElement*>( element );
    if ( beginText->isSymbol() ||
        ( beginText->getCharacter() == '/' ) ) {
        return;
    }
    char format = beginText->format();
    type = ORDINARY;
    for ( ; tokenEnd < list.count(); tokenEnd++ ) {
        element = list.at( tokenEnd );
        TokenType tt = element->getTokenType();
        if ( ( ( tt != ORDINARY ) ||
               ( element->getCharacter() == '/' ) ) &&
             ( tt != NUMBER ) ) {
            return;
        }
        if ( static_cast<TextElement*>( element )->format() != format ) {
            return;
        }
        if ( static_cast<TextElement*>( element )->isSymbol() ) {
            return;
        }
    }
}

QChar SequenceParser::getEndChar()
{
    BasicElement* element = list.at( tokenEnd );
    return element->getCharacter();
}


ElementType* SequenceParser::getPrimitive()
{
    //cerr << "SequenceParser::getPrimitive(): " << type << " "
    //     << tokenStart << " " << tokenEnd << endl;
    switch ( type ) {
    case ORDINARY: {
//         QString text = getText();
//         if ( table.contains( text ) || ( text == "\\quad" ) ) {
//             return new NameType( this, text );
//         }
//         else {
            return new TextType( this );
//         }
    }
    case NAME:
        return new NameType( this );
    case NUMBER:
        return new NumberType( this );
    case ELEMENT:
        return new ComplexElementType( this );
    case INNER:
        return new InnerElementType( this );
    case BINOP:
        return new OperatorType( this );
    case RELATION:
        return new RelationType( this );
    case PUNCTUATION:
        return new PunctuationType( this );
    case BRACKET:
        return new BracketType( this );
    case SEQUENCE:
    case SEPARATOR:
    case END:
        return 0;
    }
    return 0;
}


QString SequenceParser::text()
{
    QString text;
    for ( uint i = tokenStart; i < tokenEnd; i++ ) {
        BasicElement* element = list.at( i );
        text.append( element->getCharacter() );
    }
    return text;
}

KFORMULA_NAMESPACE_END
