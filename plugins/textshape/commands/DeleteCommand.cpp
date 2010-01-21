/*
 This file is part of the KDE project
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
 * Boston, MA 02110-1301, USA.*/

#include "DeleteCommand.h"
#include <KoTextEditor.h>
#include <TextTool.h>
#include <klocale.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KAction>
#include <QTextDocumentFragment>
#include <QUndoCommand>

#include <KDebug>
//#include <iostream>
#include <QDebug>

using namespace std;
DeleteCommand::DeleteCommand(DeleteMode mode, TextTool *tool, QUndoCommand *parent) :
    TextCommandBase (parent),
    m_tool(tool),
    m_first(true),
    m_undone(false),
    m_mode(mode),
    m_removedElements()
{
      setText(i18n("Delete"));
}

void DeleteCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);

    QTextDocument *document = m_tool->m_textEditor->document();
    KoTextDocument(document).changeTracker()->elementById(m_addedChangeElement)->setValid(false);
    foreach (int changeId, m_removedElements) {
      KoTextDocument(document).changeTracker()->elementById(changeId)->setValid(true);
    }

    m_undone = true;
}

void DeleteCommand::redo()
{
    m_undone = false;
    if (!m_first) {
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        QTextDocument *document = m_tool->m_textEditor->document();
        KoTextDocument(document).changeTracker()->elementById(m_addedChangeElement)->setValid(true);
        foreach (int changeId, m_removedElements) {
          KoTextDocument(document).changeTracker()->elementById(changeId)->setValid(false);
        }
    } else {
        m_first = false;
        m_tool->m_textEditor->beginEditBlock();
        if(m_mode == PreviousChar)
            deletePreviousChar();
        else
            deleteChar();
        m_tool->m_textEditor->endEditBlock();
    }
}

void DeleteCommand::deleteChar()
{
    QTextCursor *caret = m_tool->m_textEditor->cursor();

    if (caret->atEnd() && !caret->hasSelection())
        return;

    if (!caret->hasSelection())
        caret->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    deleteSelection(*caret);
}

void DeleteCommand::deletePreviousChar()
{
    QTextCursor *caret = m_tool->m_textEditor->cursor();

    if (caret->atStart() && !caret->hasSelection())
        return;

    if (!caret->hasSelection())
        caret->movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

    deleteSelection(*caret);
}

