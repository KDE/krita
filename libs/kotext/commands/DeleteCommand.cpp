/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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

#include "DeleteCommand.h"

#include <klocale.h>
#include <kundo2command.h>

#include <KoTextEditor.h>
#include "KoTextEditor_p.h"
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>
#include "KoBookmark.h"
#include <KoTextAnchor.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>

#include <QWeakPointer>

DeleteCommand::DeleteCommand(DeleteMode mode,
                             QTextDocument *document,
                             KoShapeController *shapeController,
                             KUndo2Command *parent)
    : KoTextCommandBase (parent)
    , m_document(document)
    , m_shapeController(shapeController)
    , m_first(true)
    , m_undone(false)
    , m_mode(mode)
{
    setText(i18nc("(qtundo-format)", "Delete"));
}

void DeleteCommand::undo()
{
    KoTextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
    updateListChanges();
    m_undone = true;
}

void DeleteCommand::redo()
{
    m_undone = false;
    if (!m_first) {
        KoTextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
    } else {
        m_first = false;
        if (m_document) {
            KoTextEditor *textEditor = KoTextDocument(m_document).textEditor();
            if (textEditor) {
                textEditor->beginEditBlock();
                doDelete();
                textEditor->endEditBlock();
            }
        }
    }
}

class DeleteVisitor : public KoTextVisitor
{
public:
    DeleteVisitor(KoTextEditor *editor, DeleteCommand *command)
        : KoTextVisitor(editor)
        , m_first(true)
        , m_mergePossible(true)
        , m_command(command)
    {
    }

    virtual void visitFragmentSelection(QTextCursor fragmentSelection)
    {
        if (m_first) {
            m_firstFormat = fragmentSelection.charFormat();
            m_first = false;
        }

        if (m_mergePossible && fragmentSelection.charFormat() != m_firstFormat) {
            m_mergePossible = false;
        }

        KoTextDocument textDocument(fragmentSelection.document());
        KoInlineTextObjectManager *manager = textDocument.inlineTextObjectManager();
        KoInlineObject *object;

        QString selected = fragmentSelection.selectedText();
        fragmentSelection.setPosition(fragmentSelection.selectionStart() + 1);
        int position = fragmentSelection.position();
        const QChar *data = selected.constData();
        for (int i = 0; i < selected.length(); i++) {
            if (data->unicode() == QChar::ObjectReplacementCharacter) {
                fragmentSelection.setPosition(position + i);
                object = manager->inlineTextObject(fragmentSelection);
                m_command->m_invalidInlineObjects.insert(object);
            }
            data++;
        }
    }

    bool m_first;
    bool m_mergePossible;
    DeleteCommand *m_command;
    QTextCharFormat m_firstFormat;
};

void DeleteCommand::doDelete()
{
    KoTextEditor *textEditor = KoTextDocument(m_document).textEditor();
    Q_ASSERT(textEditor);
    QTextCursor *caret = textEditor->cursor();
    QTextCharFormat charFormat = caret->charFormat();
    KoInlineTextObjectManager *inlineObjectManager = KoTextDocument(m_document).inlineTextObjectManager();

    if (!textEditor->hasSelection()) {
        if (m_mode == PreviousChar) {
            caret->movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        } else {
            caret->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        }
    }

    DeleteVisitor visitor(textEditor, this);
    textEditor->recursivelyVisitSelection(m_document.data()->rootFrame()->begin(), visitor);
    m_mergePossible = visitor.m_mergePossible;

    foreach (KoInlineObject *object, m_invalidInlineObjects) {
        deleteTextAnchor(object);
        deleteBookmark(object);
    }
    foreach (KoInlineObject *object, m_bookmarksToRemove) {
        inlineObjectManager->removeInlineObject(object); // doesn't remove the character
    }

    if (textEditor->hasComplexSelection()) {
        m_mergePossible = false;
    }

    if (m_mergePossible) {
        // Store various info needed for checkMerge
        m_format = textEditor->charFormat();;
        m_position = textEditor->selectionStart();
        m_length = textEditor->selectionEnd() - textEditor->selectionStart();
    }

    caret->deleteChar();

    restoreUnmatchedBookmarks(textEditor);

    caret->setCharFormat(charFormat);
}

void DeleteCommand::deleteBookmark(KoInlineObject *object)
{
    KoBookmark *bookmark = dynamic_cast<KoBookmark*>(object);
    if (bookmark) {
        KoInlineTextObjectManager *inlineObjectManager = KoTextDocument(m_document).inlineTextObjectManager();
        KoBookmarkManager *bookmarkManager = inlineObjectManager->bookmarkManager();

        KoBookmark::BookmarkType type = bookmark->type();
        if (type == KoBookmark::StartBookmark) {
            KoBookmark *endmark = bookmark->endBookmark();
            Q_ASSERT(endmark);
            if (endmark && !m_invalidInlineObjects.contains(endmark)) {
                m_unmatchedBookmarks << bookmark;
            } else {
                //don't remove it yet as we need to find it below
                m_bookmarksToRemove << bookmark;
            }
        } else if (type == KoBookmark::EndBookmark) {
            KoBookmark *startmark = bookmarkManager->retrieveBookmark(bookmark->name());
            Q_ASSERT(startmark);
            if (startmark && !m_invalidInlineObjects.contains(startmark)) {
                m_unmatchedBookmarks << bookmark;
            } else {
                inlineObjectManager->removeInlineObject(object); // doesn't remove the character
            }
        } else {
            // single bookmark - can be removed right away
            inlineObjectManager->removeInlineObject(object); // doesn't remove the character
        }
        // Note: Don't delete the object. Removed objects are stored by the bookmark manager
        // for future use. Also, start bookmarks might still have a reference to the end bookmark
        // that is being removed.
    }
}

