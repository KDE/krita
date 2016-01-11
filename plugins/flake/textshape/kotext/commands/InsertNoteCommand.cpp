/*
 This file is part of the KDE project
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

#include "InsertNoteCommand.h"

#include <klocalizedstring.h>

#include <KoTextEditor.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>
#include <KoInlineNote.h>


InsertNoteCommand::InsertNoteCommand(KoInlineNote::Type type, QTextDocument *document)
    : KUndo2Command ()
    , m_document(document)
    , m_first(true)
{
    if (type == KoInlineNote::Footnote) {
        setText(kundo2_i18n("Insert Footnote"));
    } else if (type == KoInlineNote::Endnote) {
        setText(kundo2_i18n("Insert Endnote"));
    }
    m_inlineNote = new KoInlineNote(type);
}

InsertNoteCommand::~InsertNoteCommand()
{
}

void InsertNoteCommand::undo()
{
    KUndo2Command::undo();
}

void InsertNoteCommand::redo()
{
    if (!m_first) {
        KUndo2Command::redo();
        QTextCursor cursor(m_document.data());
        cursor.setPosition(m_framePosition);
        m_inlineNote->setTextFrame(cursor.currentFrame());
        m_inlineNote->setMotherFrame(KoTextDocument(m_document).auxillaryFrame());
    } else {
        m_first = false;
        if (m_document) {
            KoTextEditor *textEditor = KoTextDocument(m_document).textEditor();
            if (textEditor) {
                textEditor->beginEditBlock();
                QTextCursor *caret = textEditor->cursor();
                if (textEditor->hasSelection()) {
                    textEditor->deleteChar(false, this);
                }
                KoInlineTextObjectManager *manager = KoTextDocument(m_document).inlineTextObjectManager();
                manager->insertInlineObject(*caret, m_inlineNote);
                m_inlineNote->setMotherFrame(KoTextDocument(m_document).auxillaryFrame());
                m_framePosition = m_inlineNote->textFrame()->lastPosition();
                textEditor->setPosition(m_framePosition);

                textEditor->endEditBlock();
            }
        }
    }
}
