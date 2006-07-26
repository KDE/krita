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

#include <QPainter>
#include <QList>
#include <QKeyEvent>

#include <kdebug.h>
#include <klocale.h>

#include "MatrixDialog.h"
#include "elementvisitor.h"
#include "FormulaElement.h"
#include "FormulaCursor.h"
#include "FormulaContainer.h"
#include "kformulacommand.h"
#include "MatrixElement.h"
#include "MatrixRowElement.h"
#include "MatrixEntryElement.h"
#include "SequenceElement.h"
#include "spaceelement.h"


namespace KFormula {

MatrixElement::MatrixElement( int rows, int columns, BasicElement* parent ) : BasicElement( parent )
{
// I have to think about how to use matrizes best with the new element list!!
	
//    MatrixRowElement* tmp = 0;
    
//    for( int row = 0; row < rows; row++ )
//    {
//	tmp = new MatrixRowElement( columns, this );
//        m_matrixRowElements.add( tmp );
//    }
}

MatrixElement::~MatrixElement()
{
}

const QList<BasicElement*>& MatrixElement::childElements()
{
    return QList<BasicElement*>();
}

int MatrixElement::rows() const
{
    return m_matrixRowElements.count();
}

int MatrixElement::cols() const
{
    return m_matrixRowElements.first()->numberOfEntries();
}

MatrixEntryElement* MatrixElement::matrixEntryAt( int row, int col )
{
    return m_matrixRowElements[ row ]->entryAtPosition( col );
}

void MatrixElement::drawInternal()
{
}

void MatrixElement::writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mtable" : "mtable" );
    QDomElement row;
    QDomElement cell;

    for ( int r = 0; r < rows(); r++ )
    {
        row = doc.createElement( oasisFormat ? "math:mtr" : "mtr" );
        de.appendChild( row );
        for ( int c = 0; c < cols(); c++ )
        {
            cell = doc.createElement( oasisFormat ? "math:mtd" : "mtd" );
            row.appendChild( cell );
    	    matrixEntryAt( r, c )->writeMathML( doc, cell, oasisFormat );
	}
    }

    parent.appendChild( de );
}



/*
void MatrixElement::entered( SequenceElement* child )
{
    formula()->tell( i18n( "Matrix element" ) );
}
*/
void MatrixElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    QVector<luPixel> toMidlines( rows() );
    QVector<luPixel> fromMidlines( rows() );
    QVector<luPixel> widths( cols() );

    toMidlines.fill(0);
    fromMidlines.fill(0);
    widths.fill(0);

    int tmpRows = rows();
    int tmpCols = cols();
    
    ContextStyle::TextStyle i_tstyle = style.convertTextStyleFraction(tstyle);
    ContextStyle::IndexStyle i_istyle = style.convertIndexStyleUpper(istyle);

/*    for (int r = 0; r < rows(); r++) {
        QList<MatrixSequenceElement*>* list = content.at(r);
        for (int c = 0; c < cols(); c++) {
            SequenceElement* element = list->at(c);
            element->calcSizes( style, i_tstyle, i_istyle );
            toMidlines[r] = qMax(toMidlines[r], element->axis( style, i_tstyle ));
            fromMidlines[r] = qMax(fromMidlines[r],
                                   element->getHeight()-element->axis( style, i_tstyle ));
            widths[c] = qMax(widths[c], element->getWidth());
        }
    }*/

    luPixel distX = style.ptToPixelX( style.getThinSpace( tstyle ) );
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );

    luPixel yPos = 0;
/*    for (int r = 0; r < tmpRows; r++) {
        QList<MatrixSequenceElement*>* list = content.at(r);
        luPixel xPos = 0;
        yPos += toMidlines[r];
        for (int c = 0; c < tmpCols; c++) {
            SequenceElement* element = list->at(c);
            switch (style.getMatrixAlignment()) {
            case ContextStyle::left:
                element->setX(xPos);
                break;
            case ContextStyle::center:
                element->setX(xPos + (widths[c] - element->getWidth())/2);
                break;
            case ContextStyle::right:
                element->setX(xPos + widths[c] - element->getWidth());
                break;
            }
            element->setY(yPos - element->axis( style, i_tstyle ));
            xPos += widths[c] + distX;
        }
        yPos += fromMidlines[r] + distY;
    }*/

    luPixel width = distX * (tmpCols - 1);
    luPixel height = distY * (tmpRows - 1);

    for (int r = 0; r < tmpRows; r++) height += toMidlines[r] + fromMidlines[r];
    for (int c = 0; c < tmpCols; c++) width += widths[c];

    setWidth(width);
    setHeight(height);
    if ((tmpRows == 2) && (tmpCols == 1)) {
        setBaseline( getMainChild()->getHeight() + distY / 2 + style.axisHeight( tstyle ) );
    }
    else {
        setBaseline( height/2 + style.axisHeight( tstyle ) );
    }
}