void DeleteCommand::deleteSelection(QTextCursor &selection)
{
    QTextDocument *document = m_tool->m_textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());

    QTextCursor checker = QTextCursor(selection);
    KoDeleteChangeMarker *deleteChangemarker = 0;
    KoDeleteChangeMarker *testMarker;

    bool backwards = (checker.anchor() > checker.position());
    int selectionBegin = qMin(checker.anchor(), checker.position());
    int selectionEnd = qMax(checker.anchor(), checker.position());
    int changeId;

    QString prefix;
    QString sufix;
    QString delText;

    checker.setPosition(selectionBegin);

    if (KoTextDocument(document).changeTracker()->displayChanges()) {
        if (KoTextDocument(document).changeTracker()->containsInlineChanges(checker.charFormat())) {
            int changeId = checker.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
            if (KoTextDocument(document).changeTracker()->elementById(changeId)->getChangeType() == KoGenChange::deleteChange) {
                prefix =  KoTextDocument(document).changeTracker()->elementById(changeId)->getDeleteData();
                KoTextDocument(document).changeTracker()->elementById(changeId)->setValid(false);
                    m_removedElements.push_back(changeId);
            }
        }
    } else {
        testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
        if (testMarker) {
            prefix = KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
            KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                m_removedElements.push_back(testMarker->changeId());
        }
    }

    checker.setPosition(selectionEnd);
    if (!checker.atEnd()) {
        checker.movePosition(QTextCursor::NextCharacter);
        testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
        if (testMarker) {
            sufix =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
            KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                m_removedElements.push_back(testMarker->changeId());
            }
    }
    checker.setPosition(selectionBegin);

    while ((checker.position() < selectionEnd) && (!checker.atEnd())) {
        QChar charAtPos = document->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos.unicode() != 0x2029) {
            testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                QString inter = KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
                delText = delText + inter;

                if (KoTextDocument(document).changeTracker()->displayChanges())
                            checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, inter.length());

                KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                    m_removedElements.push_back(testMarker->changeId());
           }
        }
        else {
            delText = delText + checker.selectedText();
        }
        checker.setPosition(checker.position());
    }

    if (!sufix.isEmpty()) {
        if (KoTextDocument(document).changeTracker()->displayChanges()) {
            selection.setPosition(selectionBegin);
            selection.setPosition(selectionEnd + sufix.length() + 1, QTextCursor::KeepAnchor);
            selectionEnd += (sufix.length() + 1);
        } else {
            selection.setPosition(selectionBegin);
            selection.setPosition(selectionEnd + 1, QTextCursor::KeepAnchor);
            selectionEnd += 1;
        }
    }

    if (!prefix.isEmpty()) {
        if (KoTextDocument(document).changeTracker()->displayChanges()) {
            selection.setPosition(selectionBegin - prefix.length() - 1);
            selection.setPosition(selectionEnd, QTextCursor::KeepAnchor);
            selectionBegin -= (prefix.length() + 1);
        } else {
            selection.setPosition(selectionBegin - 1);
            selection.setPosition(selectionEnd, QTextCursor::KeepAnchor);
            selectionBegin -= 1;
        }
    }

    QTextDocumentFragment deletedFragment = selection.selection();
    changeId = KoTextDocument(document).changeTracker()->getDeleteChangeId(i18n("Delete"), deletedFragment, selection.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
    KoChangeTrackerElement *element = KoTextDocument(document).changeTracker()->elementById(changeId);
    deleteChangemarker = new KoDeleteChangeMarker(KoTextDocument(document).changeTracker());
    deleteChangemarker->setChangeId(changeId);
    element->setDeleteChangeMarker(deleteChangemarker);
    layout->inlineTextObjectManager()->insertInlineObject(selection, deleteChangemarker);

    m_addedChangeElement = changeId;

    //Clear the changeTrackerId of the marker. Should be done in KoInlineTextObjectManager ideally.
    selection.setPosition(selectionBegin);
    selection.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QTextCharFormat markerFormat = selection.charFormat();
    markerFormat.setProperty(KoCharacterStyle::ChangeTrackerId,0);
    selection.mergeCharFormat(markerFormat);
    selection.setPosition(selectionBegin + 1);

    //Insert the deleted data again after the marker with the charformat set to the change-id
    QString deletedData = prefix + delText + sufix;
    KoTextDocument(document).changeTracker()->elementById(deleteChangemarker->changeId())->setDeleteData(deletedData);

    if (KoTextDocument(document).changeTracker()->displayChanges()) {
        QTextCharFormat f;
        f.setProperty(KoCharacterStyle::ChangeTrackerId, deleteChangemarker->changeId());
        selection.mergeCharFormat(f);
        selection.insertText(deletedData);

        if (backwards)
            selection.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, deletedData.length() + 1);
    } else {
        if (backwards)
            selection.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor,1);
    }
}

int DeleteCommand::id() const
{
    return 98765;
}

bool DeleteCommand::mergeWith( const QUndoCommand *command)
{
    class UndoTextCommand : public QUndoCommand
    {
    public:
        UndoTextCommand(QTextDocument *document, QUndoCommand *parent = 0)
        : QUndoCommand(i18n("Text"), parent),
        m_document(document)
        {}

        void undo() {
            if (m_document.isNull())
                return;
            m_document->undo(KoTextDocument(m_document).textEditor()->cursor());
        }

        void redo() {
            if (m_document.isNull())
                return;
            m_document->redo(KoTextDocument(m_document).textEditor()->cursor());
        }

        QPointer<QTextDocument> m_document;
    };

    if (command->id() != id())
        return false;

    DeleteCommand *other = const_cast<DeleteCommand *>(static_cast<const DeleteCommand *>(command));
    if (other->m_removedElements.contains(m_addedChangeElement)) {
        removeChangeElement(m_addedChangeElement);
        other->m_removedElements.removeAll(m_addedChangeElement);
        m_addedChangeElement = other->m_addedChangeElement;

        m_removedElements += other->m_removedElements;
        other->m_removedElements.clear();

        for(int i=0; i < command->childCount(); i++)
            new UndoTextCommand(m_tool->m_textEditor->document(), this);

        return true;
    }
    return false;
}

DeleteCommand::~DeleteCommand()
{
    if (m_undone) {
        removeChangeElement(m_addedChangeElement);
    } else {
        foreach (int changeId, m_removedElements) {
           removeChangeElement(changeId);
        }
    }
}

void DeleteCommand::removeChangeElement(int changeId)
{
    QTextDocument *document = m_tool->m_textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    KoChangeTrackerElement *element = KoTextDocument(document).changeTracker()->elementById(changeId);
    KoDeleteChangeMarker *marker = element->getDeleteChangeMarker();
    layout->inlineTextObjectManager()->removeInlineObject(marker);
    KoTextDocument(document).changeTracker()->removeById(changeId);
}
