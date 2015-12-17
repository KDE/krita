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
#include <KoInlineTextObjectManager.h>
#include <KoShapeController.h>
#include <KoList.h>
#include <KoParagraphStyle.h>
#include <KoTextOdfSaveHelper.h>
#include <KoTextDrag.h>
#include <KoOdf.h>
#include <KoDocumentRdfBase.h>

#include <QTextDocumentFragment>

#include <klocalizedstring.h>

#include "TextDebug.h"
#include "TextDebug.h"

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
                                                       KoShapeController *shapeController,
                                                       KUndo2Command *parent) :
    KoTextCommandBase (parent),
    m_document(document),
    m_rdf(0),
    m_shapeController(shapeController),
    m_first(true),
    m_undone(false),
    m_canMerge(true),
    m_mode(mode),
    m_removedElements()
{
      setText(kundo2_i18n("Delete"));
      m_rdf = qobject_cast<KoDocumentRdfBase*>(shapeController->resourceManager()->resource(KoText::DocumentRdf).value<QObject*>());
}

void ChangeTrackedDeleteCommand::undo()
{
    if (m_document.isNull()) return;

    KoTextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);

    KoTextDocument textDocument(m_document.data());
    textDocument.changeTracker()->elementById(m_addedChangeElement)->setValid(false);
    foreach (int changeId, m_removedElements) {
        textDocument.changeTracker()->elementById(changeId)->setValid(true);
    }
    updateListChanges();
    m_undone = true;
}

void ChangeTrackedDeleteCommand::redo()
{
    if (!m_document.isNull()) return;

    m_undone = false;
    KoTextDocument textDocument(m_document.data());

    if (!m_first) {
        KoTextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        textDocument.changeTracker()->elementById(m_addedChangeElement)->setValid(true);
        foreach (int changeId, m_removedElements) {
            textDocument.changeTracker()->elementById(changeId)->setValid(false);
        }
    } else {
        m_first = false;
        textDocument.textEditor()->beginEditBlock();
        if(m_mode == PreviousChar)
            deletePreviousChar();
        else
            deleteChar();
        textDocument.textEditor()->endEditBlock();
    }
}

void ChangeTrackedDeleteCommand::deleteChar()
{
    if (m_document.isNull()) return;

    KoTextEditor *editor = KoTextDocument(m_document).textEditor();

    if (editor->atEnd() && !editor->hasSelection())
        return;

    if (!editor->hasSelection())
        editor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    deleteSelection(editor);
}

void ChangeTrackedDeleteCommand::deletePreviousChar()
{
    if (m_document.isNull()) return;

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
    if (m_document.isNull()) return;

    m_canMerge = false;
    bool numberedListItem = false;
    if (!editor->blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem))
         numberedListItem = true;

    // Mark the complete list-item
    QTextBlock block = m_document.data()->findBlock(editor->position());
    editor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, (block.length() - 1));

    // Copy the marked item
    int from = editor->anchor();
    int to = editor->position();
    KoTextOdfSaveHelper saveHelper(m_document.data(), from, to);
    KoTextDrag drag;
#ifdef SHOULD_BUILD_RDF
    if (m_rdf) {
        saveHelper.setRdfModel(m_rdf->model());
    }
#endif
    drag.setOdf(KoOdf::mimeType(KoOdf::Text), saveHelper);
    QTextDocumentFragment fragment = editor->selection();
    drag.setData("text/html", fragment.toHtml("utf-8").toUtf8());
    drag.setData("text/plain", fragment.toPlainText().toUtf8());

    // Delete the marked section
    editor->setPosition(editor->anchor() -1);
    editor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, block.length());
    deleteSelection(editor);
    // Mark it as inserted content
    QTextCharFormat format = editor->charFormat();
    editor->registerTrackedChange(*editor->cursor(), KoGenChange::InsertChange, i18n("Key Press"), format, format, false);
    //Paste the selected text from the clipboard... (XXX: is this really correct here?)
    TextPasteCommand *pasteCommand =
            new TextPasteCommand(drag.mimeData(),
                                 m_document.data(),
                                 m_shapeController,
                                 this);

    pasteCommand->redo();

    // Convert it into a un-numbered list-item or a paragraph
    if (numberedListItem) {
        ListItemNumberingCommand *changeNumberingCommand = new ListItemNumberingCommand(editor->block(), false, this);
        changeNumberingCommand->redo();
    } else {
        KoListLevelProperties llp;
        llp.setStyle(KoListStyle::None);
        llp.setLevel(0);
        ChangeListCommand *changeListCommand = new ChangeListCommand(*editor->cursor(), llp,
                                                                     KoTextEditor::ModifyExistingList | KoTextEditor::MergeWithAdjacentList,
                                                                     this);
        changeListCommand->redo();
    }
    editor->setPosition(editor->block().position());
}

