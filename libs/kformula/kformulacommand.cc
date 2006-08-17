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

#include <klocale.h>  //This is for undo descriptions

#include "FormulaCursor.h"
#include "FormulaElement.h"
#include "kformulacommand.h"
#include "MatrixElement.h"
#include "SequenceElement.h"
#include "TextElement.h"
#include "MatrixEntryElement.h"
#include "MatrixRowElement.h"


KFORMULA_NAMESPACE_BEGIN

int PlainCommand::evilDestructionCount = 0;

PlainCommand::PlainCommand( const QString& name )
    : KNamedCommand(name)
{
    evilDestructionCount++;
}

PlainCommand::~PlainCommand()
{
    evilDestructionCount--;
}

Command::Command(const QString &name, Container* document)
        : PlainCommand(name), cursordata(0), undocursor(0), doc(document)
{
}

Command::~Command()
{
    delete undocursor;
    delete cursordata;
}

FormulaCursor* Command::getExecuteCursor()
{
    FormulaCursor* cursor = getActiveCursor();
    if (cursordata == 0) {
        setExecuteCursor(getActiveCursor());
    }
    else {
        cursor->setCursorData(cursordata);
    }
    return cursor;
}

void Command::setExecuteCursor(FormulaCursor* cursor)
{
    // assert(cursordata == 0);
    cursordata = cursor->getCursorData();
}

FormulaCursor* Command::getUnexecuteCursor()
{
    FormulaCursor* cursor = getActiveCursor();
    cursor->setCursorData(undocursor);
    destroyUndoCursor();
    return cursor;
}

void Command::setUnexecuteCursor(FormulaCursor* cursor)
{
    // assert(undocursor == 0);
    undocursor = cursor->getCursorData();
}


// ******  Generic Add command

KFCAdd::KFCAdd(const QString &name, Container *document)
        : Command(name, document)
{
//    addList.setAutoDelete( true );
}


void KFCAdd::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
//    cursor->insert(addList, beforeCursor);
    setUnexecuteCursor(cursor);
    cursor->setSelection(false);
    testDirty();
}


void KFCAdd::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    cursor->remove(addList, beforeCursor);
    //cursor->setSelection(false);
    cursor->normalize();
    testDirty();
}



// ******  Remove selection command

KFCRemoveSelection::KFCRemoveSelection(Container *document,
                                       Direction direction)
        : Command(i18n("Remove Selected Text"), document),
          dir(direction)
{
//    removedList.setAutoDelete( true );
}

void KFCRemoveSelection::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    cursor->remove(removedList, dir);
    setUnexecuteCursor(cursor);
    testDirty();
}

void KFCRemoveSelection::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
//    cursor->insert(removedList);
    cursor->setSelection(false);
    testDirty();
}



KFCReplace::KFCReplace(const QString &name, Container* document)
        : KFCAdd(name, document), removeSelection(0)
{
}

KFCReplace::~KFCReplace()
{
    delete removeSelection;
}

void KFCReplace::execute()
{
    if (getActiveCursor()->isSelection() && (removeSelection == 0)) {
        removeSelection = new KFCRemoveSelection(getDocument());
    }
    if (removeSelection != 0) {
        removeSelection->execute();
    }
    KFCAdd::execute();
}

void KFCReplace::unexecute()
{
    KFCAdd::unexecute();
    if (removeSelection != 0) {
        removeSelection->unexecute();
    }
}



KFCRemove::KFCRemove(Container *document,
                     Direction direction)
        : Command(i18n("Remove Selected Text"), document),
          element(0), simpleRemoveCursor(0), dir(direction)
{
//    removedList.setAutoDelete( true );
}

KFCRemove::~KFCRemove()
{
    delete simpleRemoveCursor;
    delete element;
}

void KFCRemove::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    cursor->remove(removedList, dir);
    if (cursor->elementIsSenseless()) {
        simpleRemoveCursor = cursor->getCursorData();
        element = cursor->replaceByMainChildContent();
    }
    setUnexecuteCursor(cursor);
    cursor->normalize( dir );
    testDirty();
}

void KFCRemove::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    if (element != 0) {
        cursor->replaceSelectionWith(element);
        element = 0;

        cursor->setCursorData(simpleRemoveCursor);
        delete simpleRemoveCursor;
        simpleRemoveCursor = 0;
    }
//    cursor->insert(removedList, dir);
    cursor->setSelection(false);
    testDirty();
}


KFCRemoveEnclosing::KFCRemoveEnclosing(Container* document,
                                       Direction dir)
        : Command(i18n("Remove Enclosing Element"), document),
          element(0), direction(dir)
{
}

KFCRemoveEnclosing::~KFCRemoveEnclosing()
{
    delete element;
}

