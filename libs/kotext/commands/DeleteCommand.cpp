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

#include "DeleteCommand.h"
#include <klocale.h>
#include <TextTool.h>
#include <kundo2command.h>
#include <KoTextEditor.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextAnchor.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>

#include <QWeakPointer>

DeleteCommand::DeleteCommand(DeleteMode mode, TextTool *tool, KUndo2Command *parent) :
    TextCommandBase (parent),
    m_tool(tool),
    m_first(true),
    m_undone(false),
    m_mode(mode)
{
    setText(i18nc("(qtundo-format)", "Delete"));
}

void DeleteCommand::undo()
{
    foreach (KUndo2Command *command, m_shapeDeleteCommands)
        command->undo();

    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
    updateListChanges();
    m_undone = true;
}

void DeleteCommand::redo()
{
    m_undone = false;
    if (!m_first) {
        foreach (KUndo2Command *command, m_shapeDeleteCommands)
            command->redo();

        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
    } else {
        m_first = false;
        KoTextEditor *textEditor = m_tool->m_textEditor.data();
        if (textEditor) {
            textEditor->beginEditBlock();

            doDelete();

            textEditor->endEditBlock();
        }
    }
}

void DeleteCommand::doDelete()
{
    KoTextEditor *textEditor = m_tool->m_textEditor.data();
    Q_ASSERT(textEditor);
    QTextCursor *caret = textEditor->cursor();
    QTextCursor cursor(*caret);

    //Store the position and length. Will be used in checkMerge
    m_position = cursor.selectionStart();
    m_length = cursor.selectionEnd() - cursor.selectionStart();

    //TODO FIXME Should handle complex selections
    //Store the charFormat. If the selection has multiple charFormats set m_multipleFormatDeletion to true. Will be used in checkMerge
    QTextCharFormat currFormat;
    QTextCharFormat firstFormat;

    m_multipleFormatDeletion = false;

    for (int i = m_position; i < (m_position + m_length); i++) {
        cursor.setPosition(i+1);
        currFormat = cursor.charFormat();

        if (i == m_position ) {
            firstFormat = currFormat;
            continue;
        }

        if (currFormat != firstFormat) {
            m_multipleFormatDeletion = true;
            break;
        }
    }

    if (!m_multipleFormatDeletion)
        m_format = caret->charFormat();;

    //Delete any inline objects present within the selection
    deleteInlineObjects();

    //Now finally Delete the selected text. Don't use selection.deleteChar() direct
    //cause the Texteditor needs to know about the changes too.
    if (m_mode == PreviousChar)
        textEditor->deletePreviousChar();
    else
        textEditor->deleteChar();
 }

void DeleteCommand::deleteInlineObjects()
{
    KoTextEditor *textEditor = m_tool->m_textEditor.data();
    Q_ASSERT(textEditor);
    QTextCursor *caret = textEditor->cursor();
    QTextCursor cursor(*caret);
    const QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(layout);

    KoInlineTextObjectManager *manager = layout->inlineTextObjectManager();
    KoInlineObject *object;

    if (cursor.hasSelection()) {
        QString selected = cursor.selectedText();
        cursor.setPosition(cursor.selectionStart() + 1);
        int position = cursor.position();
        const QChar *data = selected.constData();
        for (int i = 0; i < selected.length(); i++) {
            if (data->unicode() == QChar::ObjectReplacementCharacter) {
                cursor.setPosition(position);
                object = manager->inlineTextObject(cursor);
                deleteTextAnchor(object);
                m_invalidInlineObjects.insert(object);
            } else {
                position++;
            }
            data++;
        }
    } else {
        if (!(m_mode == PreviousChar))
            cursor.movePosition(QTextCursor::Right);

        object = manager->inlineTextObject(cursor);
        deleteTextAnchor(object);
        m_invalidInlineObjects.insert(object);
    }
}

void DeleteCommand::deleteTextAnchor(KoInlineObject *object)
{
    if (object) {
        KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(object);
        if (anchor) {
                KoShape *shape = anchor->shape();
                KUndo2Command *shapeDeleteCommand = m_tool->canvas()->shapeController()->removeShape(shape);
                shapeDeleteCommand->redo();
                m_shapeDeleteCommands.push_back(shapeDeleteCommand);
        }
    }
}

int DeleteCommand::id() const
{
    // Should be an enum declared somewhere. TextCommandBase.h ???
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

    KoTextEditor *textEditor = m_tool->m_textEditor.data();
    if (textEditor == 0)
        return false;

    if (command->id() != id())
        return false;

    if (!checkMerge(command))
        return false;

    DeleteCommand *other = const_cast<DeleteCommand *>(static_cast<const DeleteCommand *>(command));

    m_shapeDeleteCommands += other->m_shapeDeleteCommands;
    other->m_shapeDeleteCommands.clear();

    m_invalidInlineObjects += other->m_invalidInlineObjects;
    other->m_invalidInlineObjects.clear();

    for (int i=0; i < command->childCount(); i++)
        new UndoTextCommand(const_cast<QTextDocument*>(textEditor->document()), this);

    return true;
}

bool DeleteCommand::checkMerge( const KUndo2Command *command )
{
    DeleteCommand *other = const_cast<DeleteCommand *>(static_cast<const DeleteCommand *>(command));

    if (m_multipleFormatDeletion || other->m_multipleFormatDeletion)
        return false;

    if (m_position == other->m_position
            && m_format == other->m_format) {
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
    KoTextEditor *textEditor = m_tool->m_textEditor.data();
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
    if (!m_undone) {
        KoTextEditor *textEditor = m_tool->m_textEditor.data();
        if (textEditor == 0)
            return;
        foreach (KoInlineObject *object, m_invalidInlineObjects) {
            const QTextDocument *document = textEditor->document();
            KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
            KoInlineTextObjectManager *manager = layout->inlineTextObjectManager();
            manager->removeInlineObject(object);
            delete object;
        }

        foreach (KUndo2Command *command, m_shapeDeleteCommands)
            delete command;
    }
}

