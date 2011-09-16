/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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
 * Boston, MA 02110-1301, USA.*/

#include "ChangeTrackedDeleteCommand.h"
#include "TextPasteCommand.h"
#include "ListItemNumberingCommand.h"
#include "ChangeListCommand.h"

#include <KoTextEditor.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextAnchor.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoList.h>
#include <KoParagraphStyle.h>
#include <KoTextOdfSaveHelper.h>
#include <KoTextDrag.h>
#include <KoOdf.h>
#include <KoDocumentRdfBase.h>

#include <QTextDocumentFragment>

#include <kundo2command.h>
#include <klocale.h>
#include <KAction>

#include <KDebug>
#include <QDebug>
#include <QWeakPointer>

//A convenience function to get a ListIdType from a format

static KoListStyle::ListIdType ListId(const QTextListFormat &format)
{
    KoListStyle::ListIdType listId;

    if (sizeof(KoListStyle::ListIdType) == sizeof(uint))
        listId = format.property(KoListStyle::ListId).toUInt();
    else
        listId = format.property(KoListStyle::ListId).toULongLong();

    return listId;
}

using namespace std;
ChangeTrackedDeleteCommand::ChangeTrackedDeleteCommand(DeleteMode mode,
                                                       QTextDocument *document,
                                                       KoDocumentRdfBase *rdf,
                                                       KoShapeController *shapeController,
                                                       KoResourceManager *resourceManager,
                                                       KUndo2Command *parent) :
    TextCommandBase (parent),
    m_document(document),
    m_rdf(rdf),
    m_shapeController(shapeController),
    m_resourceManager(resourceManager),
    m_first(true),
    m_undone(false),
    m_canMerge(true),
    m_mode(mode),
    m_removedElements()
{
      setText(i18nc("(qtundo-format)", "Delete"));
}

void ChangeTrackedDeleteCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
    KoTextDocument(m_document).changeTracker()->elementById(m_addedChangeElement)->setValid(false);
    foreach (int changeId, m_removedElements) {
      KoTextDocument(document).changeTracker()->elementById(changeId)->setValid(true);
    }
    updateListChanges();
    m_undone = true;
}

void ChangeTrackedDeleteCommand::redo()
{
    m_undone = false;
    if (!m_first) {
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        KoTextDocument(m_document).changeTracker()->elementById(m_addedChangeElement)->setValid(true);
        foreach (int changeId, m_removedElements) {
          KoTextDocument(m_document).changeTracker()->elementById(changeId)->setValid(false);
        }
    } else {
        m_first = false;
        KoTextDocument(m_document).textEditor()->beginEditBlock();
        if(m_mode == PreviousChar)
            deletePreviousChar();
        else
            deleteChar();
        KoTextDocument(m_document).textEditor()->endEditBlock();
    }
}

void ChangeTrackedDeleteCommand::deleteChar()
{
    KoTextEditor *editor = KoTextDocument(m_document).textEditor();

    if (editor->atEnd() && !editor->hasSelection())
        return;

    if (!editor->hasSelection())
        reditor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    deleteSelection(editor);
}

void ChangeTrackedDeleteCommand::deletePreviousChar()
{
    KoTextEditor *editor = KoTextDocument(m_document).textEditor();

    if (editor->atStart() && !editor->hasSelection())
        return;

    if (!editor->hasSelection()
            && editor->block().textList()
            && (editor->position() == editor->block().position())) {
        handleListItemDelete(editor);
        return;
    }

    if (!editor->hasSelection()) {
        editor->movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }
    deleteSelection(editor);
}

void ChangeTrackedDeleteCommand::handleListItemDelete(KoTextEditor *editor)
{
    m_canMerge = false;
    const QTextDocument *document = editor.document();

    bool numberedListItem = false;
    if (!editor.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem))
         numberedListItem = true;      
 
    // Mark the complete list-item
    QTextBlock block = document->findBlock(editor.position());
    editor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, (block.length() - 1));

    // Copy the marked item
    int from = editor.anchor();
    int to = editor.position();
    KoTextOdfSaveHelper saveHelper(m_document, from, to);
    KoTextDrag drag;

    if (rdf) {
        saveHelper.setRdfModel(rdf->model());
    }
    drag.setOdf(KoOdf::mimeType(KoOdf::Text), saveHelper);
    QTextDocumentFragment fragment = editor.selection();
    drag.setData("text/html", fragment.toHtml("utf-8").toUtf8());
    drag.setData("text/plain", fragment.toPlainText().toUtf8());
    drag.addToClipboard();

    // Delete the marked section
    editor.setPosition(editor.anchor() -1);
    editor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, block.length());
    deleteSelection(editor);
    // Insert a new Block and paste the copied contents
    editor.insertBlock();
    // Mark it as inserted content
    QTextCharFormat format = editor.charFormat();
    KoTextDocument(m_document)->textEditor->registerTrackedChange(editor, KoGenChange::InsertChange, i18n("Key Press"), format, format, false);
    //Paste the selected text
    TextPasteCommand *pasteCommand =
            new TextPasteCommand(QClipboard::Clipboard,
                                 m_document,
                                 m_rdf,
                                 m_shapeController,
                                 m_resourceManager,
                                 this);
    pasteCommand->redo();

    // Convert it into a un-numbered list-item or a paragraph
    if (numberedListItem) {
        ListItemNumberingCommand *changeNumberingCommand = new ListItemNumberingCommand(editor.block(), false, this);
        changeNumberingCommand->redo();
    } else {
        ChangeListCommand *changeListCommand = new ChangeListCommand(editor, KoListStyle::None, 0,
                                                                     ChangeListCommand::ModifyExistingList | ChangeListCommand::MergeWithAdjacentList, 
                                                                     this);
        changeListCommand->redo();
    }
    editor.setPosition(editor.block().position());
}