void KFCRemoveEnclosing::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    element = cursor->removeEnclosingElement(direction);
    setUnexecuteCursor(cursor);
    //cursor->normalize();
    cursor->setSelection(false);
    testDirty();
}

void KFCRemoveEnclosing::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    cursor->replaceSelectionWith(element);
    cursor->normalize();
    cursor->setSelection(false);
    element = 0;
    testDirty();
}


// ******  Add root, bracket etc command

KFCAddReplacing::KFCAddReplacing(const QString &name, Container* document)
        : Command(name, document), element(0)
{
}

KFCAddReplacing::~KFCAddReplacing()
{
    delete element;
}


void KFCAddReplacing::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    cursor->replaceSelectionWith(element);
    setUnexecuteCursor(cursor);
    cursor->goInsideElement(element);
    element = 0;
    testDirty();
}


void KFCAddReplacing::unexecute()
{
    FormulaCursor* cursor = getUnexecuteCursor();
    element = cursor->replaceByMainChildContent();
    cursor->normalize();
    testDirty();
}


// ******  Add index command

KFCAddGenericIndex::KFCAddGenericIndex(Container* document, ElementIndexPtr _index)
        : KFCAdd(i18n("Add Index"), document), index(_index)
{
    addElement(new SequenceElement());
}

void KFCAddGenericIndex::execute()
{
    index->setToIndex(getActiveCursor());
    KFCAdd::execute();
}

/*
KFCAddIndex::KFCAddIndex(Container* document,
                         IndexElement* element, ElementIndexPtr index)
        : KFCAddReplacing(i18n("Add Index"), document),
          addIndex(document, index)
{
    setElement(element);
}

KFCAddIndex::~KFCAddIndex()
{
}

void KFCAddIndex::execute()
{
    KFCAddReplacing::execute();
    addIndex.execute();
}

void KFCAddIndex::unexecute()
{
    addIndex.unexecute();
    KFCAddReplacing::unexecute();
}
*/

KFCChangeBaseSize::KFCChangeBaseSize( const QString& name, Container* document,
                                      FormulaElement* formula, int size )
    : PlainCommand( name ), m_document( document ), m_formula( formula ), m_size( size )
{
    m_oldSize = formula->getBaseSize();
}

void KFCChangeBaseSize::execute()
{
    m_formula->setBaseSize( m_size );
    m_document->recalcLayout();
}

void KFCChangeBaseSize::unexecute()
{
    m_formula->setBaseSize( m_oldSize );
    m_document->recalcLayout();
}


FontCommand::FontCommand( const QString& name, Container* document )
    : Command( name, document )
{
//    list.setAutoDelete( false );
//    elementList.setAutoDelete( false );
}


void FontCommand::collectChildren()
{
    list.clear();
    uint count = elementList.count();
    for ( uint i=0; i<count; ++i ) {
        elementList.at( i )->dispatchFontCommand( this );
    }
}


void FontCommand::parseSequences( const QMap<SequenceElement*, int>& parents )
{
    QList<SequenceElement*> sequences = parents.keys();
    for ( QList<SequenceElement*>::iterator i = sequences.begin();
          i != sequences.end();
          ++i ) {
        ( *i )->parse();
    }
}


CharStyleCommand::CharStyleCommand( CharStyle cs, const QString& name, Container* document )
    : FontCommand( name, document ), charStyle( cs )
{
}

void CharStyleCommand::execute()
{
    collectChildren();
    QMap<SequenceElement*, int> parentCollector;

    styleList.clear();
    uint count = childrenList().count();
    styleList.reserve( count );
    for ( uint i=0; i<count; ++i ) {
        TextElement* child = childrenList().at( i );
        styleList[i] = child->getCharStyle();
        child->setCharStyle( charStyle );
        parentCollector[static_cast<SequenceElement*>( child->getParent() )] = 1;
    }
    parseSequences( parentCollector );
    testDirty();
}

void CharStyleCommand::unexecute()
{
    QMap<SequenceElement*, int> parentCollector;
    uint count = childrenList().count();
    //styleList.reserve( count );
    for ( uint i=0; i<count; ++i ) {
        TextElement* child = childrenList().at( i );
        child->setCharStyle( styleList[i] );
        parentCollector[static_cast<SequenceElement*>( child->getParent() )] = 1;
    }
    parseSequences( parentCollector );
    testDirty();
}


CharFamilyCommand::CharFamilyCommand( CharFamily cf, const QString& name, Container* document )
    : FontCommand( name, document ), charFamily( cf )
{
}

