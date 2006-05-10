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

#include <QFont>
#include <qfontmetrics.h>
#include <qpainter.h>

#include <kdebug.h>

#include "basicelement.h"
#include "contextstyle.h"
#include "elementtype.h"
#include "sequenceelement.h"
#include "sequenceparser.h"
#include "textelement.h"


KFORMULA_NAMESPACE_BEGIN

int ElementType::evilDestructionCount = 0;

/*
 * Converts CharStyle and CharFamily to the MathML 'mathvariant'
 * attribute (see MathML spec 3.2.2).
 */
QString format2variant( CharStyle style, CharFamily family )
{
    QString result;

    switch( family ) {
    case normalFamily:
    case anyFamily:
        switch( style ) {
        case normalChar:
            result = "normal"; break;
        case boldChar:
            result = "bold"; break;
        case italicChar:
            result = "italic"; break;
        case boldItalicChar:
            result = "bold-italic"; break;
        case anyChar:
            break;
        }
        break;
    case scriptFamily:
        result = "script";
        if ( style == boldChar || style == boldItalicChar )
            result = "bold-" + result;
        break;
    case frakturFamily:
        result = "fraktur";
        if ( style == boldChar || style == boldItalicChar )
            result = "bold-" + result;
        break;
    case doubleStruckFamily:
        result = "double-struck"; break;
    }

    return result;
}

ElementType::ElementType( SequenceParser* parser )
    : from( parser->getStart() ), to( parser->getEnd() ), prev( 0 )
{
    evilDestructionCount++;
}

ElementType::~ElementType()
{
    delete prev;
    evilDestructionCount--;
}


QString ElementType::text( SequenceElement* seq ) const
{
    QString str;
    for ( uint i=start(); i<end(); ++i ) {
        str.append( seq->getChild( i )->getCharacter() );
    }
    return str;
}


luPt ElementType::getSpaceBefore( const ContextStyle&,
                                ContextStyle::TextStyle )
{
    return 0;
}

luPt ElementType::getSpaceAfter( MultiElementType*,
                               const ContextStyle&,
                               ContextStyle::TextStyle )
{
    return 0;
}

luPt ElementType::getSpaceAfter( OperatorType*,
                               const ContextStyle&,
                               ContextStyle::TextStyle )
{
    return 0;
}

luPt ElementType::getSpaceAfter( RelationType*,
                               const ContextStyle&,
                               ContextStyle::TextStyle )
{
    return 0;
}

luPt ElementType::getSpaceAfter( PunctuationType*,
                               const ContextStyle&,
                               ContextStyle::TextStyle )
{
    return 0;
}

luPt ElementType::getSpaceAfter( BracketType*,
                               const ContextStyle&,
                               ContextStyle::TextStyle )
{
    return 0;
}

luPt ElementType::getSpaceAfter( ComplexElementType*,
                               const ContextStyle&,
                               ContextStyle::TextStyle )
{
    return 0;
}

luPt ElementType::getSpaceAfter( InnerElementType*,
                               const ContextStyle&,
                               ContextStyle::TextStyle )
{
    return 0;
}

luPt ElementType::thinSpaceIfNotScript( const ContextStyle& context,
                                      ContextStyle::TextStyle tstyle )
{
    if ( !context.isScript( tstyle ) ) {
        return context.getThinSpace( tstyle );
    }
    return 0;
}

luPt ElementType::mediumSpaceIfNotScript( const ContextStyle& context,
                                        ContextStyle::TextStyle tstyle )
{
    if ( !context.isScript( tstyle ) ) {
        return context.getMediumSpace( tstyle );
    }
    return 0;
}

luPt ElementType::thickSpaceIfNotScript( const ContextStyle& context,
                                       ContextStyle::TextStyle tstyle )
{
    if ( !context.isScript( tstyle ) ) {
        return context.getThickSpace( tstyle );
    }
    return 0;
}


QFont ElementType::getFont(const ContextStyle& context)
{
    return context.getDefaultFont();
}

