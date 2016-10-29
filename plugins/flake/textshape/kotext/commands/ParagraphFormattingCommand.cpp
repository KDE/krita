/* This file is part of the KDE project
 * Copyright (C) 2013 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "ParagraphFormattingCommand.h"

#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include "KoTextEditor_p.h"

#include <klocalizedstring.h>

class MergeAutoParagraphStyleVisitor : public KoTextVisitor
{
public:
    MergeAutoParagraphStyleVisitor(KoTextEditor *editor, const QTextCharFormat &deltaCharFormat, const QTextBlockFormat &deltaBlockFormat)
        : KoTextVisitor(editor)
        , m_deltaCharFormat(deltaCharFormat)
        , m_deltaBlockFormat(deltaBlockFormat)
    {
    }

    virtual void visitBlock(QTextBlock &block, const QTextCursor &caret)
    {
        for (QTextBlock::iterator it = block.begin(); it != block.end(); ++it) {
            QTextCursor fragmentSelection(caret);
            fragmentSelection.setPosition(it.fragment().position());
            fragmentSelection.setPosition(it.fragment().position() + it.fragment().length(), QTextCursor::KeepAnchor);

            if (fragmentSelection.anchor() >= fragmentSelection.position()) {
                continue;
            }

            visitFragmentSelection(fragmentSelection);
        }

        QList<QTextCharFormat>::Iterator it = m_formats.begin();
        Q_FOREACH (QTextCursor cursor, m_cursors) {
            QTextFormat prevFormat(cursor.charFormat());
            cursor.setCharFormat(*it);
            editor()->registerTrackedChange(cursor, KoGenChange::FormatChange, kundo2_i18n("Formatting"), *it, prevFormat, false);
            ++it;
        }
        QTextCursor cursor(caret);
        cursor.mergeBlockFormat(m_deltaBlockFormat);
        cursor.mergeBlockCharFormat(m_deltaCharFormat);
    }

    virtual void visitFragmentSelection(QTextCursor &fragmentSelection)
    {
        QTextCharFormat format = fragmentSelection.charFormat();
        format.merge(m_deltaCharFormat);

        m_formats.append(format);
        m_cursors.append(fragmentSelection);
    }

    QTextCharFormat m_deltaCharFormat;
    QTextBlockFormat m_deltaBlockFormat;
    QList<QTextCharFormat> m_formats;
    QList<QTextCursor> m_cursors;
};

ParagraphFormattingCommand::ParagraphFormattingCommand(KoTextEditor *editor,
                                                       const QTextCharFormat &characterFormat,
                                                       const QTextBlockFormat &blockFormat,
                                                       const KoListLevelProperties &llp,
                                                       KUndo2Command *parent)
      :KUndo2Command(parent),
       m_first(true),
       m_editor(editor),
       m_charFormat(characterFormat),
       m_blockFormat(blockFormat),
       m_levelProperties(llp)
{
    Q_ASSERT(editor);

    setText(kundo2_i18n("Direct Paragraph Formatting"));
}

ParagraphFormattingCommand::~ParagraphFormattingCommand()
{
}

void ParagraphFormattingCommand::undo()
{
    KUndo2Command::undo();
}

void ParagraphFormattingCommand::redo()
{
    if (!m_first) {
        KUndo2Command::redo();
    } else {
        MergeAutoParagraphStyleVisitor visitor(m_editor, m_charFormat, m_blockFormat);

        m_editor->recursivelyVisitSelection(m_editor->document()->rootFrame()->begin(), visitor);

        KoTextEditor::ChangeListFlags flags(KoTextEditor::AutoListStyle | KoTextEditor::DontUnsetIfSame);

        m_editor->setListProperties(m_levelProperties, flags, this);

        m_first = false;
    }
}