void CharFamilyCommand::execute()
{
    collectChildren();

    QMap<SequenceElement*, int> parentCollector;

    familyList.clear();
    uint count = childrenList().count();
    familyList.reserve( count );
    for ( uint i=0; i<count; ++i ) {
        TextElement* child = childrenList().at( i );
        familyList[i] = child->getCharFamily();
        child->setCharFamily( charFamily );
        parentCollector[static_cast<SequenceElement*>( child->getParent() )] = 1;
    }
    parseSequences( parentCollector );
    testDirty();
}

void CharFamilyCommand::unexecute()
{
    QMap<SequenceElement*, int> parentCollector;
    uint count = childrenList().count();
    //familyList.reserve( count );
    for ( uint i=0; i<count; ++i ) {
        TextElement* child = childrenList().at( i );
        child->setCharFamily( familyList[i] );
        parentCollector[static_cast<SequenceElement*>( child->getParent() )] = 1;
    }
    parseSequences( parentCollector );
    testDirty();
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
//    row = matrix->m_matrixRowElements.at( rowPos );
    
    FormulaElement* formula = matrix->formula();
/*    for ( int i = matrix->cols(); i > 0; i-- ) 
        formula->elementRemoval( row->at( i-1 ) );
    
    matrix->m_matrixRowElements.takeAt( rowPos );*/
    formula->changed();

    if ( rowPos < matrix->rows() )
        matrix->matrixEntryAt( rowPos, colPos )->goInside( cursor );
    else
        matrix->matrixEntryAt( rowPos-1, colPos )->goInside( cursor );

    testDirty();
}

void KFCRemoveRow::unexecute()
{
    //matrix->m_matrixRowElements.insert( rowPos, row );
    row = 0;
    FormulaCursor* cursor = getExecuteCursor();
    matrix->matrixEntryAt( rowPos, colPos )->goInside( cursor );
    matrix->formula()->changed();
    testDirty();
}


KFCInsertRow::KFCInsertRow( const QString& name, Container* document, MatrixElement* m, int r, int c )
    : KFCRemoveRow( name, document, m, r, c )
{
/*    row = new QList<MatrixSequenceElement*>;
    row->setAutoDelete( true );
    for ( int i = 0; i < matrix->cols(); i++ ) {
        row->append( new MatrixSequenceElement( matrix ) );
    }*/
}


KFCRemoveColumn::KFCRemoveColumn( const QString& name, Container* document, MatrixElement* m, int r, int c )
    : Command( name, document ), matrix( m ), rowPos( r ), colPos( c )
{
//    column = new QList<MatrixSequenceElement*>;
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
    for ( int i = 0; i < matrix->rows(); i++ ) {
 //       column->append( matrix->matrixEntryAt( i, colPos ) );
        formula->elementRemoval( column->at( i ) );
//        matrix->m_matrixRowElements.at( i )->takeAt( colPos );
    }
    formula->changed();
    if ( colPos < matrix->cols() )
        matrix->matrixEntryAt( rowPos, colPos )->goInside( cursor );
    else
        matrix->matrixEntryAt( rowPos, colPos-1 )->goInside( cursor );
    
    testDirty();
}

void KFCRemoveColumn::unexecute()
{
    for ( int i = 0; i < matrix->rows(); i++ ) {
//        matrix->m_matrixRowElements.at( i )->insert( colPos, column->takeAt( 0 ) );
    }
    FormulaCursor* cursor = getExecuteCursor();
    matrix->matrixEntryAt( rowPos, colPos )->goInside( cursor );
    matrix->formula()->changed();
    testDirty();
}


KFCInsertColumn::KFCInsertColumn( const QString& name, Container* document, MatrixElement* m, int r, int c )
    : KFCRemoveColumn( name, document, m, r, c )
{
    for ( int i = 0; i < matrix->rows(); i++ )
        column->append( new MatrixRowElement( matrix ) );
}

KFCNewLine::KFCNewLine( const QString& name, Container* document,
	                	MatrixEntryElement* line, uint pos )
    : Command( name, document ), m_line( line ), m_pos( pos )
{
    m_newline = new MatrixEntryElement( m_line->getParent() );
}


KFCNewLine::~KFCNewLine()
{
    delete m_newline;
}


void KFCNewLine::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    MatrixRowElement* parent = static_cast<MatrixRowElement*>( m_line->getParent() );
    int linePos = parent->m_matrixEntryElements.indexOf( m_line );
    parent->m_matrixEntryElements.insert( linePos+1, m_newline );

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
    MatrixRowElement* parent = static_cast<MatrixRowElement*>( m_line->getParent() );
    int linePos = parent->m_matrixEntryElements.indexOf( m_line );

    // Now the command owns the new line again.
    m_newline = parent->m_matrixEntryElements.at( linePos+1 );

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
    parent->m_matrixEntryElements.takeAt( linePos+1 );

    // Tell that something changed
    formula->changed();
    testDirty();
}

KFORMULA_NAMESPACE_END