void ElementType::setUpPainter(const ContextStyle& context, QPainter& painter)
{
    painter.setPen(context.getDefaultColor());
}

void ElementType::append( ElementType* element )
{
    element->prev = this;
}

void ElementType::output()
{
    kDebug( DEBUGID ) << start() << " - " << end() << endl;
}

void ElementType::saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat )
{
    for ( uint i = from; i < to; ++i ) {
        se->getChild( i )->writeMathML( doc, de, oasisFormat );
    }
}


SequenceType::SequenceType( SequenceParser* parser )
    : ElementType( parser ), last( 0 )
{
    while ( true ) {
        parser->nextToken();
        //cerr << "SequenceType::SequenceType(): " << parser->getTokenType() << " "
        //     << parser->getStart() << " " << parser->getEnd() << endl;
        if ( parser->getTokenType() == END ) {
            break;
        }
        ElementType* nextType = parser->getPrimitive();
        if ( nextType == 0 ) {
            break;
        }
        if ( last != 0 ) {
            last->append( nextType );
        }
        last = nextType;
    }
}

SequenceType::~SequenceType()
{
    delete last;
}


void SequenceType::output()
{
}


MultiElementType::MultiElementType( SequenceParser* parser )
    : ElementType( parser )
{
    for ( uint i = start(); i < end(); i++ ) {
        parser->setElementType( i, this );
    }
    m_text = parser->text();
}


luPt MultiElementType::getSpaceBefore( const ContextStyle& context,
                                     ContextStyle::TextStyle tstyle )
{
    if ( getPrev() != 0 ) {
        return getPrev()->getSpaceAfter( this, context, tstyle );
    }
    return 0;
}

luPt MultiElementType::getSpaceAfter( OperatorType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return mediumSpaceIfNotScript( context, tstyle );
}

luPt MultiElementType::getSpaceAfter( RelationType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

luPt MultiElementType::getSpaceAfter( InnerElementType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}


TextType::TextType( SequenceParser* parser )
    : MultiElementType( parser )
{
}

void TextType::saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat )
{
    for ( uint i = start(); i < end(); ++i ) {
        QDomElement text = doc.createElement( oasisFormat ? "math:mi" : "mi" );
        BasicElement* be = se->getChild( i );
        TextElement* te = static_cast<TextElement*>( be );
        QString mathvariant = format2variant( te->getCharStyle(), te->getCharFamily());
        if ( !mathvariant.isNull() )
            text.setAttribute( "mathvariant", mathvariant );
        
        text.appendChild( doc.createTextNode( be->getCharacter() ) );

        de.appendChild( text );
        if ( i != end() - 1 ) {
            QDomElement op = doc.createElement( oasisFormat ? "math:mo" : "mo" );
            op.appendChild( doc.createEntityReference( "InvisibleTimes" ) );
            de.appendChild( op );
        }
    }
}


NameType::NameType( SequenceParser* parser )
    : MultiElementType( parser )
{
}

void NameType::saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat )
{
    se->getChild( start() )->writeMathML( doc, de, oasisFormat );

    /*
    QDomElement name = doc.createElement( "mi" );
    QString value;
    for ( uint i = start(); i < end(); ++i ) {
        BasicElement* be = se->getChild( i );
        //TextElement* te = static_cast<TextElement*>( be );
        value += be->getCharacter();
    }
    name.appendChild( doc.createTextNode( value ) );
    de.appendChild( name );*/
}


QFont NameType::getFont(const ContextStyle& context)
{
    return context.getNameFont();
}

NumberType::NumberType( SequenceParser* parser )
    : MultiElementType( parser )
{
}

QFont NumberType::getFont(const ContextStyle& context)
{
    return context.getNumberFont();
}

void NumberType::setUpPainter(const ContextStyle& context, QPainter& painter)
{
    painter.setPen(context.getNumberColor());
}

