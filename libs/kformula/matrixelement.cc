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
#include "formulaelement.h"
#include "formulacursor.h"
#include "kformulacontainer.h"
#include "kformulacommand.h"
#include "matrixelement.h"
#include "sequenceelement.h"
#include "spaceelement.h"


KFORMULA_NAMESPACE_BEGIN


class MatrixSequenceElement : public SequenceElement
{
    typedef SequenceElement inherited;
  public:

    MatrixSequenceElement( BasicElement* parent = 0 ) : SequenceElement( parent ) {}
    virtual MatrixSequenceElement* clone() {
        return new MatrixSequenceElement( *this );
    }

    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
    virtual KCommand* buildCommand( Container*, Request* );
};


class KFCRemoveRow : public Command
{
  public:
    KFCRemoveRow( const QString& name, Container* document, MatrixElement* m, int r, int c );
    ~KFCRemoveRow();

    virtual void execute();
    virtual void unexecute();

  protected:
    MatrixElement* matrix;
    int rowPos;
    int colPos;

    QList<MatrixSequenceElement*>* row;
};


class KFCInsertRow : public KFCRemoveRow {
public:
    KFCInsertRow( const QString& name, Container* document, MatrixElement* m, int r, int c );

    virtual void execute()   { KFCRemoveRow::unexecute(); }
    virtual void unexecute() { KFCRemoveRow::execute(); }
};


class KFCRemoveColumn : public Command {
public:
    KFCRemoveColumn( const QString& name, Container* document, MatrixElement* m, int r, int c );
    ~KFCRemoveColumn();

    virtual void execute();
    virtual void unexecute();

protected:
    MatrixElement* matrix;
    int rowPos;
    int colPos;

    QList<MatrixSequenceElement*>* column;
};


class KFCInsertColumn : public KFCRemoveColumn {
public:
    KFCInsertColumn( const QString& name, Container* document, MatrixElement* m, int r, int c );

    virtual void execute()   { KFCRemoveColumn::unexecute(); }
    virtual void unexecute() { KFCRemoveColumn::execute(); }
};


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


KFCRemoveRow::KFCRemoveRow( const QString& name, Container* document, MatrixElement* m, int r, int c )
    : Command( name, document ), matrix( m ), rowPos( r ), colPos( c ), row( 0 )
{
}

KFCRemoveRow::~KFCRemoveRow()
{
    delete row;
}

void KFCRemoveRow::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    row = matrix->content.at( rowPos );
    FormulaElement* formula = matrix->formula();
    for ( int i = matrix->getColumns(); i > 0; i-- ) {
        formula->elementRemoval( row->at( i-1 ) );
    }
    matrix->content.takeAt( rowPos );
    formula->changed();
    if ( rowPos < matrix->getRows() ) {
        matrix->getElement( rowPos, colPos )->goInside( cursor );
    }
    else {
        matrix->getElement( rowPos-1, colPos )->goInside( cursor );
    }
    testDirty();
}

void KFCRemoveRow::unexecute()
{
    matrix->content.insert( rowPos, row );
    row = 0;
    FormulaCursor* cursor = getExecuteCursor();
    matrix->getElement( rowPos, colPos )->goInside( cursor );
    matrix->formula()->changed();
    testDirty();
}


KFCInsertRow::KFCInsertRow( const QString& name, Container* document, MatrixElement* m, int r, int c )
    : KFCRemoveRow( name, document, m, r, c )
{
    row = new QList<MatrixSequenceElement*>;
//    row->setAutoDelete( true );
    for ( int i = 0; i < matrix->getColumns(); i++ ) {
        row->append( new MatrixSequenceElement( matrix ) );
    }
}


KFCRemoveColumn::KFCRemoveColumn( const QString& name, Container* document, MatrixElement* m, int r, int c )
    : Command( name, document ), matrix( m ), rowPos( r ), colPos( c )
{
    column = new QList<MatrixSequenceElement*>;
//    column->setAutoDelete( true );
}

KFCRemoveColumn::~KFCRemoveColumn()
{
    delete column;
}

void KFCRemoveColumn::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    FormulaElement* formula = matrix->formula();
    for ( int i = 0; i < matrix->getRows(); i++ ) {
        column->append( matrix->getElement( i, colPos ) );
        formula->elementRemoval( column->at( i ) );
        matrix->content.at( i )->takeAt( colPos );
    }
    formula->changed();
    if ( colPos < matrix->getColumns() ) {
        matrix->getElement( rowPos, colPos )->goInside( cursor );
    }
    else {
        matrix->getElement( rowPos, colPos-1 )->goInside( cursor );
    }
    testDirty();
}

