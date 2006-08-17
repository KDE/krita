/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "FractionElement.h"
#include "RootElement.h"
#include "UnderOverElement.h"
#include "MultiscriptElement.h"
#include "MatrixElement.h"
#include "MatrixRowElement.h"
#include "MatrixEntryElement.h"
#include "SequenceElement.h"
#include "SpaceElement.h"
#include "TextElement.h"
#include "bracketelement.h"


namespace KFormula {

BasicElement* ElementFactory::createElement( const QString& tagName,
                                             BasicElement* parent )
{
    if( tagName == "mrow" )
	return new SequenceElement( parent ); 
/*    else if( tagName == "mi" )
	return new IdentifierElement( parent );
    else if( tagName == "mn" )
	return new NumberElement( parent );
    else if( tagName == "mo" )
	return new OperatorElement( parent );*/
    else if( tagName == "mtext" )
	return new TextElement( parent );
    else if( tagName == "mspace" )
	return new SpaceElement( parent );
/*    else if( tagName == "ms" )
	return new StringElement( parent );
    else if( tagName == "mglyph" )
	return new GlyphElement( parent );*/
    else if( tagName == "mfrac" )
	return new FractionElement( parent );
    else if( tagName == "msqrt" )
	return new RootElement( parent );
    else if( tagName == "mroot" )
	return new RootElement( parent );
/*    else if( tagName == "mstyle" )
	return new StyleElement( parent );
    else if( tagName == "merror" )
	return new ErrorElement( parent );
    else if( tagName == "mpadded" )
	return new PaddedElement( parent );
    else if( tagName == "mphantom" )
	return new PhantomElement( parent );
    else if( tagName == "mfenced" )
	return new FencedElement( parent );
    else if( tagName == "menclose" )
	return new EncloseElement( parent );*/
    else if( tagName == "msub" )
	return new MultiscriptElement( parent );
    else if( tagName == "msup" )
	return new MultiscriptElement( parent );
    else if( tagName == "msubsup" )
	return new MultiscriptElement( parent );    
    else if( tagName == "munder" )
	return new UnderOverElement( parent );
    else if( tagName == "mover" )
	return new UnderOverElement( parent );
    else if( tagName == "munderover" )
	return new UnderOverElement( parent );
    else if( tagName == "mmultiscripts" )
	return new MultiscriptElement( parent );

    return 0;
}

} // namespace KFormula
