/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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

#include "MatrixElement.h"
#include "MatrixRowElement.h"
#include "MatrixEntryElement.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>
#include <QPainter>

namespace KFormula {

MatrixElement::MatrixElement( BasicElement* parent ) : BasicElement( parent )
{
}

MatrixElement::~MatrixElement()
{
}

void MatrixElement::paint( QPainter& painter ) const
{
    // TODO paint the frame, rowlines, columnlines
    foreach( MatrixRowElement* tmpRow, m_matrixRowElements )
        tmpRow->paint( painter );
}

void MatrixElement::calculateSize()
{
    // TODO implement rowspacing
    double tmpHeight = 0.0;
    double tmpWidth = 0.0;
    QPointF tmpOrigin = origin();
    foreach( MatrixRowElement* tmpRow, m_matrixRowElements )
    {
        tmpRow->calculateSize();
        tmpWidth = qMax( tmpRow->width(), tmpWidth );
	tmpHeight += tmpRow->height();
	tmpRow->setOrigin( tmpOrigin );
	tmpOrigin = origin() + QPointF( 0, tmpHeight ); 
    }
    setHeight( tmpHeight );
    setWidth( tmpWidth );
    setBaseLine( tmpHeight/2 + parentElement()->height()/2 );
}

const QList<BasicElement*> MatrixElement::childElements()
{
    QList<BasicElement*> tmp;
    foreach( MatrixRowElement* tmpRow, m_matrixRowElements )
        tmp << tmpRow;
    return tmp;
}

void MatrixElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )
        m_matrixRowElements[ 0 ]->moveRight( cursor, this ); // enter the matrix element
    else
        parentElement()->moveRight( cursor, this );
}

void MatrixElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )
        m_matrixRowElements.last()->moveLeft( cursor, this );// enter the matrix element
    else
	parentElement()->moveLeft( cursor, this );
}

void MatrixElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    if( !childElements().contains( from ) )
        return;

    for( int i = 1; i < m_matrixRowElements.count()-1; i++ ) // end with the forlast
    {                                                        // the last can't move down
        if( m_matrixRowElements[ i ] == from )
            m_matrixRowElements[ i++ ]->moveDown( cursor, this );
    }
}

void MatrixElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    if( !childElements().contains( from ) )
        return;

    for( int i = 1; i < m_matrixRowElements.count(); i++ ) // start with 1 because the
    {                                                      // uppest can't move up
        if( m_matrixRowElements[ i ] == from )
            m_matrixRowElements[ i-- ]->moveUp( cursor, this );
    }
}

void MatrixElement::readMathML( const QDomElement& element )
{
    readMathMLAttributes( element );
   
    MatrixRowElement* tmpElement = 0;
    QDomElement tmp = element.firstChildElement();    // read each mtr element 
    while( !tmp.isNull() )
    {
        tmpElement = new MatrixRowElement( this );
        m_matrixRowElements << tmpElement;
	tmpElement->readMathML( tmp );
	tmp = tmp.nextSiblingElement();
    }
}

void MatrixElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    writer->startElement( oasisFormat ? "math:mtable" : "mtable" );
    writeMathMLAttributes( writer );
    
    foreach( MatrixRowElement* tmpRow, m_matrixRowElements )  // write each mtr element
	tmpRow->writeMathML( writer, oasisFormat );
    
    writer->endElement();
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

/**
 * Sets the cursor inside this element to its start position.
 * For most elements that is the main child.
 */
void MatrixElement::goInside(FormulaCursor* cursor)
{
    matrixEntryAt(0, 0)->goInside(cursor);
}

void MatrixElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
/*    for (int r = 0; r < rows(); r++) {
        for (int c = 0; c < cols(); c++) {
            if (child == matrixEntryAt(r, c))
                cursor->setTo(this, r*cols()+c);
        }
    }*/
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
