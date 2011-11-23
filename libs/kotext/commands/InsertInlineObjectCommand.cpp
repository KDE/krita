/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "InsertInlineObjectCommand.h"
#include "KoTextEditor.h"
#include "KoTextDocument.h"
#include "KoInlineTextObjectManager.h"
#include "KoInlineObject.h"

InsertInlineObjectCommand::InsertInlineObjectCommand(KoInlineObject *inlineObject, QTextDocument *document, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_inlineObject(inlineObject)
    , m_document(document)
    , m_first(true)
    , m_position(-1)
{
}

InsertInlineObjectCommand::~InsertInlineObjectCommand()
{
    if (m_deleteInlineObject) {
        delete m_inlineObject;
    }
}

void InsertInlineObjectCommand::redo()
{
    KUndo2Command::redo();

    KoTextDocument doc(m_document);
    KoTextEditor *editor = doc.textEditor();
    doc.inlineTextObjectManager()->insertInlineObject(*editor->cursor(), m_inlineObject);
    if (m_first) {
        m_position = editor->cursor()->position();
        m_first = false;
    }
    editor->setPosition(m_position);
    QTextCharFormat format = editor->charFormat();
    m_inlineObject->updatePosition(m_document, m_position, format);

    m_deleteInlineObject = false;
}

void InsertInlineObjectCommand::undo()
{
    KUndo2Command::undo();
    // remove the inlineObject's character from the document
    // remove the inlineObject from the inline object manager
    m_deleteInlineObject = true;
}
