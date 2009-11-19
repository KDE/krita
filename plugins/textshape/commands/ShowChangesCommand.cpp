/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
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

#include "ShowChangesCommand.h"

#include <KoTextEditor.h>
#include <TextTool.h>
#include <klocale.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KAction>

ShowChangesCommand::ShowChangesCommand(bool showChanges, TextTool *tool, QUndoCommand *parent) :
    TextCommandBase (parent),
    m_showChanges(showChanges),
    m_tool(tool),
    m_first(true)
{
    if (showChanges)
      setText(i18n("Show Changes"));
    else
      setText(i18n("Hide Changes"));
}

void ShowChangesCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this, m_tool);
    m_tool->m_actionShowChanges->setChecked(!m_showChanges);
    enableDisableStates(!m_showChanges);
}

void ShowChangesCommand::redo()
{
    if (!m_first) {
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this, m_tool);
        m_tool->m_actionShowChanges->setChecked(m_showChanges);
        enableDisableStates(m_showChanges);
    } else {
        m_first = false;
        enableDisableChanges();
    }
}

void ShowChangesCommand::enableDisableChanges()
{
    if (m_tool->m_changeTracker) {
        enableDisableStates(m_showChanges);

        if(m_showChanges)
          insertDeletedChanges();
        else
          removeDeletedChanges();

        const QTextDocument *doc = m_tool->m_textEditor->document();
        Q_ASSERT(doc);
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
        if(lay)
          lay->scheduleLayout();
    }
}

void ShowChangesCommand::enableDisableStates(bool showChanges)
{
    m_tool->m_changeTracker->setEnabled(showChanges);
    m_tool->m_changeTracker->setDisplayDeleted(showChanges);

    QTextCharFormat format = m_tool->m_textEditor->charFormat();
    format.clearProperty(KoCharacterStyle::ChangeTrackerId);
    m_tool->m_textEditor->setCharFormat(format);

    m_tool->m_textTyping = false;
    m_tool->m_textDeleting = false;
}

void ShowChangesCommand::insertDeletedChanges()
{
    int numAddedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    KoTextDocument(m_tool->m_textEditor->document()).changeTracker()->getDeletedChanges(elementVector);

    QTextCursor caret(m_tool->m_textEditor->document());
    caret.beginEditBlock();

    foreach(KoChangeTrackerElement *element, elementVector)
    {
        if (element->isValid()) {
          QTextCharFormat f;
          caret.setPosition(element->getDeleteChangeMarker()->position() + numAddedChars +  1);
          f.setProperty(KoCharacterStyle::ChangeTrackerId, element->getDeleteChangeMarker()->changeId());
          caret.mergeCharFormat(f);
          caret.insertText(element->getDeleteData());
          numAddedChars += element->getDeleteData().length();
        }
    }

    caret.endEditBlock();
}

void ShowChangesCommand::removeDeletedChanges()
{
    int numDeletedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    KoTextDocument(m_tool->m_textEditor->document()).changeTracker()->getDeletedChanges(elementVector);

    QTextCursor caret(m_tool->m_textEditor->document());
    caret.beginEditBlock();

    foreach(KoChangeTrackerElement *element, elementVector)
    {
        if (element->isValid()) {
          QTextCharFormat f;
          caret.setPosition(element->getDeleteChangeMarker()->position() +  1 - numDeletedChars);
          caret.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, element->getDeleteData().length());
          caret.removeSelectedText();
          numDeletedChars += element->getDeleteData().length();
        }
    }

    caret.endEditBlock();
}

ShowChangesCommand::~ShowChangesCommand()
{

}
