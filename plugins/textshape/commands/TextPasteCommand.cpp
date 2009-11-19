/*
 This file is part of the KDE project
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

#include "TextPasteCommand.h"

#include <KoTextEditor.h>

#include <KoTextDocument.h>
#include <KoTextPaste.h>
#include "TextTool.h"

#include <klocale.h>
#include <kdebug.h>

#include <QApplication>
#include <QMimeData>
#include "DeleteCommand.h"
#include <KAction>

TextPasteCommand::TextPasteCommand(QClipboard::Mode mode, TextTool *tool, QUndoCommand *parent)
    : QUndoCommand (parent),
    m_tool(tool),
    m_first(true),
    m_mode(mode)
{
    setText(i18n("Paste"));
}

void TextPasteCommand::undo()
{
    QUndoCommand::undo();
}

void TextPasteCommand::redo()
{
    KoTextEditor *editor = KoTextDocument(m_tool->m_textShapeData->document()).textEditor();
    if (!m_first) {
        QUndoCommand::redo();
    }
    else {
        //kDebug() << "begin paste command";
        editor->cursor()->beginEditBlock();
        m_first = false;
        if (editor->hasSelection()) {//TODO
            if(m_tool->m_actionShowChanges->isChecked())
              editor->addCommand(new DeleteCommand(DeleteCommand::NextChar, m_tool));
            else
              editor->deleteChar();
        }

        // check for mime type
        const QMimeData *data = QApplication::clipboard()->mimeData(m_mode);

        if (data->hasFormat("application/vnd.oasis.opendocument.text")) {
            //kDebug() << "pasting odf text";
            KoTextPaste paste(m_tool->m_textShapeData, *editor->cursor(), m_tool->m_canvas);
            paste.paste(KoOdf::Text, data);
            //kDebug() << "done with pasting odf";
        } else if (data->hasHtml()) {
            //kDebug() << "pasting html";
            editor->cursor()->insertHtml(data->html());
            //kDebug() << "done with pasting";
        } else if (data->hasText()) {
            //kDebug() << "pasting text";
            editor->cursor()->insertText(data->text());
            //kDebug() << "done with pasting";
        }
        editor->cursor()->endEditBlock();
    }
}