void NumberType::saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat )
{
    QDomElement name = doc.createElement( oasisFormat ? "math:mn"  : "mn" );
    QString value;
    for ( uint i = start(); i < end(); ++i ) {
        BasicElement* be = se->getChild( i );
        value += be->getCharacter();
    }
    TextElement* te = static_cast<TextElement*>( se->getChild( start() ) );
    QString mathvariant = format2variant( te->getCharStyle(), te->getCharFamily() );
    if ( !mathvariant.isNull() )
        name.setAttribute( "mathvariant", mathvariant );

    name.appendChild( doc.createTextNode( value ) );
    de.appendChild( name );
}


SingleElementType::SingleElementType( SequenceParser* parser )
    : ElementType( parser )
{
    parser->setElementType( start(), this );
}

AbstractOperatorType::AbstractOperatorType( SequenceParser* parser )
    : SingleElementType( parser )
{
}

void AbstractOperatorType::saveMathML( SequenceElement* se, QDomDocument& doc, QDomElement de, bool oasisFormat )
{
    QDomElement op = doc.createElement( oasisFormat ? "math:mo" : "mo" );
    BasicElement* be = se->getChild( start() );
    if ( be->getCharacter().latin1() != 0 ) {
        // latin-1 char
        op.appendChild( doc.createTextNode( be->getCharacter() ) );
    }
    else {
        // unicode char
        QString s;
        op.appendChild( doc.createEntityReference( s.sprintf( "#x%05X", be->getCharacter().unicode() ) ) );
    }
    TextElement* te = static_cast<TextElement*>( be );
    QString mathvariant = format2variant( te->getCharStyle(), te->getCharFamily() );
    if ( !mathvariant.isNull() )
        op.setAttribute( "mathvariant", mathvariant );

    de.appendChild( op );
}

OperatorType::OperatorType( SequenceParser* parser )
    : AbstractOperatorType( parser )
{
}

luPt OperatorType::getSpaceBefore( const ContextStyle& context,
                                 ContextStyle::TextStyle tstyle )
{
    if ( getPrev() != 0 ) {
        return getPrev()->getSpaceAfter( this, context, tstyle );
    }
    return 0;
}

luPt OperatorType::getSpaceAfter( MultiElementType*,
                                const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    return mediumSpaceIfNotScript( context, tstyle );
}

luPt OperatorType::getSpaceAfter( BracketType*,
                                const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    return mediumSpaceIfNotScript( context, tstyle );
}

luPt OperatorType::getSpaceAfter( ComplexElementType*,
                                const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    return mediumSpaceIfNotScript( context, tstyle );
}

luPt OperatorType::getSpaceAfter( InnerElementType*,
                                const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    return mediumSpaceIfNotScript( context, tstyle );
}


QFont OperatorType::getFont(const ContextStyle& context)
{
    return context.getOperatorFont();
}

void OperatorType::setUpPainter(const ContextStyle& context, QPainter& painter)
{
    painter.setPen(context.getOperatorColor());
}


RelationType::RelationType( SequenceParser* parser )
    : AbstractOperatorType( parser )
{
}

luPt RelationType::getSpaceBefore( const ContextStyle& context,
                                 ContextStyle::TextStyle tstyle )
{
    if ( getPrev() != 0 ) {
        return getPrev()->getSpaceAfter( this, context, tstyle );
    }
    return 0;
}