void MatrixElement::draw( QPainter& painter, const LuPixelRect& rect,
                          const ContextStyle& style,
                          ContextStyle::TextStyle tstyle,
                          ContextStyle::IndexStyle istyle,
                          const LuPixelPoint& parentOrigin )
{
/*    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( rect ) )
    //    return;

    int rows = getRows();
    int columns = getColumns();

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < columns; c++) {
            getElement(r, c)->draw(painter, rect, style,
				   style.convertTextStyleFraction(tstyle),
				   style.convertIndexStyleUpper(istyle),
				   myPos);
        }
    }

    // Debug
    //painter.setPen(Qt::red);
    //painter.drawRect(myPos.x(), myPos.y(), getWidth(), getHeight());*/
}


void MatrixElement::dispatchFontCommand( FontCommand* cmd )
{
    for (int r = 0; r < rows(); r++) {
        for (int c = 0; c < cols(); c++) {
            matrixEntryAt( r, c )->dispatchFontCommand( cmd );
        }
    }
}

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void MatrixElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if( cursor->isSelectionMode() )
        getParent()->moveLeft(cursor, this);
    else
    {
        if (from == getParent())
        	matrixEntryAt( rows()-1, cols()-1 )->moveLeft( cursor, this );
        else
       	{
            bool linear = cursor->getLinearMovement();
            int row = 0;
            int column = 0;
            if( searchElement( from, row, column ) )
	    {
                if (column > 0)
                    matrixEntryAt( row, column-1 )->moveLeft( cursor, this );
                else if( linear && ( row > 0 ) ) 
                    matrixEntryAt( row-1, cols()-1 )->moveLeft( cursor, this );
                else 
                    getParent()->moveLeft(cursor, this);
            }
            else 
                getParent()->moveLeft(cursor, this);
        }
    }
}

/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void MatrixElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        if (from == getParent()) {
            matrixEntryAt(0, 0)->moveRight(cursor, this);
        }
        else {
            bool linear = cursor->getLinearMovement();
            int row = 0;
            int column = 0;
            if (searchElement(from, row, column)) {
                if( column < cols()-1 ) {
                    matrixEntryAt( row, column+1 )->moveRight(cursor, this);
                }
                else if ( linear && ( row < rows()-1) )
                    matrixEntryAt( row+1, 0 )->moveRight(cursor, this);
                else {
                    getParent()->moveRight(cursor, this);
                }
            }
            else {
                getParent()->moveRight(cursor, this);
            }
        }
    }
}

/**
 * Enters this element while moving up starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or above it.
 */
void MatrixElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveUp(cursor, this);
    }
    else {
        if (from == getParent()) {
            matrixEntryAt(0, 0)->moveRight(cursor, this);
        }
        else {
            int row = 0;
            int column = 0;
            if (searchElement(from, row, column)) {
                if (row > 0) {
                    matrixEntryAt(row-1, column)->moveRight(cursor, this);
                }
                else {
                    getParent()->moveUp(cursor, this);
                }
            }
            else {
                getParent()->moveUp(cursor, this);
            }
        }
    }
}

/**
 * Enters this element while moving down starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or below it.
 */
void MatrixElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveDown(cursor, this);
    }
    else {
        if (from == getParent()) {
            matrixEntryAt(0, 0)->moveRight(cursor, this);
        }
        else {
            int row = 0;
            int column = 0;
            if (searchElement(from, row, column)) {
                if (row < rows()-1) {
                    matrixEntryAt(row+1, column)->moveRight(cursor, this);
                }
                else {
                    getParent()->moveDown(cursor, this);
                }
            }
            else {
                getParent()->moveDown(cursor, this);
            }
        }
    }
}

/**
 * Sets the cursor inside this element to its start position.
 * For most elements that is the main child.
 */
void MatrixElement::goInside(FormulaCursor* cursor)
{
    matrixEntryAt(0, 0)->goInside(cursor);
}


// If there is a main child we must provide the insert/remove semantics.
SequenceElement* MatrixElement::getMainChild()
{
  // only temporary
  return 0;//    return matrixEntryAt( 0, 0 );
}

void MatrixElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    for (int r = 0; r < rows(); r++) {
        for (int c = 0; c < cols(); c++) {
            if (child == matrixEntryAt(r, c))
                cursor->setTo(this, r*cols()+c);
        }
    }
}