void DeleteCommand::restoreUnmatchedBookmarks(KoTextEditor *editor)
{
    QTextCursor *caret = editor->cursor();
    int currentPosition = caret->position();

    // now restore the bookmarks that spanned beyond the selection we removed
    foreach(KoBookmark *bookmark, m_unmatchedBookmarks) {

        QTextCharFormat oldCf = editor->charFormat();
        // create a new format out of the old so that the current formatting is
        // also used for the inserted object.  KoVariables render text too ;)
        QTextCharFormat cf(oldCf);
        cf.setObjectType(QTextFormat::UserObject + 1);
        cf.setProperty(KoInlineTextObjectManager::InlineInstanceId, bookmark->id());
        caret->insertText(QString(QChar::ObjectReplacementCharacter), cf);
        // reset to use old format so that the InlineInstanceId is no longer set.
        caret->setCharFormat(oldCf);
    }

    editor->updateInlineObjectPosition(currentPosition);
}

void DeleteCommand::deleteTextAnchor(KoInlineObject *object)
{
    if (object) {
        KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(object);
        if (anchor) {
            KoShape *shape = anchor->shape();
            KUndo2Command *shapeDeleteCommand = m_shapeController->removeShape(shape, this);
            shapeDeleteCommand->redo();
        }
    }
}

int DeleteCommand::id() const
{
    // Should be an enum declared somewhere. KoTextCommandBase.h ???
    return 56789;
}

bool DeleteCommand::mergeWith(const KUndo2Command *command)
{
    class UndoTextCommand : public KUndo2Command
    {
    public:
        UndoTextCommand(QTextDocument *document, KUndo2Command *parent = 0)
        : KUndo2Command(i18nc("(qtundo-format)", "Text"), parent),
        m_document(document)
        {}

        void undo() {
            QTextDocument *doc = m_document.data();
            if (doc)
                doc->undo(KoTextDocument(doc).textEditor()->cursor());
        }

        void redo() {
            QTextDocument *doc = m_document.data();
            if (doc)
                doc->redo(KoTextDocument(doc).textEditor()->cursor());
        }

        QWeakPointer<QTextDocument> m_document;
    };

    KoTextEditor *textEditor = KoTextDocument(m_document).textEditor();
    if (textEditor == 0)
        return false;

    if (command->id() != id())
        return false;

    if (!checkMerge(command))
        return false;

    DeleteCommand *other = const_cast<DeleteCommand *>(static_cast<const DeleteCommand *>(command));

    m_invalidInlineObjects += other->m_invalidInlineObjects;
    other->m_invalidInlineObjects.clear();

    for (int i=0; i < command->childCount(); i++)
        new UndoTextCommand(const_cast<QTextDocument*>(textEditor->document()), this);

    return true;
}

bool DeleteCommand::checkMerge(const KUndo2Command *command)
{
    DeleteCommand *other = const_cast<DeleteCommand *>(static_cast<const DeleteCommand *>(command));

    if (!(m_mergePossible && other->m_mergePossible))
        return false;

    if (m_position == other->m_position && m_format == other->m_format) {
        m_length += other->m_length;
        return true;
    }

    if ( (other->m_position + other->m_length == m_position)
            && (m_format == other->m_format)) {
        m_position = other->m_position;
        m_length += other->m_length;
        return true;
    }
    return false;
}

void DeleteCommand::updateListChanges()
{
    KoTextEditor *textEditor = KoTextDocument(m_document).textEditor();
    if (textEditor == 0)
        return;
    QTextDocument *document = const_cast<QTextDocument*>(textEditor->document());
    QTextCursor tempCursor(document);
    QTextBlock startBlock = document->findBlock(m_position);
    QTextBlock endBlock = document->findBlock(m_position + m_length);
    if (endBlock != document->end())
        endBlock = endBlock.next();
    QTextList *currentList;

    for (QTextBlock currentBlock = startBlock; currentBlock != endBlock; currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        currentList = tempCursor.currentList();
        if (currentList) {
            KoListStyle::ListIdType listId;
            if (sizeof(KoListStyle::ListIdType) == sizeof(uint))
                listId = currentList->format().property(KoListStyle::ListId).toUInt();
            else
                listId = currentList->format().property(KoListStyle::ListId).toULongLong();

            if (!KoTextDocument(document).list(currentBlock)) {
                KoList *list = KoTextDocument(document).list(listId);
                if (list) {
                    list->updateStoredList(currentBlock);
                }
            }
        }
    }
}

DeleteCommand::~DeleteCommand()
{
}
