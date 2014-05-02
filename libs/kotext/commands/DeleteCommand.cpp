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
#include <KoTextRangeManager.h>
#include <KoAnchorInlineObject.h>
#include <KoAnchorTextRange.h>
#include <KoAnnotation.h>
#include <KoSection.h>
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
    KoTextRangeManager *rangeManager = KoTextDocument(m_document).textRangeManager();
    foreach (KoTextRange *range, m_rangesToRemove) {
        rangeManager->insert(range);
    }
}

void DeleteCommand::redo()
{
    m_undone = false;
    if (!m_first) {
        KoTextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        KoTextRangeManager *rangeManager = KoTextDocument(m_document).textRangeManager();
        foreach (KoTextRange *range, m_rangesToRemove) {
            rangeManager->remove(range);
        }
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

    virtual void visitFragmentSelection(QTextCursor &fragmentSelection)
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

        QString selected = fragmentSelection.selectedText();
        fragmentSelection.setPosition(fragmentSelection.selectionStart() + 1);
        int position = fragmentSelection.position();
        const QChar *data = selected.constData();
        for (int i = 0; i < selected.length(); i++) {
            if (data->unicode() == QChar::ObjectReplacementCharacter) {
                fragmentSelection.setPosition(position + i);
                KoInlineObject *object = manager->inlineTextObject(fragmentSelection);
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

bool DeleteCommand::getPreviousBlock(QTextCursor &cur)
{
    QTextCursor prev = cur;
    bool ok = prev.movePosition(QTextCursor::PreviousBlock);
    qDebug() << "going to previous block " << prev.position() << ' ' << ok << ' ' << prev.block().blockNumber();

    while (ok && prev.currentFrame() != cur.currentFrame()) {
        ok = prev.movePosition(QTextCursor::PreviousBlock);
        qDebug() << "going to previous block " << prev.position() << ' ' << ok << ' ' << prev.block().blockNumber();
    }

    if (!ok || prev.currentFrame() != cur.currentFrame()) {
        // there is no previous block
        qDebug() << "Cannot find previous block";
        return false;
    }

    cur = prev;
    return true;
}

bool DeleteCommand::getNextBlock(QTextCursor &cur)
{
    QTextCursor next = cur;
    bool ok = next.movePosition(QTextCursor::NextBlock);
    qDebug() << "going to next block " << next.position() << ' ' << ok << ' ' << next.block().blockNumber();

    while (ok && next.currentFrame() != cur.currentFrame()) {
        ok = next.movePosition(QTextCursor::PreviousBlock);
        qDebug() << "going to next block " << next.position() << ' ' << ok << ' ' << next.block().blockNumber();
    }

    if (!ok || next.currentFrame() != next.currentFrame()) {
        // there is no previous block
        qDebug() << "Cannot find next block";
        return false;
    }
    cur = next;
    return true;
}

void DeleteCommand::deleteSingleSections(QTextCursor &cur)
{
    QTextBlockFormat format = cur.blockFormat();
    QVariant var = format.property(KoParagraphStyle::SectionStartings);
    QList<QVariant> open_list = var.value< QList<QVariant> >();

    var = format.property(KoParagraphStyle::SectionEndings);
    QList<QVariant> close_list = var.value< QList<QVariant> >();

    for (QList<QVariant>::iterator open_it = open_list.begin();
         open_it != open_list.end(); ) {
        KoSection *sec = static_cast<KoSection *>(open_it->value<void *>());

        bool found = false;
        QList<QVariant>::iterator close_it = close_list.begin();
        for (; close_it != close_list.end(); close_it++) {
            KoSectionEnd *sec_end = static_cast<KoSectionEnd *>(close_it->value<void *>());

            if (sec_end->name == sec->name()) {
                found = true;
                break;
            }
        }

        if (found) {
            open_it = open_list.erase(open_it);
            close_list.erase(close_it);
        } else {
            open_it++;
        }
    }

    var.setValue< QList<QVariant> >(close_list);
    format.setProperty(KoParagraphStyle::SectionEndings, var);
    var.setValue< QList<QVariant> >(open_list);
    format.setProperty(KoParagraphStyle::SectionStartings, var);
    cur.setBlockFormat(format);
}

void DeleteCommand::doDelete()
{
    KoTextEditor *textEditor = KoTextDocument(m_document).textEditor();
    Q_ASSERT(textEditor);
    QTextCursor *caret = textEditor->cursor();
    QTextCharFormat charFormat = caret->charFormat();
    bool caretAtBeginOfBlock = (caret->position() == caret->block().position());

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
        deleteAnchorInlineObject(object);
    }

    //FIXME: can we have a deal with complex selection there??

    // we are going from the end of the selection,
    // find all the paragraph delimiters,
    // and push ends of section backwards
    QTextCursor cur = *caret;
    cur.setPosition(caret->selectionEnd());
    cur.movePosition(QTextCursor::PreviousCharacter);
    while (cur.block().position() + cur.block().length() > caret->selectionStart()) {

        if (cur.block().position() + cur.block().length() > caret->selectionEnd()) {
            if (getPreviousBlock(cur)) {
                continue;
            } else {
                break;
            }
        }

        QTextCursor next = cur;
        if (getNextBlock(next)) {
            QTextBlockFormat cur_format = cur.blockFormat();
            QTextBlockFormat next_format = next.blockFormat();

            if (next_format.hasProperty(KoParagraphStyle::SectionEndings)) {
                deleteSingleSections(next);
                next_format = next.blockFormat();

                QVariant var = next_format.property(KoParagraphStyle::SectionEndings);
                QList<QVariant> close_list_next = var.value< QList<QVariant> >();

                QList<QVariant> close_list_cur;
                if (cur_format.hasProperty(KoParagraphStyle::SectionEndings)) {
                    var = cur_format.property(KoParagraphStyle::SectionEndings);
                    close_list_cur = var.value< QList<QVariant> >();
                }

                if (!close_list_next.empty()) {
                    close_list_cur.append(close_list_next);

                    var.setValue< QList<QVariant> >(close_list_cur);
                    cur_format.setProperty(KoParagraphStyle::SectionEndings, var);
                    cur.setBlockFormat(cur_format);

//                  next_format.clearProperty(KoParagraphStyle::SectionEndings);
//                  next.setBlockFormat(next_format);
                }
            }
        }

        if (!getPreviousBlock(cur))
            break;
    }

    // We are going from the start of the selection,
    // find all the paragraph delimiters
    // and push starts of the sections forward.
    // If start has met the end, we don't push it forward,
    // but deleting the end and the start
    cur = *caret;
    cur.setPosition(caret->selectionStart());
    while (cur.block().position() + cur.block().length() <= caret->selectionEnd()) {
        QTextCursor next = cur;
        if (getNextBlock(next)) {
            QTextCursor nextnext = next;
            if (getNextBlock(nextnext)) {
                QTextBlockFormat nextnext_format = nextnext.blockFormat();
                QTextBlockFormat next_format = next.blockFormat();

                if (next_format.hasProperty(KoParagraphStyle::SectionStartings)) {
                    deleteSingleSections(next);
                    next_format = next.blockFormat();

                    QVariant var = next_format.property(KoParagraphStyle::SectionStartings);
                    QList<QVariant> open_list_next = var.value< QList<QVariant> >();

                    QList<QVariant> open_list_nextnext;
                    if (nextnext_format.hasProperty(KoParagraphStyle::SectionStartings)) {
                        var = nextnext_format.property(KoParagraphStyle::SectionStartings);
                        open_list_nextnext = var.value< QList<QVariant> >();
                    }


                    if (!open_list_next.empty()) {
                        open_list_next.append(open_list_nextnext);

                        var.setValue< QList<QVariant> >(open_list_next);
                        nextnext_format.setProperty(KoParagraphStyle::SectionStartings, var);
                        nextnext.setBlockFormat(nextnext_format);

                        next_format.clearProperty(KoParagraphStyle::SectionEndings);
                        next.setBlockFormat(next_format);
                    }
                }
            }
        }

        if (!getNextBlock(cur))
            break;
    }

    KoTextRangeManager *rangeManager = KoTextDocument(m_document).textRangeManager();

    m_rangesToRemove = rangeManager->textRangesChangingWithin(textEditor->document(), textEditor->selectionStart(), textEditor->selectionEnd(), textEditor->selectionStart(), textEditor->selectionEnd());

    foreach (KoTextRange *range, m_rangesToRemove) {
        KoAnchorTextRange *anchorRange = dynamic_cast<KoAnchorTextRange *>(range);
        KoAnnotation *annotation = dynamic_cast<KoAnnotation *>(range);
        if (anchorRange) {
            KoShape *shape = anchorRange->anchor()->shape();
            if (m_shapeController) {
                KUndo2Command *shapeDeleteCommand = m_shapeController->removeShape(shape, this);
                shapeDeleteCommand->redo();
            }
            // via m_shapeController->removeShape a DeleteAnchorsCommand should be created that
            // also calls rangeManager->remove(range), so we shouldn't do that here aswell
        } else if (annotation) {
            KoShape *shape = annotation->annotationShape();
            if (m_shapeController) {
                KUndo2Command *shapeDeleteCommand = m_shapeController->removeShape(shape, this);
                shapeDeleteCommand->redo();
            }
            // via m_shapeController->removeShape a DeleteAnnotationsCommand should be created that
            // also calls rangeManager->remove(range), so we shouldn't do that here aswell
        } else {
            rangeManager->remove(range);
        }
    }

    if (textEditor->hasComplexSelection()) {
        m_mergePossible = false;
    }

    if (m_mergePossible) {
        // Store various info needed for checkMerge
        m_format = textEditor->charFormat();
        m_position = textEditor->selectionStart();
        m_length = textEditor->selectionEnd() - textEditor->selectionStart();
    }

    caret->deleteChar();

    if (m_mode != PreviousChar || !caretAtBeginOfBlock) {
        caret->setCharFormat(charFormat);
    }
}

void DeleteCommand::deleteAnchorInlineObject(KoInlineObject *object)
{
    if (object) {
        KoAnchorInlineObject *anchorObject = dynamic_cast<KoAnchorInlineObject *>(object);
        if (anchorObject) {
            KoShape *shape = anchorObject->anchor()->shape();
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