bool MatrixElement::searchElement(BasicElement* element, int& row, int& column)
{
    for (int r = 0; r < rows(); r++) {
        for (int c = 0; c < cols(); c++) {
            if (element == matrixEntryAt(r, c)) {
                row = r;
                column = c;
                return true;
            }
        }
    }
    return false;
}


/**
 * Appends our attributes to the dom element.
 */
void MatrixElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    element.setAttribute( "ROWS", rows() );
    element.setAttribute( "COLUMNS", cols() );

    QDomDocument doc = element.ownerDocument();

    for (int r = 0; r < rows(); r++) {
        for (int c = 0; c < cols(); c++) {
    	    QDomElement tmp = matrixEntryAt( r, c )->getElementDom(doc);
            element.appendChild( tmp );
	}
        element.appendChild( doc.createComment( "end of row" ) );
    }
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool MatrixElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    int rows = 0;
    QString rowStr = element.attribute("ROWS");
    if(!rowStr.isNull()) {
        rows = rowStr.toInt();
    }
    if (rows == 0) {
        kWarning( DEBUGID ) << "Rows <= 0 in MatrixElement." << endl;
        return false;
    }

    QString columnStr = element.attribute("COLUMNS");
    int cols = 0;
    if(!columnStr.isNull()) {
        cols = columnStr.toInt();
    }
    if (cols == 0) {
        kWarning( DEBUGID ) << "Columns <= 0 in MatrixElement." << endl;
        return false;
    }
/*
    content.clear();
    for (int r = 0; r < rows; r++) {
        QList<MatrixSequenceElement*>* list = new QList<MatrixSequenceElement*>;
//        list->setAutoDelete(true);
        content.append(list);
        for (int c = 0; c < cols; c++) {
            MatrixSequenceElement* element = new MatrixSequenceElement(this);
            list->append(element);
	}
    }*/
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool MatrixElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node))
        return false;

    int r = 0;
    int c = 0;
    while ( !node.isNull() && r < rows() ) {
        if ( node.isElement() ) {
            SequenceElement* element = matrixEntryAt( r, c );
            QDomElement e = node.toElement();
            if ( !element->buildFromDom( e ) ) {
                return false;
            }
            c++;
            if ( c == cols() ) {
                c = 0;
                r++;
            }
        }
        node = node.nextSibling();
    }
    return true;
}



/*
class MatrixSequenceElement : public SequenceElement
{
    typedef SequenceElement inherited;
  public:

    MatrixSequenceElement( BasicElement* parent = 0 ) : SequenceElement( parent ) {}
    virtual MatrixSequenceElement* clone() {
        return new MatrixSequenceElement( *this );
    }
*/
    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
/*    virtual KCommand* buildCommand( Container*, Request* );
};
*/


/*
KCommand* MatrixSequenceElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        return 0;
    }

    switch ( *request ) {
    case req_appendColumn:
    case req_appendRow:
    case req_insertColumn:
    case req_removeColumn:
    case req_insertRow:
    case req_removeRow: {
        MatrixElement* matrix = static_cast<MatrixElement*>( getParent() );
        FormulaCursor* cursor = container->activeCursor();
        for ( int row = 0; row < matrix->getRows(); row++ ) {
            for ( int col = 0; col < matrix->getColumns(); col++ ) {
                if ( matrix->getElement( row, col ) == cursor->getElement() ) {
                    switch ( *request ) {
                    case req_appendColumn:
                        return new KFCInsertColumn( i18n( "Append Column" ), container, matrix, row, matrix->getColumns() );
                    case req_appendRow:
                        return new KFCInsertRow( i18n( "Append Row" ), container, matrix, matrix->getRows(), col );
                    case req_insertColumn:
                        return new KFCInsertColumn( i18n( "Insert Column" ), container, matrix, row, col );
                    case req_removeColumn:
                        if ( matrix->getColumns() > 1 ) {
                            return new KFCRemoveColumn( i18n( "Remove Column" ), container, matrix, row, col );
                        }
                        break;
                    case req_insertRow:
                        return new KFCInsertRow( i18n( "Insert Row" ), container, matrix, row, col );
                    case req_removeRow:
                        if ( matrix->getRows() > 1 ) {
                            return new KFCRemoveRow( i18n( "Remove Row" ), container, matrix, row, col );
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        kWarning( DEBUGID ) << "MatrixSequenceElement::buildCommand: Sequence not found." << endl;
        break;
    }
    default:
        break;
    }
    return inherited::buildCommand( container, request );
}
*/



} // namespace KFormula
