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

#include "MatrixEntryElement.h"
#include "FormulaContainer.h"
#include "kformulacommand.h"
#include "FormulaCursor.h"
#include "spaceelement.h"

#include <klocale.h>

namespace KFormula {

MatrixEntryElement::MatrixEntryElement( BasicElement* parent ) : SequenceElement( parent )
{
  //  tabs.setAutoDelete( false );
}

void MatrixEntryElement::drawInternal()
{
}



void MatrixEntryElement::calcSizes( const ContextStyle& context, ContextStyle::TextStyle tstyle,
                                          ContextStyle::IndexStyle istyle )
{
    tabs.clear();
    SequenceElement::calcSizes( context, tstyle, istyle );
}


void MatrixEntryElement::registerTab( BasicElement* tab )
{
    tabs.append( tab );
}


KCommand* MatrixEntryElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() )
        return 0;

    switch ( *request )
    {
        case req_remove: {
        // Remove this line if its empty.
        // Remove the formula if this line was the only one.
        break;
        }
    case req_addNewline: {
        FormulaCursor* cursor = container->activeCursor();
        return new KFCNewLine( i18n( "Add Newline" ), container, this, cursor->getPos() );
    }
    case req_addTabMark: {
        KFCReplace* command = new KFCReplace( i18n("Add Tabmark"), container );
        SpaceElement* element = new SpaceElement( THIN, true );
        command->addElement( element );
        return command;
    }
    default:
        break;
    }
    return SequenceElement::buildCommand( container, request );
}

const QList<BasicElement*>& MatrixEntryElement::childElements()
{
    return QList<BasicElement*>();
}


KCommand* MatrixEntryElement::input( Container* container, QKeyEvent* event )
{
    int action = event->key();
    //int state = event->state();
    //MoveFlag flag = movementFlag(state);

    switch ( action ) {
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        Request newline( req_addNewline );
        return buildCommand( container, &newline );
    }
    case Qt::Key_Tab: {
        Request r( req_addTabMark );
        return buildCommand( container, &r );
    }
    }
    return SequenceElement::input( container, event );
}


KCommand* MatrixEntryElement::input( Container* container, QChar ch )
{
    int latin1 = ch.toLatin1();
    switch (latin1) {
    case '&': {
        Request r( req_addTabMark );
        return buildCommand( container, &r );
    }
    }
    return SequenceElement::input( container, ch );
}


void MatrixEntryElement::moveTabTo( int i, luPixel pos )
{
/*    BasicElement* marker = tab( i );
    luPixel diff = pos - marker->getX();
    marker->setWidth( marker->getWidth() + diff );

    for ( int p = childPos( marker )+1; p < countChildren(); ++p ) {
        BasicElement* child = childAt( p );
        child->setX( child->getX() + diff );
    }

    setWidth( getWidth()+diff );*/
}


int MatrixEntryElement::tabBefore( int pos )
{
    if ( tabs.isEmpty() ) {
        return -1;
    }
    int tabNum = 0;
    for ( int i=0; i<pos; ++i ) {
        BasicElement* child = childAt( i );
        if ( tabs.at( tabNum ) == child ) {
            if ( tabNum+1 == tabs.count() ) {
                return tabNum;
            }
            ++tabNum;
        }
    }
    return static_cast<int>( tabNum )-1;
}


int MatrixEntryElement::tabPos( int i )
{
/*    if ( i < tabs.count() ) {
        return childPos( tabs.at( i ) );
    }
    return -1;*/
}

void MatrixEntryElement::writeMathML( QDomDocument& doc,
                                            QDomNode& parent, bool oasisFormat )
{
    // parent is required to be a <mtr> tag

    QDomElement tmp = doc.createElement( "TMP" );

    SequenceElement::writeMathML( doc, tmp, oasisFormat );

    /* Now we re-parse the Dom tree, because of the TabMarkers
     * that have no direct representation in MathML but mark the
     * end of a <mtd> tag.
     */

    QDomElement mtd = doc.createElement( oasisFormat ? "math:mtd" : "mtd" );

    // The mrow, if it exists.
    QDomNode n = tmp.firstChild().firstChild();
    while ( !n.isNull() ) {
        // the illegal TabMarkers are children of the mrow, child of tmp.
        if ( n.isElement() && n.toElement().tagName() == "TAB" ) {
            parent.appendChild( mtd );
            mtd = doc.createElement( oasisFormat ? "math:mtd" : "mtd" );
        }
        else {
            mtd.appendChild( n.cloneNode() ); // cloneNode needed?
        }
        n = n.nextSibling();
    }

    parent.appendChild( mtd );
}

} // namespace KFormula
