/* This file is part of the KDE project
* Copyright (C) 2010 Pierre Stirnweiss \pstirnweiss@googlemail.com>
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

#include "RejectChangeCommand.h"

#include <KoGenChange.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>

#include <changetracker/KoChangeTracker.h>
#include <changetracker/KoChangeTrackerElement.h>
#include <styles/KoCharacterStyle.h>

#include <KLocale>

#include <QPair>
#include <QStack>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

RejectChangeCommand::RejectChangeCommand (int changeId, QList<QPair<int, int> > changeRanges, QTextDocument *document, QUndoCommand* parent) : TextCommandBase(parent),
    m_first(true),
    m_changeId(changeId),
    m_changeRanges(changeRanges),
    m_document(document)
{
    setText(i18n("Reject change"));

    m_changeTracker = KoTextDocument(m_document).changeTracker();
    m_layout = dynamic_cast<KoTextDocumentLayout*>(document->documentLayout());
}

RejectChangeCommand::~RejectChangeCommand()
{
}

void RejectChangeCommand::redo()
{
    if (m_first) {
        m_first = false;
        QTextCursor cursor(m_document);
        if (m_changeTracker->elementById(m_changeId)->getChangeType() == KoGenChange::InsertChange) {
            QList<QPair<int, int> >::const_iterator it;
            QStack<QPair<int, int> > deleteRanges;
            for (it = m_changeRanges.constBegin(); it != m_changeRanges.constEnd(); it++) {
                deleteRanges.push(QPair<int, int>((*it).first, (*it).second));
            }
            while (!deleteRanges.isEmpty()) {
                QPair<int, int> range = deleteRanges.pop();
                cursor.setPosition(range.first);
                cursor.setPosition(range.second, QTextCursor::KeepAnchor);
                cursor.deleteChar();
            }
        }
        else if (m_changeTracker->elementById(m_changeId)->getChangeType() == KoGenChange::FormatChange) {
            QList<QPair<int, int> >::const_iterator it;
            for (it = m_changeRanges.constBegin(); it != m_changeRanges.constEnd(); it++) {
                cursor.setPosition((*it).first);
                cursor.setPosition((*it).second, QTextCursor::KeepAnchor);
                int changeId = cursor.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
                QTextCharFormat format = m_changeTracker->elementById(m_changeId)->getPrevFormat().toCharFormat();
                if (changeId == m_changeId) {
                    if (int parentChangeId = m_changeTracker->parent(m_changeId)) {
                        format.setProperty(KoCharacterStyle::ChangeTrackerId, parentChangeId);
                    }
                    else {
                        format.clearProperty(KoCharacterStyle::ChangeTrackerId);
                    }
                    cursor.setCharFormat(format);
                }
            }
        } else if (m_changeTracker->elementById(m_changeId)->getChangeType() == KoGenChange::DeleteChange){
            QList<QPair<int, int> >::const_iterator it;
            QStack<QPair<int, int> > deleteRanges;
            for (it = m_changeRanges.constBegin(); it != m_changeRanges.constEnd(); it++) {
                cursor.setPosition((*it).first);
                cursor.setPosition((*it).second, QTextCursor::KeepAnchor);
                deleteRanges.push(QPair<int, int>((*it).first, (*it).second));
            }
            while (!deleteRanges.isEmpty()) {
                QPair<int, int> range = deleteRanges.pop();
                cursor.setPosition(range.first);
                cursor.setPosition(range.second, QTextCursor::KeepAnchor);
                if (dynamic_cast<KoDeleteChangeMarker*>(m_layout->inlineTextObjectManager()->inlineTextObject(cursor))) {
                    cursor.deleteChar();
                }
                else {
                    QTextCharFormat format = cursor.charFormat();
                    format.clearProperty(KoCharacterStyle::ChangeTrackerId);
                    cursor.setCharFormat(format);
                }
            }
        }
        m_changeTracker->acceptRejectChange(m_changeId, true);
    }
    else {
        m_changeTracker->acceptRejectChange(m_changeId, true);
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
    }
    emit acceptRejectChange();}

void RejectChangeCommand::undo()
{
    m_changeTracker->acceptRejectChange(m_changeId, false);
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
    emit acceptRejectChange();
}

#include <RejectChangeCommand.moc>
