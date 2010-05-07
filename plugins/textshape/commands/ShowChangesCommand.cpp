/*
 * This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include <iostream>
#include "ShowChangesCommand.h"

#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoTextEditor.h>
#include <KoTextAnchor.h>
#include <KoInlineTextObjectManager.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoShapeContainer.h>

#include <KAction>
#include <klocale.h>

#include <QTextDocument>
#include <QtAlgorithms>
#include <QList>

ShowChangesCommand::ShowChangesCommand(bool showChanges, QTextDocument *document, KoCanvasBase *canvas, QUndoCommand *parent) :
    TextCommandBase (parent),
    m_document(document),
    m_first(true),
    m_showChanges(showChanges),
    m_canvas(canvas)
{
    Q_ASSERT(document);
    m_changeTracker = KoTextDocument(m_document).changeTracker();
    m_textEditor = KoTextDocument(m_document).textEditor();
    if (showChanges)
      setText(i18n("Show Changes"));
    else
      setText(i18n("Hide Changes"));
}

void ShowChangesCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
    foreach (QUndoCommand *shapeCommand, m_shapeCommands)
        shapeCommand->undo();
    emit toggledShowChange(!m_showChanges);
    enableDisableStates(!m_showChanges);
}

void ShowChangesCommand::redo()
{
    if (!m_first) {
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        foreach (QUndoCommand *shapeCommand, m_shapeCommands)
            shapeCommand->redo();
        emit toggledShowChange(m_showChanges);
        enableDisableStates(m_showChanges);
    } else {
        m_first = false;
        enableDisableChanges();
    }
}

void ShowChangesCommand::enableDisableChanges()
{
    if (m_changeTracker) {
        enableDisableStates(m_showChanges);

        if(m_showChanges)
          insertDeletedChanges();
        else
          removeDeletedChanges();

        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
        if (lay)
          lay->scheduleLayout();
    }
}

void ShowChangesCommand::enableDisableStates(bool showChanges)
{
    m_changeTracker->setDisplayChanges(showChanges);

    QTextCharFormat format = m_textEditor->charFormat();
    format.clearProperty(KoCharacterStyle::ChangeTrackerId);
    m_textEditor->setCharFormat(format);
}

bool isPositionLessThan(KoChangeTrackerElement *element1, KoChangeTrackerElement *element2)
{
    return element1->getDeleteChangeMarker()->position() < element2->getDeleteChangeMarker()->position();
}

void ShowChangesCommand::insertDeletedChanges()
{
    int numAddedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    KoTextDocument(m_textEditor->document()).changeTracker()->getDeletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach (KoChangeTrackerElement *element, elementVector) {
        if (element->isValid()) {
            QTextCursor caret(element->getDeleteChangeMarker()->document());
            caret.setPosition(element->getDeleteChangeMarker()->position() + numAddedChars +  1);
            QTextCharFormat f = caret.charFormat();
            f.setProperty(KoCharacterStyle::ChangeTrackerId, element->getDeleteChangeMarker()->changeId());
            f.clearProperty(KoCharacterStyle::InlineInstanceId);
            caret.setCharFormat(f);
            int insertPosition = caret.position();
            insertDeleteFragment(caret, element->getDeleteChangeMarker());
            checkAndAddAnchoredShapes(insertPosition, fragmentLength(element->getDeleteData()));
            numAddedChars += fragmentLength(element->getDeleteData());
        }
    }
}

void ShowChangesCommand::checkAndAddAnchoredShapes(int position, int length)
{
    QTextCursor cursor(m_textEditor->document());
    for (int i=position;i < (position + length);i++) {
        if (m_textEditor->document()->characterAt(i) == QChar::ObjectReplacementCharacter) {
            cursor.setPosition(i+1);
            KoInlineObject *object = KoTextDocument(m_textEditor->document()).inlineTextObjectManager()->inlineTextObject(cursor);
            if (!object)
                continue;

            KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(object);
            if (!anchor)
                continue;
           
            KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
            KoShapeContainer *container = dynamic_cast<KoShapeContainer *>(lay->shapeForPosition(i));
            
            // a very ugly hack. Since this class is going away soon, it should be okay
            if (!container)
                container = dynamic_cast<KoShapeContainer *>((lay->shapes()).at(0));

            if (container) {
                container->addShape(anchor->shape());
                QUndoCommand *shapeCommand = m_canvas->shapeController()->addShapeDirect(anchor->shape());
                shapeCommand->redo();
                m_shapeCommands.push_front(shapeCommand);
            }
        }
    }
}

void ShowChangesCommand::removeDeletedChanges()
{
    int numDeletedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    m_changeTracker->getDeletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach(KoChangeTrackerElement *element, elementVector) {
        if (element->isValid()) {
            QTextCursor caret(element->getDeleteChangeMarker()->document());
            QTextCharFormat f;
            int deletePosition = element->getDeleteChangeMarker()->position() + 1 - numDeletedChars;
            caret.setPosition(deletePosition);
            int deletedLength = fragmentLength(element->getDeleteData());
            caret.setPosition(deletePosition + deletedLength, QTextCursor::KeepAnchor);
            checkAndRemoveAnchoredShapes(deletePosition, fragmentLength(element->getDeleteData()));
            caret.removeSelectedText();
            numDeletedChars += fragmentLength(element->getDeleteData());
        }
    }
}

void ShowChangesCommand::checkAndRemoveAnchoredShapes(int position, int length)
{
    QTextCursor cursor(m_textEditor->document());
    for (int i=position;i < (position + length);i++) {
        if (m_textEditor->document()->characterAt(i) == QChar::ObjectReplacementCharacter) {
            cursor.setPosition(i+1);
            KoInlineObject *object = KoTextDocument(m_textEditor->document()).inlineTextObjectManager()->inlineTextObject(cursor);
            if (!object)
                continue;

            KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(object);
            if (!anchor)
                continue;
            
            QUndoCommand *shapeCommand = m_canvas->shapeController()->removeShape(anchor->shape());
            shapeCommand->redo();
            m_shapeCommands.push_front(shapeCommand);
        }
    }
}

void ShowChangesCommand::insertDeleteFragment(QTextCursor &cursor, KoDeleteChangeMarker *marker)
{
    QTextDocumentFragment fragment =  KoTextDocument(cursor.document()).changeTracker()->elementById(marker->changeId())->getDeleteData();
    QTextDocument tempDoc;
    QTextCursor tempCursor(&tempDoc);
    tempCursor.insertFragment(fragment);

    bool deletedListItem = false;
    
    for (QTextBlock currentBlock = tempDoc.begin(); currentBlock != tempDoc.end(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        QTextList *textList = tempCursor.currentList();
        KoList *currentList = KoTextDocument(cursor.document()).list(cursor.block());

        if (textList) {
            if (textList->format().property(KoDeleteChangeMarker::DeletedList).toBool() && !currentList) {
                //Found a Deleted List in the fragment. Create a new KoList.
                KoListStyle::ListIdType listId;
                if (sizeof(KoListStyle::ListIdType) == sizeof(uint))
                    listId = textList->format().property(KoListStyle::ListId).toUInt();
                else
                    listId = textList->format().property(KoListStyle::ListId).toULongLong();
                KoListStyle *style = marker->getDeletedListStyle(listId);
                currentList = new KoList(cursor.document(), style);    
            }
            
            deletedListItem = currentBlock.blockFormat().property(KoDeleteChangeMarker::DeletedListItem).toBool();
            if (deletedListItem && currentBlock != tempDoc.begin()) {
                // Found a deleted list item in the fragment. So insert a new list-item
                int deletedListItemLevel = KoList::level(currentBlock);
                cursor.insertBlock(currentBlock.blockFormat(), currentBlock.charFormat());
                if(!currentList) {
                    //This happens when a part of a paragraph and a succeeding list-item are deleted together
                    //So go to the next block and insert it in the list there.
                    QTextCursor tmp(cursor);
                    tmp.setPosition(tmp.block().next().position());
                    currentList = KoTextDocument(tmp.document()).list(tmp.block());
                } 
                currentList->add(cursor.block(), deletedListItemLevel);
            }
        } else if (tempCursor.currentTable()) {
            QTextTable *deletedTable = tempCursor.currentTable();
            int numRows = deletedTable->rows();
            int numColumns = deletedTable->columns();
            QTextTable *insertedTable = cursor.insertTable(numRows, numColumns, deletedTable->format());
            for (int i=0; i<numRows; i++) {
                for (int j=0; j<numColumns; j++) {
                    tempCursor.setPosition(deletedTable->cellAt(i,j).firstCursorPosition().position());
                    tempCursor.setPosition(deletedTable->cellAt(i,j).lastCursorPosition().position(), QTextCursor::KeepAnchor);
                    insertedTable->cellAt(i,j).setFormat(deletedTable->cellAt(i,j).format().toTableCellFormat());
                    cursor.setPosition(insertedTable->cellAt(i,j).firstCursorPosition().position());
                    cursor.insertFragment(tempCursor.selection());
                }
            }
            tempCursor.setPosition(deletedTable->cellAt(numRows-1,numColumns-1).lastCursorPosition().position());
            currentBlock = tempCursor.block();
            //Move the cursor outside of table
            cursor.setPosition(cursor.position() + 1);
            continue;
        } else {
            // This block does not contain a list. So no special work here. 
            if (currentBlock != tempDoc.begin())
                cursor.insertBlock(currentBlock.blockFormat(), currentBlock.charFormat());
        }

        /********************************************************************************************************************/
        /*This section of code is a work-around for a bug in the Qt. This work-around is safe. If and when the bug is fixed */
        /*the if condition would never be true and the code would never get executed                                        */
        /********************************************************************************************************************/
        if ((KoList::level(cursor.block()) != KoList::level(currentBlock)) && currentBlock.text().length()) {
            if (!currentList) {
                QTextCursor tmp(cursor);
                tmp.setPosition(tmp.block().previous().position());
                currentList = KoTextDocument(tmp.document()).list(tmp.block());
            }
            currentList->add(cursor.block(), KoList::level(currentBlock));
        }
        /********************************************************************************************************************/
        
        // Finally insert all the contents of the block into the main document.
        QTextBlock::iterator it;
        for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid())
                cursor.insertText(currentFragment.text(), currentFragment.charFormat());
        }
    }
}