void ChangeTrackedDeleteCommand::deleteSelection(KoTextEditor *editor)
{
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());

    // XXX: don't allow anyone to steal our cursor!
    QTextCursor *selection = editor->cursor();
    QTextCursor checker = QTextCursor(editor->cursor());
    KoDeleteChangeMarker *deleteChangemarker = 0;
    KoDeleteChangeMarker *testMarker;

    bool backwards = (checker.anchor() > checker.position());
    int selectionBegin = qMin(checker.anchor(), checker.position());
    int selectionEnd = qMax(checker.anchor(), checker.position());
    int changeId;

    QList<KoShape *> shapesInSelection;

    checker.setPosition(selectionBegin);

    while ((checker.position() < selectionEnd) && (!checker.atEnd())) {
        QChar charAtPos = document->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                QTextDocumentFragment inter = KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
                if (!KoTextDocument(document).changeTracker()->displayChanges()) {
                    KoChangeTracker::insertDeleteFragment(checker, testMarker);
                    selectionEnd = selectionEnd + KoChangeTracker::fragmentLength(inter);
                }
                checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, KoChangeTracker::fragmentLength(inter));
                KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                m_removedElements.push_back(testMarker->changeId());
           } else {
                KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(layout->inlineTextObjectManager()->inlineTextObject(checker));
                if (anchor)
                    shapesInSelection.push_back(anchor->shape());
           } 
        }
        checker.setPosition(checker.position());
    }

    checker.setPosition(selectionBegin);

    if (!KoTextDocument(document).changeTracker()->displayChanges()) {
        QChar charAtPos = document->characterAt(checker.position() - 1);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                KoChangeTracker::insertDeleteFragment(checker, testMarker);
                QTextDocumentFragment prevFragment = KoTextDocument(checker.document()).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
                selectionBegin += KoChangeTracker::fragmentLength(prevFragment);
                selectionEnd += KoChangeTracker::fragmentLength(prevFragment);
            }
        }
    }

    if (KoTextDocument(document).changeTracker()->containsInlineChanges(checker.charFormat())) {
        int changeId = checker.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
        if (KoTextDocument(document).changeTracker()->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
            QTextDocumentFragment prefix =  KoTextDocument(document).changeTracker()->elementById(changeId)->getDeleteData();
            selectionBegin -= (KoChangeTracker::fragmentLength(prefix) + 1 );
            KoTextDocument(document).changeTracker()->elementById(changeId)->setValid(false);
            m_removedElements.push_back(changeId);
        }
    }

    checker.setPosition(selectionEnd);
    if (!checker.atEnd()) {
        QChar charAtPos = document->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                QTextDocumentFragment sufix =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
                if (!KoTextDocument(document).changeTracker()->displayChanges())
                    KoChangeTracker::insertDeleteFragment(checker, testMarker);
                selectionEnd += KoChangeTracker::fragmentLength(sufix) + 1;
                KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                m_removedElements.push_back(testMarker->changeId());
            }
        }
    }

    selection->setPosition(selectionBegin);
    selection->setPosition(selectionEnd, QTextCursor::KeepAnchor);
    QTextDocumentFragment deletedFragment;
    changeId = KoTextDocument(document).changeTracker()->getDeleteChangeId(i18n("Delete"), deletedFragment, 0);
    KoChangeTrackerElement *element = KoTextDocument(document).changeTracker()->elementById(changeId);
    deleteChangemarker = new KoDeleteChangeMarker(KoTextDocument(document).changeTracker());
    deleteChangemarker->setChangeId(changeId);
    element->setDeleteChangeMarker(deleteChangemarker);

    QTextCharFormat charFormat;
    charFormat.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    selection->mergeCharFormat(charFormat);

    deletedFragment = KoChangeTracker::generateDeleteFragment(*selection, deleteChangemarker);
    element->setDeleteData(deletedFragment);

    //Store the position and length. Will be used in updateListChanges()
    m_position = (selection->anchor() < selection->position()) ? selection->anchor():selection->position();
    m_length = qAbs(selection->anchor() - selection->position());

    updateListIds(editor);
    layout->inlineTextObjectManager()->insertInlineObject(*selection, deleteChangemarker);

    m_addedChangeElement = changeId;
    
    //Insert the deleted data again after the marker with the charformat set to the change-id
    if (KoTextDocument(document).changeTracker()->displayChanges()) {
        int startPosition = selection->position();
        KoChangeTracker::insertDeleteFragment(*selection, deleteChangemarker);
        QTextCursor tempCursor((*selection);
        tempCursor.setPosition(startPosition);
        tempCursor.setPosition(selection->position(), QTextCursor::KeepAnchor);
        // XXX: why was this commented out?
        //tempCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, fragmentLength(deletedFragment));
        updateListIds(tempCursor);
        if (backwards) {
            selection.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, KoChangeTracker::fragmentLength(deletedFragment) + 1);
        }
    } else {
        if (backwards) {
            selection.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor,1);
        }

        foreach (KoShape *shape, shapesInSelection) {
            KUndo2Command *shapeDeleteCommand = m_shapeController->removeShape(shape, this);
            shapeDeleteCommand->redo();
            m_canMerge = false;
        }
    }
}