void KFCRemoveColumn::unexecute()
{
    for ( int i = 0; i < matrix->getRows(); i++ ) {
        matrix->content.at( i )->insert( colPos, column->takeAt( 0 ) );
    }
    FormulaCursor* cursor = getExecuteCursor();
    matrix->getElement( rowPos, colPos )->goInside( cursor );
    matrix->formula()->changed();
    testDirty();
}


KFCInsertColumn::KFCInsertColumn( const QString& name, Container* document, MatrixElement* m, int r, int c )
    : KFCRemoveColumn( name, document, m, r, c )
{
    for ( int i = 0; i < matrix->getRows(); i++ ) {
        column->append( new MatrixSequenceElement( matrix ) );
    }
}


MatrixElement::MatrixElement( int rows, int columns, BasicElement* parent)
    : BasicElement(parent)
{
    for ( int r = 0; r < rows; r++) {
        QList<MatrixSequenceElement*>* list = new QList<MatrixSequenceElement*>;
//        list->setAutoDelete(true);
        for ( int c = 0; c < columns; c++) {
            list->append(new MatrixSequenceElement(this));
        }
        content.append(list);
    }
//    content.setAutoDelete(true);
}

MatrixElement::~MatrixElement()
{
}


MatrixElement::MatrixElement( const MatrixElement& other )
    : BasicElement( other )
{
    foreach( QList<MatrixSequenceElement*>* tmp, other.content )
    {
        QList<MatrixSequenceElement*>* list = new QList<MatrixSequenceElement*>;
//        list->setAutoDelete(true);
        foreach( MatrixSequenceElement* tmpCol, *tmp )
	{
            MatrixSequenceElement *mse = new MatrixSequenceElement( *tmpCol );
            list->append( mse );
            mse->setParent( this );
        }
        content.append(list);
    }
//    content.setAutoDelete(true);
}


bool MatrixElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void MatrixElement::entered( SequenceElement* /*child*/ )
{
    formula()->tell( i18n( "Matrix element" ) );
}


BasicElement* MatrixElement::goToPos( FormulaCursor* cursor, bool& handled,
                                      const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());

        int rows = getRows();
        int columns = getColumns();

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < columns; c++) {
                BasicElement* element = getElement(r, c);
                e = element->goToPos(cursor, handled, point, myPos);
                if (e != 0) {
                    return e;
                }
            }
        }

        // We are in one of those gaps.
        luPixel dx = point.x() - myPos.x();
        luPixel dy = point.y() - myPos.y();

        int row = rows;
        for (int r = 0; r < rows; r++) {
            BasicElement* element = getElement(r, 0);
            if (element->getY() > dy) {
                row = r;
                break;
            }
        }
        if (row == 0) {
            BasicElement* element = getParent();
            element->moveLeft(cursor, this);
            handled = true;
            return element;
        }
        row--;

        int column = columns;
        for (int c = 0; c < columns; c++) {
            BasicElement* element = getElement(row, c);
            if (element->getX() > dx) {
                column = c;
                break;
            }
        }
        if (column == 0) {
            BasicElement* element = getParent();
            element->moveLeft(cursor, this);
            handled = true;
            return element;
        }
        column--;

        // Rescan the rows with the actual colums required.
        row = rows;
        for (int r = 0; r < rows; r++) {
            BasicElement* element = getElement(r, column);
            if (element->getY() > dy) {
                row = r;
                break;
            }
        }
        if (row == 0) {
            BasicElement* element = getParent();
            element->moveLeft(cursor, this);
            handled = true;
            return element;
        }
        row--;

        BasicElement* element = getElement(row, column);
        element->moveLeft(cursor, this);
        handled = true;
        return element;
    }
    return 0;
}


// drawing
//
// Drawing depends on a context which knows the required properties like
// fonts, spaces and such.
// It is essential to calculate elements size with the same context
// before you draw.