int ShowChangesCommand::fragmentLength(QTextDocumentFragment fragment)
{
    QTextDocument tempDoc;
    QTextCursor tempCursor(&tempDoc);
    tempCursor.insertFragment(fragment);
    int length = 0;
    bool deletedListItem = false;
    for (QTextBlock currentBlock = tempDoc.begin(); currentBlock != tempDoc.end(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        if (tempCursor.currentList()) {
            deletedListItem = currentBlock.blockFormat().property(KoDeleteChangeMarker::DeletedListItem).toBool();
            if (currentBlock != tempDoc.begin() && deletedListItem)
                length += 1; //For the Block separator
        } else if (tempCursor.currentTable()) {
            QTextTable *deletedTable = tempCursor.currentTable();
            int numRows = deletedTable->rows();
            int numColumns = deletedTable->columns();
            for (int i=0; i<numRows; i++) {
                for (int j=0; j<numColumns; j++) {
                    length += 1;
                    length += (deletedTable->cellAt(i,j).lastCursorPosition().position() - deletedTable->cellAt(i,j).firstCursorPosition().position());
                }
            }
            tempCursor.setPosition(deletedTable->cellAt(numRows-1,numColumns-1).lastCursorPosition().position());
            currentBlock = tempCursor.block();
            length += 1;
            continue;
        } else {
            if (currentBlock != tempDoc.begin())
                length += 1; //For the Block Separator
        }
        

        QTextBlock::iterator it;
        for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid())
                length += currentFragment.text().length();
        }
    }
    
    return length;
}

ShowChangesCommand::~ShowChangesCommand()
{
}

#include <ShowChangesCommand.moc>