int ChangeTrackedDeleteCommand::id() const
{
    return 98765;
}

bool ChangeTrackedDeleteCommand::mergeWith( const KUndo2Command *command)
{
    class UndoTextCommand : public KUndo2Command
    {
    public:
        UndoTextCommand(QTextDocument *document, KUndo2Command *parent = 0)
        : KUndo2Command(i18nc("(qtundo-format)", "Text"), parent),
        m_document(document)
        {}

        void undo() {
            QTextDocument *doc = const_cast<QTextDocument*>(m_document.data());
            if (doc)
                doc->undo(KoTextDocument(doc).textEditor()->cursor());
        }

        void redo() {
            QTextDocument *doc = const_cast<QTextDocument*>(m_document.data());
            if (doc)
                doc->redo(KoTextDocument(doc).textEditor()->cursor());
        }

        QWeakPointer<QTextDocument> m_document;
    };

    if (command->id() != id())
        return false;

    ChangeTrackedDeleteCommand *other = const_cast<ChangeTrackedDeleteCommand *>(static_cast<const ChangeTrackedDeleteCommand *>(command));

    if (other->m_canMerge == false)
        return false;

    if (other->m_removedElements.contains(m_addedChangeElement)) {
        removeChangeElement(m_addedChangeElement);
        other->m_removedElements.removeAll(m_addedChangeElement);
        m_addedChangeElement = other->m_addedChangeElement;

        m_removedElements += other->m_removedElements;
        other->m_removedElements.clear();

        m_newListIds = other->m_newListIds;

        m_position = other->m_position;
        m_length = other->m_length;

        for(int i=0; i < command->childCount(); i++) {
            new UndoTextCommand(m_document, this);
        }

        return true;
    }
    return false;
}

void ChangeTrackedDeleteCommand::updateListIds(KoTextEditor *editor)
{
    m_newListIds.clear();
    QTextCursor tempCursor(m_document);
    QTextBlock startBlock = m_document->findBlock(editor->anchor());
    QTextBlock endBlock = m_document->findBlock(editor->position());
    QTextList *currentList;

    for (QTextBlock currentBlock = startBlock; currentBlock != endBlock.next(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        currentList = tempCursor.currentList();
        if (currentList) {
            KoListStyle::ListIdType listId = ListId(currentList->format());
            m_newListIds.push_back(listId);
        }
    }
}
void ChangeTrackedDeleteCommand::updateListChanges()
{
    QTextCursor tempCursor(m_document);
    QTextBlock startBlock = m_document->findBlock(m_position);
    QTextBlock endBlock = m_document->findBlock(m_position + m_length);
    QTextList *currentList;
    int newListIdsCounter = 0;

    for (QTextBlock currentBlock = startBlock; currentBlock != endBlock.next(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        currentList = tempCursor.currentList();
        if (currentList) {
            KoListStyle::ListIdType listId = m_newListIds[newListIdsCounter];
            if (!KoTextDocument(m_document).list(currentBlock)) {
                KoList *list = KoTextDocument(m_document).list(listId);
                if (list)
                    list->updateStoredList(currentBlock);
            }
            newListIdsCounter++;
        }
    }
}

ChangeTrackedDeleteCommand::~ChangeTrackedDeleteCommand()
{
    if (m_undone) {
        removeChangeElement(m_addedChangeElement);
    } else {
        foreach (int changeId, m_removedElements) {
           removeChangeElement(changeId);
        }
    }
}

void ChangeTrackedDeleteCommand::removeChangeElement(int changeId)
{
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    KoChangeTrackerElement *element = KoTextDocument(m_document).changeTracker()->elementById(changeId);
    KoDeleteChangeMarker *marker = element->getDeleteChangeMarker();
    layout->inlineTextObjectManager()->removeInlineObject(marker);
    KoTextDocument(m_document).changeTracker()->removeById(changeId);
}
