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

#ifndef ELEMENTTYPE_H
#define ELEMENTTYPE_H

#include <QFont>
#include <QString>
#include <qdom.h>

#include "contextstyle.h"
#include "kformuladefs.h"

class QPainter;

KFORMULA_NAMESPACE_BEGIN

class BasicElement;
class BracketType;
class ComplexElementType;
class InnerElementType;
class MultiElementType;
class OperatorType;
class PunctuationType;
class RelationType;
class SequenceElement;
class SequenceParser;
class TextElement;


/**
 * Basis of all types. Types make up a hierarchy that describes
 * the semantic of the sequence.
 */
class KOFORMULA_EXPORT ElementType { // exported for unit tests
public:
    ElementType(SequenceParser* parser);
    virtual ~ElementType();

    /**
     * @returns whether we want to see this element.
     */
    virtual bool isInvisible(const TextElement&) const { return false; }

    /**
     * @returns the spanned text. seq must be the original
     * parent sequence.
     */
    virtual QString text( SequenceElement* seq ) const;

    /**
     * @returns the position of the first character
     */
    uint start() const { return from; }

    /**
     * @returns the position of the first character after the typed element
     */
    uint end() const { return to; }

    /**
     * @returns the space to be left before each char
     * for the given style and font size.
     */
    virtual luPt getSpaceBefore( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( MultiElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( OperatorType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( RelationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( PunctuationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( BracketType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( ComplexElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( InnerElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );

    /**
     * @returns the font to be used for this kind of element
     */
    virtual QFont getFont( const ContextStyle& context );

    /**
     * sets the painters pen to a appropriate value
     */
    virtual void setUpPainter( const ContextStyle& context, QPainter& painter );

    // debug
    static int getEvilDestructionCount() { return evilDestructionCount; }

    virtual void output();

    /**
     * Adds a type at the end of the list.
     */
    void append( ElementType* );

    ElementType* getPrev() const { return prev; }

    virtual void saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat = false );

    virtual bool multiElement() const { return false; }

protected:

    void setStart( uint start ) { from = start; }
    void setEnd( uint end ) { to = end; }

    luPt thinSpaceIfNotScript( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    luPt mediumSpaceIfNotScript( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    luPt thickSpaceIfNotScript( const ContextStyle& context, ContextStyle::TextStyle tstyle );

private:

    /**
     * the index of the first element that belongs
     * to the name.
     */
    uint from;

    /**
     * the index of the first element that doesn't belong
     * to the name.
     */
    uint to;

    /**
     * We implement this list ourselves because we need to know
     * our neighbours.
     */
    ElementType* prev;

    // debug
    static int evilDestructionCount;
};


/**
 * The token that belongs to a sequence. Contains all the
 * other tokens.
 */
class SequenceType : public ElementType {
public:
    SequenceType( SequenceParser* parser );
    ~SequenceType();

    virtual void output();
private:

    /**
     * The last token type of this sequences chain.
     */
    ElementType* last;
};


/**
 * Basis for all tokens that run along several elements.
 */
class MultiElementType : public ElementType {
public:
    MultiElementType( SequenceParser* parser );

    virtual luPt getSpaceBefore( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( OperatorType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( RelationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( InnerElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );

    virtual bool multiElement() const { return true; }

    /**
     * @returns the spanned text. seq must be the original
     * parent sequence.
     */
    virtual QString text( SequenceElement* /*seq*/ ) const { return m_text; }

private:

    QString m_text;
};


/**
 * A text element that doesn't belong to an name.
 * This might be considered an error.
 */
class TextType : public MultiElementType {
public:
    TextType( SequenceParser* parser );
    virtual void saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat = false );
};


/**
 * A range of elements that make up a number.
 */
class NumberType : public MultiElementType {
public:
    NumberType(SequenceParser* parser);

    /**
     * @returns the font to be used for this kind of element
     */
    virtual QFont getFont(const ContextStyle& context);

    /**
     * sets the painters pen to a appropriate value
     */
    virtual void setUpPainter(const ContextStyle& context, QPainter& painter);

    virtual void saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat = false );
};


/**
 * Basis for all tokens that consist of one element only.
 */
class SingleElementType : public ElementType {
public:
    SingleElementType( SequenceParser* parser );
};


/**
 * A recognized name.
 */
class NameType : public MultiElementType {
public:
    NameType( SequenceParser* parser );

    /**
     * @returns the font to be used for this kind of element
     */
    virtual QFont getFont( const ContextStyle& context );

    virtual void saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat = false );

private:
};


class AbstractOperatorType : public SingleElementType {
public:
    AbstractOperatorType( SequenceParser* parser );

    void saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat = false  );
};

class OperatorType : public AbstractOperatorType {
public:
    OperatorType( SequenceParser* parser );

    virtual luPt getSpaceBefore( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( MultiElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( BracketType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( ComplexElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( InnerElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );

    /**
     * @returns the font to be used for this kind of element
     */
    virtual QFont getFont(const ContextStyle& context);

    /**
     * sets the painters pen to a appropriate value
     */
    virtual void setUpPainter(const ContextStyle& context, QPainter& painter);
};


class RelationType : public AbstractOperatorType {
public:
    RelationType( SequenceParser* parser );

    virtual luPt getSpaceBefore( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( MultiElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( BracketType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( ComplexElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( InnerElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );

    /**
     * @returns the font to be used for this kind of element
     */
    virtual QFont getFont( const ContextStyle& context );

    /**
     * sets the painters pen to a appropriate value
     */
    virtual void setUpPainter( const ContextStyle& context, QPainter& painter );
};


class PunctuationType : public AbstractOperatorType {
public:
    PunctuationType( SequenceParser* parser );

    virtual luPt getSpaceBefore( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( MultiElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( RelationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( PunctuationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( BracketType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( ComplexElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( InnerElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );

    /**
     * @returns the font to be used for this kind of element
     */
    virtual QFont getFont( const ContextStyle& context );

    /**
     * sets the painters pen to a appropriate value
     */
    virtual void setUpPainter( const ContextStyle& context, QPainter& painter );
};


class BracketType : public SingleElementType {
public:
    BracketType( SequenceParser* parser );

    virtual luPt getSpaceBefore( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( OperatorType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( RelationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( InnerElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
};


class ComplexElementType : public SingleElementType {
public:
    ComplexElementType( SequenceParser* parser );

    // these spacings are equal to the ones from MultiElementType
    virtual luPt getSpaceBefore( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( OperatorType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( RelationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( InnerElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
};


class InnerElementType : public SingleElementType {
public:
    InnerElementType( SequenceParser* parser );

    virtual luPt getSpaceBefore( const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( MultiElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( OperatorType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( RelationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( PunctuationType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( BracketType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( ComplexElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
    virtual luPt getSpaceAfter( InnerElementType* type, const ContextStyle& context, ContextStyle::TextStyle tstyle );
};


KFORMULA_NAMESPACE_END

#endif // ELEMENTTYPE_H