/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void MatrixElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    QVector<luPixel> toMidlines(getRows());
    QVector<luPixel> fromMidlines(getRows());
    QVector<luPixel> widths(getColumns());

    toMidlines.fill(0);
    fromMidlines.fill(0);
    widths.fill(0);

    int rows = getRows();
    int columns = getColumns();

    ContextStyle::TextStyle i_tstyle = style.convertTextStyleFraction(tstyle);
    ContextStyle::IndexStyle i_istyle = style.convertIndexStyleUpper(istyle);

    for (int r = 0; r < rows; r++) {
        QList<MatrixSequenceElement*>* list = content.at(r);
        for (int c = 0; c < columns; c++) {
            SequenceElement* element = list->at(c);
            element->calcSizes( style, i_tstyle, i_istyle );
            toMidlines[r] = qMax(toMidlines[r], element->axis( style, i_tstyle ));
            fromMidlines[r] = qMax(fromMidlines[r],
                                   element->getHeight()-element->axis( style, i_tstyle ));
            widths[c] = qMax(widths[c], element->getWidth());
        }
    }

    luPixel distX = style.ptToPixelX( style.getThinSpace( tstyle ) );
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );

    luPixel yPos = 0;
    for (int r = 0; r < rows; r++) {
        QList<MatrixSequenceElement*>* list = content.at(r);
        luPixel xPos = 0;
        yPos += toMidlines[r];
        for (int c = 0; c < columns; c++) {
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
    }

    luPixel width = distX * (columns - 1);
    luPixel height = distY * (rows - 1);

    for (int r = 0; r < rows; r++) height += toMidlines[r] + fromMidlines[r];
    for (int c = 0; c < columns; c++) width += widths[c];

    setWidth(width);
    setHeight(height);
    if ((rows == 2) && (columns == 1)) {
        setBaseline( getMainChild()->getHeight() + distY / 2 + style.axisHeight( tstyle ) );
    }
    else {
        setBaseline( height/2 + style.axisHeight( tstyle ) );
    }
}

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void MatrixElement::draw( QPainter& painter, const LuPixelRect& rect,
                          const ContextStyle& style,
                          ContextStyle::TextStyle tstyle,
                          ContextStyle::IndexStyle istyle,
                          const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
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
    //painter.drawRect(myPos.x(), myPos.y(), getWidth(), getHeight());
}


void MatrixElement::dispatchFontCommand( FontCommand* cmd )
{
    int rows = getRows();
    int columns = getColumns();

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < columns; c++) {
            getElement(r, c)->dispatchFontCommand( cmd );
        }
    }
}


