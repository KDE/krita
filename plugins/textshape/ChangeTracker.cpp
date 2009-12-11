/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#include "ChangeTracker.h"
#include "TextTool.h"

#include <KDebug>

ChangeTracker::ChangeTracker(TextTool *parent)
        : QObject(parent),
        m_document(0),
        m_tool(parent),
        m_enableSignals(true),
        m_reverseUndo(false)
{
}

void ChangeTracker::setDocument(QTextDocument * document)
{
    m_reverseUndo = false;
    if (m_document)
        disconnect(m_document, SIGNAL(contentsChange(int, int, int)), this, SLOT(contentsChange(int, int, int)));
    m_document = document;
    if (m_document)
        connect(m_document, SIGNAL(contentsChange(int, int, int)), this, SLOT(contentsChange(int, int, int)));
}

int ChangeTracker::getChangeId(QString title, int existingChangeId)
{
    Q_UNUSED(title);
    Q_UNUSED(existingChangeId);
    kDebug(32500) << "ChangeTracker::changeId :" << m_changeId;
    return m_changeId++;
}

void ChangeTracker::contentsChange(int from, int charsRemoves, int charsAdded)
{
    Q_UNUSED(from);
    Q_UNUSED(charsRemoves);
    Q_UNUSED(charsAdded);
/*
    if (! m_enableSignals) return;
    m_enableSignals = false;
    kDebug(32500) << "ChangeTracker::contentsChange" << from << "," << charsRemoves << "," << charsAdded;

    if (charsRemoves == 0 && charsAdded == 0) {
        // I think we can quietly ignore this.
    } else if (charsRemoves == 0) { // easy
        QTextCursor cursor(m_document);
        cursor.setPosition(from);
        cursor.setPosition(from + charsAdded, QTextCursor::KeepAnchor);
        kDebug(32500) << "   added text:" << cursor.selectedText();
    } else {
        bool prev = m_tool->m_allowAddUndoCommand;
        m_tool->m_allowAddUndoCommand = false;
        m_reverseUndo ? m_document->redo() : m_document->undo();
        QTextCursor cursor(m_document);
        cursor.setPosition(from);
        cursor.setPosition(from + charsRemoves, QTextCursor::KeepAnchor);
        QString previousText = cursor.selectedText();
        m_reverseUndo ? m_document->undo() : m_document->redo();
        m_tool->m_allowAddUndoCommand = prev;

        cursor.setPosition(from);
        cursor.setPosition(from + charsAdded, QTextCursor::KeepAnchor);

        kDebug(32500) << "   - " << previousText;
        kDebug(32500) << "   + " << cursor.selectedText();
    }

    m_enableSignals = true;
    m_reverseUndo = false;
*/
}

void ChangeTracker::notifyForUndo()
{
    m_reverseUndo = true;
}

#include "ChangeTracker.moc"
