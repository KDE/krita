/* This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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

#include <klocalizedstring.h>

#include <KoList.h>
#include <KoTextEditor.h>
#include <KoTextEditor_p.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>
#include <KoAnchorInlineObject.h>
#include <KoAnchorTextRange.h>
#include <KoAnnotation.h>
#include <KoSection.h>
#include <KoSectionUtils.h>
#include <KoSectionModel.h>
#include <KoSectionEnd.h>
#include <KoShapeController.h>

bool DeleteCommand::SectionDeleteInfo::operator<(const DeleteCommand::SectionDeleteInfo &other) const
{
    // At first we remove sections that lays deeper in tree
    // On one level we delete sections by descending order of their childIdx
    // That is needed on undo, cuz we want it to be simply done by inserting
    // sections back in reverse order of their deletion.
    // Without childIdx compare it is possible that we will want to insert
    // section on position 2 while the number of children is less than 2.

    if (section->level() != other.section->level()) {
        return section->level() > other.section->level();
    }
    return childIdx > other.childIdx;
}

DeleteCommand::DeleteCommand(DeleteMode mode,
                             QTextDocument *document,
                             KoShapeController *shapeController,
                             KUndo2Command *parent)
    : KoTextCommandBase (parent)
    , m_document(document)
    , m_shapeController(shapeController)
    , m_first(true)
    , m_mode(mode)
    , m_mergePossible(true)
{
    setText(kundo2_i18n("Delete"));
}

void DeleteCommand::undo()
{
    KoTextCommandBase::undo();
    UndoRedoFinalizer finalizer(this); // Look at KoTextCommandBase documentation

    // KoList
    updateListChanges();

    // KoTextRange
    KoTextRangeManager *rangeManager = KoTextDocument(m_document).textRangeManager();
    foreach (KoTextRange *range, m_rangesToRemove) {
        rangeManager->insert(range);
    }

    // KoInlineObject
    foreach (KoInlineObject *object, m_invalidInlineObjects) {
        object->manager()->addInlineObject(object);
    }

    // KoSectionModel
    insertSectionsToModel();
}

void DeleteCommand::redo()
{
    if (!m_first) {
        KoTextCommandBase::redo();
        UndoRedoFinalizer finalizer(this); // Look at KoTextCommandBase documentation

        // KoTextRange
        KoTextRangeManager *rangeManager = KoTextDocument(m_document).textRangeManager();
        foreach (KoTextRange *range, m_rangesToRemove) {
            rangeManager->remove(range);
        }

        // KoSectionModel
        deleteSectionsFromModel();

        // TODO: there is nothing for InlineObjects and Lists. Is it OK?
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

// Section handling algorithm:
//   At first, we go though the all section starts and ends
// that are in selection, and delete all pairs, because
// they will be deleted.
//   Then we have multiple cases: selection start split some block
// or don't split any block.
//   In the first case all formatting info will be stored in the
// split block(it has startBlockNum number).
//   In the second case it will be stored in the block pointed by the
// selection end(it has endBlockNum number).
//   Also there is a trivial case, when whole selection is inside
// one block, in this case hasEntirelyInsideBlock will be false
// and we will do nothing.

class DeleteVisitor : public KoTextVisitor
{
public:
    DeleteVisitor(KoTextEditor *editor, DeleteCommand *command)
        : KoTextVisitor(editor)
        , m_first(true)
        , m_command(command)
        , m_startBlockNum(-1)
        , m_endBlockNum(-1)
        , m_hasEntirelyInsideBlock(false)
    {
    }

    virtual void visitBlock(QTextBlock &block, const QTextCursor &caret)
    {
        for (QTextBlock::iterator it = block.begin(); it != block.end(); ++it) {
            QTextCursor fragmentSelection(caret);
            fragmentSelection.setPosition(qMax(caret.selectionStart(), it.fragment().position()));
            fragmentSelection.setPosition(
                qMin(caret.selectionEnd(), it.fragment().position() + it.fragment().length()),
                QTextCursor::KeepAnchor
            );

            if (fragmentSelection.anchor() >= fragmentSelection.position()) {
                continue;
            }

            visitFragmentSelection(fragmentSelection);
        }

        // Section handling below
        bool doesBeginInside = false;
        bool doesEndInside = false;
        if (block.position() >= caret.selectionStart()) { // Begin of the block is inside selection.
            doesBeginInside = true;
            QList<KoSection *> openList = KoSectionUtils::sectionStartings(block.blockFormat());
            foreach (KoSection *sec, openList) {
                m_curSectionDelimiters.push_back(SectionHandle(sec->name(), sec));
            }
        }

        if (block.position() + block.length() <= caret.selectionEnd()) { // End of the block is inside selection.
            doesEndInside = true;
            QList<KoSectionEnd *> closeList = KoSectionUtils::sectionEndings(block.blockFormat());
            foreach (KoSectionEnd *se, closeList) {
                if (!m_curSectionDelimiters.empty() && m_curSectionDelimiters.last().name == se->name()) {
                    KoSection *section = se->correspondingSection();
                    int childIdx = KoTextDocument(m_command->m_document).sectionModel()
                        ->findRowOfChild(section);

                    m_command->m_sectionsToRemove.push_back(
                        DeleteCommand::SectionDeleteInfo(
                            section,
                            childIdx
                        )
                    );
                    m_curSectionDelimiters.pop_back(); // This section will die
                } else {
                    m_curSectionDelimiters.push_back(SectionHandle(se->name(), se));
                }
            }
        }

        if (!doesBeginInside && doesEndInside) {
            m_startBlockNum = block.blockNumber();
        } else if (doesBeginInside && !doesEndInside) {
            m_endBlockNum = block.blockNumber();
        } else if (doesBeginInside && doesEndInside) {
            m_hasEntirelyInsideBlock = true;
        }
    }

    virtual void visitFragmentSelection(QTextCursor &fragmentSelection)
    {
        if (m_first) {
            m_firstFormat = fragmentSelection.charFormat();
            m_first = false;
        }

        if (m_command->m_mergePossible && fragmentSelection.charFormat() != m_firstFormat) {
            m_command->m_mergePossible = false;
        }

        // Handling InlineObjects below
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

    enum SectionHandleAction
    {
        SectionClose, ///< Denotes close of the section.
        SectionOpen ///< Denotes start or beginning of the section.
    };

    /// Helper struct for handling sections.
    struct SectionHandle {
        QString name; ///< Name of the section.
        SectionHandleAction type; ///< Action of a SectionHandle.

        KoSection *dataSec; ///< Pointer to KoSection.
        KoSectionEnd *dataSecEnd; ///< Pointer to KoSectionEnd.

        SectionHandle(QString _name, KoSection *_data)
            : name(_name)
            , type(SectionOpen)
            , dataSec(_data)
            , dataSecEnd(0)
        {
        }

        SectionHandle(QString _name, KoSectionEnd *_data)
            : name(_name)
            , type(SectionClose)
            , dataSec(0)
            , dataSecEnd(_data)
        {
        }
    };

    bool m_first;
    DeleteCommand *m_command;
    QTextCharFormat m_firstFormat;
    int m_startBlockNum;
    int m_endBlockNum;
    bool m_hasEntirelyInsideBlock;
    QList<SectionHandle> m_curSectionDelimiters;
};

void DeleteCommand::finalizeSectionHandling(QTextCursor *cur, DeleteVisitor &v)
{
    // Lets handle pointers from block formats first
    // It means that selection isn't within one block.
    if (v.m_hasEntirelyInsideBlock || v.m_startBlockNum != -1 || v.m_endBlockNum != -1) {
        QList<KoSection *> openList;
        QList<KoSectionEnd *> closeList;
        foreach (const DeleteVisitor::SectionHandle &handle, v.m_curSectionDelimiters) {
            if (handle.type == v.SectionOpen) { // Start of the section.
                openList << handle.dataSec;
            } else { // End of the section.
                closeList << handle.dataSecEnd;
            }
        }

        // We're expanding ends in affected blocks to the end of the start block,
        // delete all sections, that are entirely in affected blocks,
        // and move ends, we have, to the begin of the next after the end block.
        if (v.m_startBlockNum != -1) {
            QTextBlockFormat fmt = cur->document()->findBlockByNumber(v.m_startBlockNum).blockFormat();
            QTextBlockFormat fmt2 = cur->document()->findBlockByNumber(v.m_endBlockNum + 1).blockFormat();
            fmt.clearProperty(KoParagraphStyle::SectionEndings);

            // m_endBlockNum != -1 in this case.
            QList<KoSectionEnd *> closeListEndBlock = KoSectionUtils::sectionEndings(
                cur->document()->findBlockByNumber(v.m_endBlockNum).blockFormat());

            while (!openList.empty() && !closeListEndBlock.empty()
                && openList.last()->name() == closeListEndBlock.first()->name()) {

                int childIdx = KoTextDocument(m_document)
                    .sectionModel()->findRowOfChild(openList.back());
                m_sectionsToRemove.push_back(
                    DeleteCommand::SectionDeleteInfo(
                        openList.back(),
                        childIdx
                    )
                );

                openList.pop_back();
                closeListEndBlock.pop_front();
            }
            openList << KoSectionUtils::sectionStartings(fmt2);
            closeList << closeListEndBlock;

            // We leave open section of start block untouched.
            KoSectionUtils::setSectionStartings(fmt2, openList);
            KoSectionUtils::setSectionEndings(fmt, closeList);

            QTextCursor changer = *cur;
            changer.setPosition(cur->document()->findBlockByNumber(v.m_startBlockNum).position());
            changer.setBlockFormat(fmt);
            if (v.m_endBlockNum + 1 < cur->document()->blockCount()) {
                changer.setPosition(cur->document()->findBlockByNumber(v.m_endBlockNum + 1).position());
                changer.setBlockFormat(fmt2);
            }
        } else { // v.m_startBlockNum == -1
            // v.m_endBlockNum != -1 in this case.
            // We're pushing all new section info to the end block.
            QTextBlockFormat fmt = cur->document()->findBlockByNumber(v.m_endBlockNum).blockFormat();
            QList<KoSection *> allStartings = KoSectionUtils::sectionStartings(fmt);
            fmt.clearProperty(KoParagraphStyle::SectionStartings);

            QList<KoSectionEnd *> pairedEndings;
            QList<KoSectionEnd *> unpairedEndings;

            foreach (KoSectionEnd *se, KoSectionUtils::sectionEndings(fmt)) {
                KoSection *sec = se->correspondingSection();

                if (allStartings.contains(sec)) {
                    pairedEndings << se;
                } else {
                    unpairedEndings << se;
                }
            }

            if (cur->selectionStart()) {
                QTextCursor changer = *cur;
                changer.setPosition(cur->selectionStart() - 1);

                QTextBlockFormat prevFmt = changer.blockFormat();
                QList<KoSectionEnd *> prevEndings = KoSectionUtils::sectionEndings(prevFmt);

                prevEndings = prevEndings + closeList;

                KoSectionUtils::setSectionEndings(prevFmt, prevEndings);
                changer.setBlockFormat(prevFmt);
            }

            KoSectionUtils::setSectionStartings(fmt, openList);
            KoSectionUtils::setSectionEndings(fmt, pairedEndings + unpairedEndings);

            QTextCursor changer = *cur;
            changer.setPosition(cur->document()->findBlockByNumber(v.m_endBlockNum).position());
            changer.setBlockFormat(fmt);
        }
    }

    // Now lets deal with KoSectionModel
    qSort(m_sectionsToRemove.begin(), m_sectionsToRemove.end());
    deleteSectionsFromModel();
}

void DeleteCommand::deleteSectionsFromModel()
{
    KoSectionModel *model = KoTextDocument(m_document).sectionModel();
    foreach (const SectionDeleteInfo &info, m_sectionsToRemove) {
        model->deleteFromModel(info.section);
    }
}

void DeleteCommand::insertSectionsToModel()
{
    KoSectionModel *model = KoTextDocument(m_document).sectionModel();
    QList<SectionDeleteInfo>::iterator it = m_sectionsToRemove.end();
    while (it != m_sectionsToRemove.begin()) {
        it--;
        model->insertToModel(it->section, it->childIdx);
    }
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

    // Sections Model
    finalizeSectionHandling(caret, visitor); // Finalize section handling routine.

    // InlineObjects
    foreach (KoInlineObject *object, m_invalidInlineObjects) {
        deleteInlineObject(object);
    }

    // Ranges
    KoTextRangeManager *rangeManager = KoTextDocument(m_document).textRangeManager();

    m_rangesToRemove = rangeManager->textRangesChangingWithin(
        textEditor->document(),
        textEditor->selectionStart(),
        textEditor->selectionEnd(),
        textEditor->selectionStart(),
        textEditor->selectionEnd()
    );

    foreach (KoTextRange *range, m_rangesToRemove) {
        KoAnchorTextRange *anchorRange = dynamic_cast<KoAnchorTextRange *>(range);
        KoAnnotation *annotation = dynamic_cast<KoAnnotation *>(range);
        if (anchorRange) {
            // we should only delete the anchor if the selection is covering it... not if the selection is
            // just adjecent to the anchor. This is more in line with what other wordprocessors do
            if (anchorRange->position() != textEditor->selectionStart()
                    && anchorRange->position() != textEditor->selectionEnd()) {
                KoShape *shape = anchorRange->anchor()->shape();
                if (m_shapeController) {
                    KUndo2Command *shapeDeleteCommand = m_shapeController->removeShape(shape, this);
                    shapeDeleteCommand->redo();
                }
                // via m_shapeController->removeShape a DeleteAnchorsCommand should be created that
                // also calls rangeManager->remove(range), so we shouldn't do that here aswell
            }
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

    // Check: is merge possible?
    if (textEditor->hasComplexSelection()) {
        m_mergePossible = false;
    }

    //FIXME: lets forbid merging of "section affecting" deletions by now
    if (!m_sectionsToRemove.empty()) {
        m_mergePossible = false;
    }

    if (m_mergePossible) {
        // Store various info needed for checkMerge
        m_format = textEditor->charFormat();
        m_position = textEditor->selectionStart();
        m_length = textEditor->selectionEnd() - textEditor->selectionStart();
    }

    // Actual deletion of text
    caret->deleteChar();

    if (m_mode != PreviousChar || !caretAtBeginOfBlock) {
        caret->setCharFormat(charFormat);
    }
}

void DeleteCommand::deleteInlineObject(KoInlineObject *object)
{
    if (object) {
        KoAnchorInlineObject *anchorObject = dynamic_cast<KoAnchorInlineObject *>(object);
        if (anchorObject) {
            KoShape *shape = anchorObject->anchor()->shape();
            KUndo2Command *shapeDeleteCommand = m_shapeController->removeShape(shape, this);
            shapeDeleteCommand->redo();
        } else {
            object->manager()->removeInlineObject(object);
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
        : KUndo2Command(kundo2_i18n("Text"), parent),
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

    if ((other->m_position + other->m_length == m_position)
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