// navigation
//
// The elements are responsible to handle cursor movement themselves.
// To do this they need to know the direction the cursor moves and
// the element it comes from.
//
// The cursor might be in normal or in selection mode.

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void MatrixElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        if (from == getParent()) {
            getElement(getRows()-1, getColumns()-1)->moveLeft(cursor, this);
        }
        else {
            bool linear = cursor->getLinearMovement();
            int row = 0;
            int column = 0;
            if (searchElement(from, row, column)) {
                if (column > 0) {
                    getElement(row, column-1)->moveLeft(cursor, this);
                }
                else if (linear && (row > 0)) {
                    getElement(row-1, getColumns()-1)->moveLeft(cursor, this);
                }
                else {
                    getParent()->moveLeft(cursor, this);
                }
            }
            else {
                getParent()->moveLeft(cursor, this);
            }
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
            getElement(0, 0)->moveRight(cursor, this);
        }
        else {
            bool linear = cursor->getLinearMovement();
            int row = 0;
            int column = 0;
            if (searchElement(from, row, column)) {
                if (column < getColumns()-1) {
                    getElement(row, column+1)->moveRight(cursor, this);
                }
                else if (linear && (row < getRows()-1)) {
                    getElement(row+1, 0)->moveRight(cursor, this);
                }
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
            getElement(0, 0)->moveRight(cursor, this);
        }
        else {
            int row = 0;
            int column = 0;
            if (searchElement(from, row, column)) {
                if (row > 0) {
                    getElement(row-1, column)->moveRight(cursor, this);
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
            getElement(0, 0)->moveRight(cursor, this);
        }
        else {
            int row = 0;
            int column = 0;
            if (searchElement(from, row, column)) {
                if (row < getRows()-1) {
                    getElement(row+1, column)->moveRight(cursor, this);
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
    getElement(0, 0)->goInside(cursor);
}


// If there is a main child we must provide the insert/remove semantics.
SequenceElement* MatrixElement::getMainChild()
{
    return content.at(0)->at(0);
}

void MatrixElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    int rows = getRows();
    int columns = getColumns();
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < columns; c++) {
            if (child == getElement(r, c)) {
                cursor->setTo(this, r*columns+c);
            }
        }
    }
}

bool MatrixElement::searchElement(BasicElement* element, int& row, int& column)
{
    int rows = getRows();
    int columns = getColumns();
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < columns; c++) {
            if (element == getElement(r, c)) {
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

    int rows = getRows();
    int cols = getColumns();

    element.setAttribute("ROWS", rows);
    element.setAttribute("COLUMNS", cols);

    QDomDocument doc = element.ownerDocument();

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
    	    QDomElement tmp = getElement(r,c)->getElementDom(doc);
            element.appendChild(tmp);
	}
        element.appendChild(doc.createComment("end of row"));
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

    content.clear();
    for (int r = 0; r < rows; r++) {
        QList<MatrixSequenceElement*>* list = new QList<MatrixSequenceElement*>;
//        list->setAutoDelete(true);
        content.append(list);
        for (int c = 0; c < cols; c++) {
            MatrixSequenceElement* element = new MatrixSequenceElement(this);
            list->append(element);
	}
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool MatrixElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    int rows = getRows();
    int cols = getColumns();

    int r = 0;
    int c = 0;
    while ( !node.isNull() && r < rows ) {
        if ( node.isElement() ) {
            SequenceElement* element = getElement( r, c );
            QDomElement e = node.toElement();
            if ( !element->buildFromDom( e ) ) {
                return false;
            }
            c++;
            if ( c == cols ) {
                c = 0;
                r++;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString MatrixElement::toLatex()
{
    //All the border handling must be implemented here too

    QString matrix;
    uint cols=getColumns();
    uint rows=getRows();

    matrix="\\begin{array}{ ";
    for(uint i=0;i<cols;i++)
	matrix+="c ";

    matrix+="} ";

    for (uint r = 0; r < rows; r++) {
        for (uint c = 0; c < cols; c++) {
            matrix+=getElement(r, c)->toLatex();
	    if( c < cols-1)    matrix+=" & ";
        }
    	if(r < rows-1 ) matrix+=" \\\\ ";
    }

    matrix+=" \\end{array}";

    return matrix;
}

QString MatrixElement::formulaString()
{
    QString matrix = "[";
    uint cols=getColumns();
    uint rows=getRows();
    for (uint r = 0; r < rows; r++) {
        matrix += "[";
        for (uint c = 0; c < cols; c++) {
            matrix+=getElement(r, c)->formulaString();
	    if ( c < cols-1 ) matrix+=", ";
        }
        matrix += "]";
    	if ( r < rows-1 ) matrix += ", ";
    }
    matrix += "]";
    return matrix;
}


SequenceElement* MatrixElement::elementAt(int row, int column)
{
    return getElement( row, column );
}


void MatrixElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mtable" : "mtable" );
    QDomElement row;
    QDomElement cell;

    int rows = getRows();
    int cols = getColumns();

    for ( int r = 0; r < rows; r++ )
    {
        row = doc.createElement( oasisFormat ? "math:mtr" : "mtr" );
        de.appendChild( row );
        for ( int c = 0; c < cols; c++ )
        {
            cell = doc.createElement( oasisFormat ? "math:mtd" : "mtd" );
            row.appendChild( cell );
    	    getElement(r,c)->writeMathML( doc, cell, oasisFormat );
	}
    }

    parent.appendChild( de );
}


//////////////////////////////////////////////////////////////////////////////


/**
 * The lines behaviour is (a little) different from that
 * of ordinary sequences.
 */
class MultilineSequenceElement : public SequenceElement {
    typedef SequenceElement inherited;
public:

    MultilineSequenceElement( BasicElement* parent = 0 );

    virtual MultilineSequenceElement* clone() {
        return new MultilineSequenceElement( *this );
    }

    virtual BasicElement* goToPos( FormulaCursor*, bool& handled,
                                   const LuPixelPoint& point, const LuPixelPoint& parentOrigin );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle );

    virtual void registerTab( BasicElement* tab );

    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
    virtual KCommand* buildCommand( Container*, Request* );

    virtual KCommand* input( Container* container, QKeyEvent* event );

    virtual KCommand* input( Container* container, QChar ch );

    int tabCount() const { return tabs.count(); }

    BasicElement* tab( int i ) { return tabs.at( i ); }

    /// Change the width of tab i and move all elements after it.
    void moveTabTo( int i, luPixel pos );

    /// Return the greatest tab number less than pos.
    int tabBefore( int pos );

    /// Return the position of tab i.
    int tabPos( int i );

    virtual void writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat = false );

private:

    QList<BasicElement*> tabs;
};


// Split the line at position pos.
class KFCNewLine : public Command {
public:
    KFCNewLine( const QString& name, Container* document,
                MultilineSequenceElement* line, uint pos );

    virtual ~KFCNewLine();

    virtual void execute();
    virtual void unexecute();

private:
    MultilineSequenceElement* m_line;
    MultilineSequenceElement* m_newline;
    uint m_pos;
};


KFCNewLine::KFCNewLine( const QString& name, Container* document,
                        MultilineSequenceElement* line, uint pos )
    : Command( name, document ),
      m_line( line ), m_pos( pos )
{
    m_newline = new MultilineSequenceElement( m_line->getParent() );
}


KFCNewLine::~KFCNewLine()
{
    delete m_newline;
}


void KFCNewLine::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    MultilineElement* parent = static_cast<MultilineElement*>( m_line->getParent() );
    int linePos = parent->content.indexOf( m_line );
    parent->content.insert( linePos+1, m_newline );

    // If there are children to be moved.
    if ( m_line->countChildren() > static_cast<int>( m_pos ) ) {

        // Remove anything after position pos from the current line
        m_line->selectAllChildren( cursor );
        cursor->setMark( m_pos );
        QList<BasicElement*> elementList;
        m_line->remove( cursor, elementList, beforeCursor );

        // Insert the removed stuff into the new line
        m_newline->goInside( cursor );
        m_newline->insert( cursor, elementList, beforeCursor );
        cursor->setPos( cursor->getMark() );
    }
    else {
        m_newline->goInside( cursor );
    }

    // The command no longer owns the new line.
    m_newline = 0;

    // Tell that something changed
    FormulaElement* formula = m_line->formula();
    formula->changed();
    testDirty();
}


void KFCNewLine::unexecute()
{
    FormulaCursor* cursor = getExecuteCursor();
    MultilineElement* parent = static_cast<MultilineElement*>( m_line->getParent() );
    int linePos = parent->content.indexOf( m_line );

    // Now the command owns the new line again.
    m_newline = parent->content.at( linePos+1 );

    // Tell all cursors to leave this sequence
    FormulaElement* formula = m_line->formula();
    formula->elementRemoval( m_newline );

    // If there are children to be moved.
    if ( m_newline->countChildren() > 0 ) {

        // Remove anything from the line to be deleted
        m_newline->selectAllChildren( cursor );
        QList<BasicElement*> elementList;
        m_newline->remove( cursor, elementList, beforeCursor );

        // Insert the removed stuff into the previous line
        m_line->moveEnd( cursor );
        m_line->insert( cursor, elementList, beforeCursor );
        cursor->setPos( cursor->getMark() );
    }
    else {
        m_line->moveEnd( cursor );
    }
    parent->content.takeAt( linePos+1 );

    // Tell that something changed
    formula->changed();
    testDirty();
}


MultilineSequenceElement::MultilineSequenceElement( BasicElement* parent )
    : SequenceElement( parent )
{
  //  tabs.setAutoDelete( false );
}


BasicElement* MultilineSequenceElement::goToPos( FormulaCursor* cursor, bool& handled,
                                                 const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    //LuPixelPoint myPos(parentOrigin.x() + getX(),
    //                   parentOrigin.y() + getY());
    BasicElement* e = inherited::goToPos(cursor, handled, point, parentOrigin);

    if (e == 0) {
        // If the mouse was behind this line put the cursor to the last position.
        if ( ( point.x() > getX()+getWidth() ) &&
             ( point.y() >= getY() ) &&
             ( point.y() < getY()+getHeight() ) ) {
            cursor->setTo(this, countChildren());
            handled = true;
            return this;
        }
    }
    return e;
}


void MultilineSequenceElement::calcSizes( const ContextStyle& context,
                                          ContextStyle::TextStyle tstyle,
                                          ContextStyle::IndexStyle istyle )
{
    tabs.clear();
    inherited::calcSizes( context, tstyle, istyle );
}


void MultilineSequenceElement::registerTab( BasicElement* tab )
{
    tabs.append( tab );
}


KCommand* MultilineSequenceElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        return 0;
    }

    switch ( *request ) {
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
    return inherited::buildCommand( container, request );
}


KCommand* MultilineSequenceElement::input( Container* container, QKeyEvent* event )
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
    return inherited::input( container, event );
}


KCommand* MultilineSequenceElement::input( Container* container, QChar ch )
{
    int latin1 = ch.toLatin1();
    switch (latin1) {
    case '&': {
        Request r( req_addTabMark );
        return buildCommand( container, &r );
    }
    }
    return inherited::input( container, ch );
}


void MultilineSequenceElement::moveTabTo( int i, luPixel pos )
{
    BasicElement* marker = tab( i );
    luPixel diff = pos - marker->getX();
    marker->setWidth( marker->getWidth() + diff );

    for ( int p = childPos( marker )+1; p < countChildren(); ++p ) {
        BasicElement* child = getChild( p );
        child->setX( child->getX() + diff );
    }

    setWidth( getWidth()+diff );
}


int MultilineSequenceElement::tabBefore( int pos )
{
    if ( tabs.isEmpty() ) {
        return -1;
    }
    int tabNum = 0;
    for ( int i=0; i<pos; ++i ) {
        BasicElement* child = getChild( i );
        if ( tabs.at( tabNum ) == child ) {
            if ( tabNum+1 == tabs.count() ) {
                return tabNum;
            }
            ++tabNum;
        }
    }
    return static_cast<int>( tabNum )-1;
}


int MultilineSequenceElement::tabPos( int i )
{
    if ( i < tabs.count() ) {
        return childPos( tabs.at( i ) );
    }
    return -1;
}


void MultilineSequenceElement::writeMathML( QDomDocument& doc,
                                            QDomNode parent, bool oasisFormat )
{
    // parent is required to be a <mtr> tag

    QDomElement tmp = doc.createElement( "TMP" );

    inherited::writeMathML( doc, tmp, oasisFormat );

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


MultilineElement::MultilineElement( BasicElement* parent )
    : BasicElement( parent )
{
//    content.setAutoDelete( true );
    content.append( new MultilineSequenceElement( this ) );
}

MultilineElement::~MultilineElement()
{
}

MultilineElement::MultilineElement( const MultilineElement& other )
    : BasicElement( other )
{
//    content.setAutoDelete( true );
    int count = other.content.count();
    for (int i = 0; i < count; i++) {
        MultilineSequenceElement* line = content.at(i)->clone();
        line->setParent( this );
        content.append( line );
    }
}


bool MultilineElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void MultilineElement::entered( SequenceElement* /*child*/ )
{
    formula()->tell( i18n( "Multi line element" ) );
}


/**
 * Returns the element the point is in.
 */
BasicElement* MultilineElement::goToPos( FormulaCursor* cursor, bool& handled,
                                         const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = inherited::goToPos(cursor, handled, point, parentOrigin);
    if ( e != 0 ) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());

        uint count = content.count();
        for ( uint i = 0; i < count; ++i ) {
            MultilineSequenceElement* line = content.at(i);
            e = line->goToPos(cursor, handled, point, myPos);
            if (e != 0) {
                return e;
            }
        }
        return this;
    }
    return 0;
}

