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

InsertInlineObjectCommand::InsertInlineObjectCommand(KoTextAnchor *anchor, QTextDocument *document, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_anchor(anchor)
    , m_document(document)
{
}

InsertInlineObjectCommand::~InsertInlineObjectCommand()
{
    if (m_deleteAnchor) {
        delete m_anchor;
    }
}

void InsertInlineObjectCommand::redo()
{
    KUndo2Command::redo();

    // put us back in the inline object manager

    // redo the deletion of the object remplacement character

    m_deleteAnchor = false;
}

void InsertInlineObjectCommand::undo()
{
    KUndo2Command::undo();
    // remove the anchor's character from the document
    // remove the anchor from the inline object manager
    m_deleteAnchor = true;
}