void ChangeTrackedDeleteCommand::deleteSelection(KoTextEditor *editor)
{
    if (m_document.isNull()) return;

    // XXX: don't allow anyone to steal our cursor!
    QTextCursor *selection = editor->cursor();
    QTextCursor checker = QTextCursor(*editor->cursor());

    bool backwards = (checker.anchor() > checker.position());
    int selectionBegin = qMin(checker.anchor(), checker.position());
    int selectionEnd = qMax(checker.anchor(), checker.position());
    int changeId;

    QList<KoShape *> shapesInSelection;

    checker.setPosition(selectionBegin);

    KoTextDocument textDocument(m_document.data());
    KoInlineTextObjectManager *inlineTextObjectManager = textDocument.inlineTextObjectManager();

    while ((checker.position() < selectionEnd) && (!checker.atEnd())) {
        QChar charAtPos = m_document.data()->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        if (inlineTextObjectManager->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
        /* This has changed but since this entire command is going away - let's not bother
                KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(inlineTextObjectManager->inlineTextObject(checker));
                if (anchor)
                    shapesInSelection.push_back(anchor->shape());
        */
        }
        checker.setPosition(checker.position());
    }

    checker.setPosition(selectionBegin);

    if (!KoTextDocument(m_document).changeTracker()->displayChanges()) {
        QChar charAtPos = m_document.data()->characterAt(checker.position() - 1);
    }

    if (KoTextDocument(m_document).changeTracker()->containsInlineChanges(checker.charFormat())) {
        int changeId = checker.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
        if (KoTextDocument(m_document).changeTracker()->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
            QTextDocumentFragment prefix =  KoTextDocument(m_document).changeTracker()->elementById(changeId)->getDeleteData();
            selectionBegin -= (KoChangeTracker::fragmentLength(prefix) + 1 );
            KoTextDocument(m_document).changeTracker()->elementById(changeId)->setValid(false);
            m_removedElements.push_back(changeId);
        }
    }

    checker.setPosition(selectionEnd);
    if (!checker.atEnd()) {
        QChar charAtPos = m_document.data()->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter);
    }

    selection->setPosition(selectionBegin);
    selection->setPosition(selectionEnd, QTextCursor::KeepAnchor);
    QTextDocumentFragment deletedFragment;
    changeId = KoTextDocument(m_document).changeTracker()->getDeleteChangeId(i18n("Delete"), deletedFragment, 0);
    KoChangeTrackerElement *element = KoTextDocument(m_document).changeTracker()->elementById(changeId);

    QTextCharFormat charFormat;
    charFormat.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    selection->mergeCharFormat(charFormat);

    deletedFragment = KoChangeTracker::generateDeleteFragment(*selection);
    element->setDeleteData(deletedFragment);

    //Store the position and length. Will be used in updateListChanges()
    m_position = (selection->anchor() < selection->position()) ? selection->anchor():selection->position();
    m_length = qAbs(selection->anchor() - selection->position());

    updateListIds(*editor->cursor());

    m_addedChangeElement = changeId;

    //Insert the deleted data again after the marker with the charformat set to the change-id
    if (KoTextDocument(m_document).changeTracker()->displayChanges()) {
        int startPosition = selection->position();
        KoChangeTracker::insertDeleteFragment(*selection);
        QTextCursor tempCursor(*selection);
        tempCursor.setPosition(startPosition);
        tempCursor.setPosition(selection->position(), QTextCursor::KeepAnchor);
        // XXX: why was this commented out?
        //tempCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, fragmentLength(deletedFragment));
        updateListIds(tempCursor);
        if (backwards) {
            selection->movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, KoChangeTracker::fragmentLength(deletedFragment) + 1);
        }
    } else {
        if (backwards) {
            selection->movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor,1);
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
            : KUndo2Command(kundo2_i18n("Text"), parent),
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
            new UndoTextCommand(m_document.data(), this);
        }

        return true;
    }
    return false;
}

void ChangeTrackedDeleteCommand::updateListIds(QTextCursor &cursor)
{
    if (m_document.isNull()) return;

    m_newListIds.clear();
    QTextCursor tempCursor(m_document.data());
    QTextBlock startBlock = m_document.data()->findBlock(cursor.anchor());
    QTextBlock endBlock = m_document.data()->findBlock(cursor.position());
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
    if (m_document.isNull()) return;

    QTextCursor tempCursor(m_document.data());
    QTextBlock startBlock = m_document.data()->findBlock(m_position);
    QTextBlock endBlock = m_document.data()->findBlock(m_position + m_length);
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
    KoTextDocument textDocument(m_document);
    KoChangeTrackerElement *element = textDocument.changeTracker()->elementById(changeId);
    KoTextDocument(m_document).changeTracker()->removeById(changeId);
}