void MultilineElement::goInside( FormulaCursor* cursor )
{
    content.at( 0 )->goInside( cursor );
}

void MultilineElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    // If you want to select more than one line you'll have to
    // select the whole element.
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        // Coming from the parent (sequence) we go to
        // the very last position
        if (from == getParent()) {
            content.at( content.count()-1 )->moveLeft(cursor, this);
        }
        else {
            // Coming from one of the lines we go to the previous line
            // or to the parent if there is none.
            int pos = content.indexOf( static_cast<MultilineSequenceElement*>( from ) );
            if ( pos > -1 ) {
                if ( pos > 0 ) {
                    content.at( pos-1 )->moveLeft( cursor, this );
                }
                else {
                    getParent()->moveLeft(cursor, this);
                }
            }
            else {
                kDebug( DEBUGID ) << k_funcinfo << endl;
                kDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
            }
        }
    }
}

void MultilineElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        if (from == getParent()) {
            content.at( 0 )->moveRight(cursor, this);
        }
        else {
            int pos = content.indexOf( static_cast<MultilineSequenceElement*>( from ) );
            if ( pos > -1 ) {
                int upos = pos;
                if ( upos < content.count() ) {
                    if ( upos < content.count()-1 ) {
                        content.at( upos+1 )->moveRight( cursor, this );
                    }
                    else {
                        getParent()->moveRight(cursor, this);
                    }
                    return;
                }
            }
            kDebug( DEBUGID ) << k_funcinfo << endl;
            kDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
        }
    }
}

void MultilineElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    // If you want to select more than one line you'll have to
    // select the whole element.
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        // Coming from the parent (sequence) we go to
        // the very last position
        if (from == getParent()) {
            content.at( content.count()-1 )->moveLeft(cursor, this);
        }
        else {
            // Coming from one of the lines we go to the previous line
            // or to the parent if there is none.
            int pos = content.indexOf( static_cast<MultilineSequenceElement*>( from ) );
            if ( pos > -1 ) {
                if ( pos > 0 ) {
                    //content.at( pos-1 )->moveLeft( cursor, this );
                    // This is rather hackish.
                    // But we know what elements we have here.
                    int cursorPos = cursor->getPos();
                    MultilineSequenceElement* current = content.at( pos );
                    MultilineSequenceElement* newLine = content.at( pos-1 );
                    int tabNum = current->tabBefore( cursorPos );
                    if ( tabNum > -1 ) {
                        int oldTabPos = current->tabPos( tabNum );
                        int newTabPos = newLine->tabPos( tabNum );
                        if ( newTabPos > -1 ) {
                            cursorPos += newTabPos-oldTabPos;
                            int nextNewTabPos = newLine->tabPos( tabNum+1 );
                            if ( nextNewTabPos > -1 ) {
                                cursorPos = qMin( cursorPos, nextNewTabPos );
                            }
                        }
                        else {
                            cursorPos = newLine->countChildren();
                        }
                    }
                    else {
                        int nextNewTabPos = newLine->tabPos( 0 );
                        if ( nextNewTabPos > -1 ) {
                            cursorPos = qMin( cursorPos, nextNewTabPos );
                        }
                    }
                    cursor->setTo( newLine,
                                   qMin( cursorPos,
                                         newLine->countChildren() ) );
                }
                else {
                    getParent()->moveLeft(cursor, this);
                }
            }
            else {
                kDebug( DEBUGID ) << k_funcinfo << endl;
                kDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
            }
        }
    }
}

void MultilineElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        if (from == getParent()) {
            content.at( 0 )->moveRight(cursor, this);
        }
        else {
            int pos = content.indexOf( static_cast<MultilineSequenceElement*>( from ) );
            if ( pos > -1 ) {
                int upos = pos;
                if ( upos < content.count() ) {
                    if ( upos < content.count()-1 ) {
                        //content.at( upos+1 )->moveRight( cursor, this );
                        // This is rather hackish.
                        // But we know what elements we have here.
                        int cursorPos = cursor->getPos();
                        MultilineSequenceElement* current = content.at( upos );
                        MultilineSequenceElement* newLine = content.at( upos+1 );
                        int tabNum = current->tabBefore( cursorPos );
                        if ( tabNum > -1 ) {
                            int oldTabPos = current->tabPos( tabNum );
                            int newTabPos = newLine->tabPos( tabNum );
                            if ( newTabPos > -1 ) {
                                cursorPos += newTabPos-oldTabPos;
                                int nextNewTabPos = newLine->tabPos( tabNum+1 );
                                if ( nextNewTabPos > -1 ) {
                                    cursorPos = qMin( cursorPos, nextNewTabPos );
                                }
                            }
                            else {
                                cursorPos = newLine->countChildren();
                            }
                        }
                        else {
                            int nextNewTabPos = newLine->tabPos( 0 );
                            if ( nextNewTabPos > -1 ) {
                                cursorPos = qMin( cursorPos, nextNewTabPos );
                            }
                        }
                        cursor->setTo( newLine,
                                       qMin( cursorPos,
                                             newLine->countChildren() ) );
                    }
                    else {
                        getParent()->moveRight(cursor, this);
                    }
                    return;
                }
            }
            kDebug( DEBUGID ) << k_funcinfo << endl;
            kDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
        }
    }
}


void MultilineElement::calcSizes( const ContextStyle& context,
                                  ContextStyle::TextStyle tstyle,
                                  ContextStyle::IndexStyle istyle )
{
    luPt mySize = context.getAdjustedSize( tstyle );
    QFont font = context.getDefaultFont();
    font.setPointSizeF( context.layoutUnitPtToPt( mySize ) );
    QFontMetrics fm( font );
    luPixel leading = context.ptToLayoutUnitPt( fm.leading() );
    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle ) );

    int count = content.count();
    luPixel height = -leading;
    luPixel width = 0;
    int tabCount = 0;
    for ( int i = 0; i < count; ++i ) {
        MultilineSequenceElement* line = content.at(i);
        line->calcSizes( context, tstyle, istyle );
        tabCount = qMax( tabCount, line->tabCount() );

        height += leading;
        line->setX( 0 );
        line->setY( height );
        height += line->getHeight() + distY;
        width = qMax( line->getWidth(), width );
    }

    // calculate the tab positions
    for ( int t = 0; t < tabCount; ++t ) {
        luPixel pos = 0;
        for ( int i = 0; i < count; ++i ) {
            MultilineSequenceElement* line = content.at(i);
            if ( t < line->tabCount() ) {
                pos = qMax( pos, line->tab( t )->getX() );
            }
            else {
                pos = qMax( pos, line->getWidth() );
            }
        }
        for ( int i = 0; i < count; ++i ) {
            MultilineSequenceElement* line = content.at(i);
            if ( t < line->tabCount() ) {
                line->moveTabTo( t, pos );
                width = qMax( width, line->getWidth() );
            }
        }
    }

    setHeight( height );
    setWidth( width );
    if ( count == 1 ) {
        setBaseline( content.at( 0 )->getBaseline() );
    }
    else {
        // There's always a first line. No formulas without lines.
        setBaseline( height/2 + context.axisHeight( tstyle ) );
    }
}

void MultilineElement::draw( QPainter& painter, const LuPixelRect& r,
                             const ContextStyle& context,
                             ContextStyle::TextStyle tstyle,
                             ContextStyle::IndexStyle istyle,
                             const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x() + getX(), parentOrigin.y() + getY() );
    int count = content.count();

    if ( context.edit() ) {
        int tabCount = 0;
        painter.setPen( context.getHelpColor() );
        for ( int i = 0; i < count; ++i ) {
            MultilineSequenceElement* line = content.at(i);
            if ( tabCount < line->tabCount() ) {
                for ( int t = tabCount; t < line->tabCount(); ++t ) {
                    BasicElement* marker = line->tab( t );
                    painter.drawLine( context.layoutUnitToPixelX( myPos.x()+marker->getX() ),
                                      context.layoutUnitToPixelY( myPos.y() ),
                                      context.layoutUnitToPixelX( myPos.x()+marker->getX() ),
                                      context.layoutUnitToPixelY( myPos.y()+getHeight() ) );
                }
                tabCount = line->tabCount();
            }
        }
    }

    for ( int i = 0; i < count; ++i ) {
        MultilineSequenceElement* line = content.at(i);
        line->draw( painter, r, context, tstyle, istyle, myPos );
    }
}