luPt RelationType::getSpaceAfter( MultiElementType*,
                                const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

luPt RelationType::getSpaceAfter( BracketType*,
                                const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

luPt RelationType::getSpaceAfter( ComplexElementType*,
                                const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

luPt RelationType::getSpaceAfter( InnerElementType*,
                                const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

QFont RelationType::getFont( const ContextStyle& context )
{
    return context.getOperatorFont();
}

void RelationType::setUpPainter( const ContextStyle& context, QPainter& painter )
{
    painter.setPen(context.getOperatorColor());
}



PunctuationType::PunctuationType( SequenceParser* parser )
    : AbstractOperatorType( parser )
{
}

luPt PunctuationType::getSpaceBefore( const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    if ( getPrev() != 0 ) {
        return getPrev()->getSpaceAfter( this, context, tstyle );
    }
    return 0;
}

luPt PunctuationType::getSpaceAfter( MultiElementType*,
                                   const ContextStyle& context,
                                   ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

luPt PunctuationType::getSpaceAfter( RelationType*,
                                   const ContextStyle& context,
                                   ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

luPt PunctuationType::getSpaceAfter( PunctuationType*,
                                   const ContextStyle& context,
                                   ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

luPt PunctuationType::getSpaceAfter( BracketType*,
                                   const ContextStyle& context,
                                   ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

luPt PunctuationType::getSpaceAfter( ComplexElementType*,
                                   const ContextStyle& context,
                                   ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

luPt PunctuationType::getSpaceAfter( InnerElementType*,
                                   const ContextStyle& context,
                                   ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

QFont PunctuationType::getFont( const ContextStyle& context )
{
    return context.getOperatorFont();
}

void PunctuationType::setUpPainter( const ContextStyle& context, QPainter& painter )
{
    painter.setPen( context.getDefaultColor() );
}


BracketType::BracketType( SequenceParser* parser )
    : SingleElementType( parser )
{
}

luPt BracketType::getSpaceBefore( const ContextStyle& context,
                                ContextStyle::TextStyle tstyle )
{
    if ( getPrev() != 0 ) {
        return getPrev()->getSpaceAfter( this, context, tstyle );
    }
    return 0;
}

luPt BracketType::getSpaceAfter( OperatorType*,
                               const ContextStyle& context,
                               ContextStyle::TextStyle tstyle )
{
    return mediumSpaceIfNotScript( context, tstyle );
}

luPt BracketType::getSpaceAfter( RelationType*,
                               const ContextStyle& context,
                               ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

luPt BracketType::getSpaceAfter( InnerElementType*,
                               const ContextStyle& context,
                               ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}


ComplexElementType::ComplexElementType( SequenceParser* parser )
    : SingleElementType( parser )
{
}

luPt ComplexElementType::getSpaceBefore( const ContextStyle& context,
                                       ContextStyle::TextStyle tstyle )
{
    if ( getPrev() != 0 ) {
        return getPrev()->getSpaceAfter( this, context, tstyle );
    }
    return 0;
}

luPt ComplexElementType::getSpaceAfter( OperatorType*,
                                      const ContextStyle& context,
                                      ContextStyle::TextStyle tstyle )
{
    return mediumSpaceIfNotScript( context, tstyle );
}

luPt ComplexElementType::getSpaceAfter( RelationType*,
                                      const ContextStyle& context,
                                      ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

luPt ComplexElementType::getSpaceAfter( InnerElementType*,
                                      const ContextStyle& context,
                                      ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}


InnerElementType::InnerElementType( SequenceParser* parser )
    : SingleElementType( parser )
{
}

luPt InnerElementType::getSpaceBefore( const ContextStyle& context,
                                     ContextStyle::TextStyle tstyle )
{
    if ( getPrev() != 0 ) {
        return getPrev()->getSpaceAfter( this, context, tstyle );
    }
    return 0;
}

luPt InnerElementType::getSpaceAfter( MultiElementType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

luPt InnerElementType::getSpaceAfter( OperatorType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return mediumSpaceIfNotScript( context, tstyle );
}

luPt InnerElementType::getSpaceAfter( RelationType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return thickSpaceIfNotScript( context, tstyle );
}

luPt InnerElementType::getSpaceAfter( PunctuationType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

luPt InnerElementType::getSpaceAfter( BracketType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

luPt InnerElementType::getSpaceAfter( ComplexElementType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}

luPt InnerElementType::getSpaceAfter( InnerElementType*,
                                    const ContextStyle& context,
                                    ContextStyle::TextStyle tstyle )
{
    return thinSpaceIfNotScript( context, tstyle );
}


KFORMULA_NAMESPACE_END
