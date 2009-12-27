/* This file is part of the KDE project
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

#include "AcceptChangeCommand.h"

#include <KoGenChange.h>
#include <KoTextDocument.h>

#include <changetracker/KoChangeTracker.h>
#include <changetracker/KoChangeTrackerElement.h>
#include <styles/KoCharacterStyle.h>

#include <KLocale>

#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

AcceptChangeCommand::AcceptChangeCommand (int changeId, int changeStart, int changeEnd, QTextDocument *document, QUndoCommand* parent) : TextCommandBase(parent),
    m_first(true),
    m_changeId(changeId),
    m_changeStart(changeStart),
    m_changeEnd(changeEnd),
    m_document(document)
{
    setText(i18n("Accept change"));

    m_changeTracker = KoTextDocument(m_document).changeTracker();
}

AcceptChangeCommand::~AcceptChangeCommand()
{
}

void AcceptChangeCommand::redo()
{
    if (m_first) {
        m_first = false;
        if (m_changeTracker->elementById(m_changeId)->getChangeType() == KoGenChange::insertChange) {
            QTextCursor cursor(m_document);
            cursor.setPosition(m_changeStart);
            cursor.setPosition(m_changeEnd, QTextCursor::KeepAnchor);
            QTextCharFormat format = cursor.charFormat();
            format.clearProperty(KoCharacterStyle::ChangeTrackerId);
            cursor.setCharFormat(format);
        }
    }
    else {
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
    }
}

void AcceptChangeCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
}

