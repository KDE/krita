/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
                 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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
#include "AnnotationElement.h"
#include "FencedElement.h"
#include "EncloseElement.h"
#include "ErrorElement.h"
#include "FractionElement.h"
#include "GlyphElement.h"
#include "IdentifierElement.h"
#include "TableElement.h"
#include "TableRowElement.h"
#include "TableDataElement.h"
#include "MultiscriptElement.h"
#include "NumberElement.h"
#include "OperatorElement.h"
#include "PaddedElement.h"
#include "PhantomElement.h"
#include "RootElement.h"
#include "RowElement.h"
#include "SpaceElement.h"
#include "StringElement.h"
#include "StyleElement.h"
#include "SubSupElement.h"
#include "TextElement.h"
#include "UnderOverElement.h"
#include "SquareRootElement.h"
#include "UnknownElement.h"

#include <kdebug.h>

BasicElement* ElementFactory::createElement( const QString& tagName,
                                             BasicElement* parent )
{
//    kWarning( DEBUGID ) << "Creating element: " << tagName;
    if ( tagName == "mi" )
        return new IdentifierElement( parent );
    else if ( tagName == "mo" )
        return new OperatorElement( parent );
    else if ( tagName == "mn" )
        return new NumberElement( parent );
    else if ( tagName == "mtext" )
        return new TextElement( parent );
    else if ( tagName == "ms" )
        return new StringElement( parent );
    else if ( tagName == "mspace" )
        return new SpaceElement( parent );
    else if ( tagName == "mglyph" )
        return new GlyphElement( parent );
    else if ( tagName == "mrow" )
          return new RowElement( parent );
    else if ( tagName == "mfrac" )
          return new FractionElement( parent );
    else if ( tagName == "msqrt" )
          return new SquareRootElement( parent );
    else if ( tagName == "mroot" )
          return new RootElement( parent );
    else if ( tagName == "mstyle" )
          return new StyleElement( parent );
    else if ( tagName == "merror" )
          return new ErrorElement( parent );
    else if ( tagName == "mpadded" )
          return new PaddedElement( parent );
    else if ( tagName == "mphantom" )
          return new PhantomElement( parent );
    else if ( tagName == "mtable" )
          return new TableElement( parent );
    else if ( tagName == "mtr" )
          return new TableRowElement( parent );
    else if ( tagName == "mtd" )
          return new TableDataElement( parent );
    else if ( tagName == "mfenced" )
          return new FencedElement( parent );
    else if ( tagName == "menclose" )
          return new EncloseElement( parent );
    else if ( tagName == "msub")
          return new SubSupElement( parent, SubScript );
    else if ( tagName == "msup")
          return new SubSupElement( parent, SupScript );
    else if ( tagName == "msubsup")
          return new SubSupElement( parent, SubSupScript );
    else if ( tagName == "munder")
          return new UnderOverElement( parent, Under );
    else if ( tagName == "mover")
          return new UnderOverElement( parent, Over );
    else if ( tagName == "munderover")
          return new UnderOverElement( parent, UnderOver );
    else if ( tagName == "mmultiscripts" )
          return new MultiscriptElement( parent );
    else if ( tagName == "annotation" )
          return new AnnotationElement( parent );

    kWarning( DEBUGID ) << "Do not know how to create the following element: " << tagName;
    return new UnknownElement( parent );
}

QString ElementFactory::elementName( ElementType type )
{
    switch ( type ) {
        case Identifier:
            return "mi";
        case Operator:
            return "mo";
        case Number:
            return "mn";
        case Text:
            return "mtext";
        case Glyph:
            return "mglyph";
        case String:
            return "ms";
        case Space:
            return "mspace";
        case Row:
            return "mrow";
        case Fraction:
            return "mfrac";
        case Phantom:
            return "mphantom";
        case Style:
            return "mstyle";
        case Padded:
            return "mpadded";
        case Error:
            return "merror";
        case Fenced:
            return "mfenced";
        case Enclose:
            return "menclose";
        case UnderOver:
            return "munderover";
        case Under:
            return "munder";
        case Over:
            return "mover";
        case SubScript:
            return "msub";
        case SupScript:
            return "msup";
        case SubSupScript:
            return "msubsup";
        case MultiScript:
            return "mmultiscripts";
        case Root:
            return "mroot";
        case SquareRoot:
            return "msqrt";
        case Table:
            return "mtable";
        case TableRow:
            return "mtr"; // TODO: Check for mlabeledtr
        case TableData:
            return "mtd";
        case Action:
            return "maction";
        case Basic:
            return "mrow";
        case Formula:
            return "math";
        case Annotation:
            return "annotation";
        default:
            kWarning( DEBUGID ) << "Invalid element type " << type;
    }

    return QString();
}