void MultilineElement::dispatchFontCommand( FontCommand* cmd )
{
    int count = content.count();
    for ( int i = 0; i < count; ++i ) {
        MultilineSequenceElement* line = content.at(i);
        line->dispatchFontCommand( cmd );
    }
}

void MultilineElement::insert( FormulaCursor* cursor,
                               QList<BasicElement*>& newChildren,
                               Direction direction )
{
    MultilineSequenceElement* e = static_cast<MultilineSequenceElement*>(newChildren.takeAt(0));
    e->setParent(this);
    content.insert( cursor->getPos(), e );

    if (direction == beforeCursor) {
        e->moveLeft(cursor, this);
    }
    else {
        e->moveRight(cursor, this);
    }
    cursor->setSelection(false);
    formula()->changed();
}

void MultilineElement::remove( FormulaCursor* cursor,
                               QList<BasicElement*>& removedChildren,
                               Direction direction )
{
    if ( content.count() == 1 ) { //&& ( cursor->getPos() == 0 ) ) {
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
    }
    else {
        MultilineSequenceElement* e = content.takeAt( cursor->getPos() );
        removedChildren.append( e );
        formula()->elementRemoval( e );
        //cursor->setTo( this, denominatorPos );
        formula()->changed();
    }
}

void MultilineElement::normalize( FormulaCursor* cursor, Direction direction )
{
    int pos = cursor->getPos();
    if ( ( cursor->getElement() == this ) &&
         ( pos > -1 ) && ( pos <= content.count() ) ) {
        switch ( direction ) {
        case beforeCursor:
            if ( pos > 0 ) {
                content.at( pos-1 )->moveLeft( cursor, this );
                break;
            }
            // no break! intended!
        case afterCursor:
            if ( pos < content.count() ) {
                content.at( pos )->moveRight( cursor, this );
            }
            else {
                content.at( pos-1 )->moveLeft( cursor, this );
            }
            break;
        }
    }
    else {
        inherited::normalize( cursor, direction );
    }
}

SequenceElement* MultilineElement::getMainChild()
{
    return content.at( 0 );
}

void MultilineElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    int pos = content.indexOf( dynamic_cast<MultilineSequenceElement*>( child ) );
    if ( pos > -1 ) {
        cursor->setTo( this, pos );
        //content.at( pos )->moveRight( cursor, this );
    }
}


/**
 * Appends our attributes to the dom element.
 */
void MultilineElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    int lineCount = content.count();
    element.setAttribute( "LINES", lineCount );

    QDomDocument doc = element.ownerDocument();
    for ( int i = 0; i < lineCount; ++i ) {
        QDomElement tmp = content.at( i )->getElementDom(doc);
        element.appendChild(tmp);
    }
}

void MultilineElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mtable" : "mtable" );
    QDomElement row; QDomElement cell;

    for ( int i = 0; i < content.count(); ++i ) {
        row = doc.createElement( oasisFormat ? "math:mtr" : "mtr" );
        de.appendChild( row );
        //cell = doc.createElement( "mtd" );
        //row.appendChild( cell );

        //content.at( i )->writeMathML( doc, cell );
        content.at( i )->writeMathML( doc, row, oasisFormat );
    }

    parent.appendChild( de );
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool MultilineElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    int lineCount = 0;
    QString lineCountStr = element.attribute("LINES");
    if(!lineCountStr.isNull()) {
        lineCount = lineCountStr.toInt();
    }
    if (lineCount == 0) {
        kWarning( DEBUGID ) << "lineCount <= 0 in MultilineElement." << endl;
        return false;
    }

    content.clear();
    for ( int i = 0; i < lineCount; ++i ) {
        MultilineSequenceElement* element = new MultilineSequenceElement(this);
        content.append(element);
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool MultilineElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    int lineCount = content.count();
    int i = 0;
    while ( !node.isNull() && i < lineCount ) {
        if ( node.isElement() ) {
            SequenceElement* element = content.at( i );
            QDomElement e = node.toElement();
            if ( !element->buildFromDom( e ) ) {
                return false;
            }
            ++i;
        }
        node = node.nextSibling();
    }
    return true;
}

QString MultilineElement::toLatex()
{
    int lineCount = content.count();
    QString muliline = "\\begin{split} ";
    for ( int i = 0; i < lineCount; ++i ) {
        muliline += content.at( i )->toLatex();
    	muliline += " \\\\ ";
    }
    muliline += "\\end{split}";
    return muliline;
}

// Does this make any sense at all?
QString MultilineElement::formulaString()
{
    int lineCount = content.count();
    QString muliline = "";
    for ( int i = 0; i < lineCount; ++i ) {
        muliline += content.at( i )->formulaString();
    	muliline += "\n";
    }
    //muliline += "";
    return muliline;
}


KFORMULA_NAMESPACE_END
