/*
 * This file is part of the KDE project
 * Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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

#include "SplitSectionsCommand.h"
#include <KoSection.h>
#include <KoSectionEnd.h>
#include <KoTextDocument.h>
#include <KoParagraphStyle.h>
#include <KoTextEditor.h>
#include <KoSectionUtils.h>

#include <klocalizedstring.h>
#include <kundo2command.h>

SplitSectionsCommand::SplitSectionsCommand(QTextDocument *document, SplitType type, int splitPosition)
    : KUndo2Command ()
    , m_first(true)
    , m_document(document)
    , m_type(type)
    , m_splitPosition(splitPosition)
{
    if (m_type == Startings) {
        setText(kundo2_i18n("Split sections startings"));
    } else { // Endings
        setText(kundo2_i18n("Split sections endings"));
    }
}

SplitSectionsCommand::~SplitSectionsCommand()
{
}

void SplitSectionsCommand::undo()
{
    KUndo2Command::undo();
    //FIXME: if it will go to KoTextCommandBase, place UndoRedoFinalizer here

    // All formatting changes will be undone automatically.
    // Model Level is untouched.
}

void SplitSectionsCommand::redo()
{
    KoTextDocument koDocument(m_document);

    if (!m_first) {
        KUndo2Command::redo();
        //FIXME: if it will go to KoTextCommandBase, place UndoRedoFinalizer here

        // All formatting changes will be redone automatically.
        // Model level is untouched.
    } else {
        m_first = false;

        KoTextEditor *editor = koDocument.textEditor();

        if (m_type == Startings) {
            editor->movePosition(QTextCursor::StartOfBlock);
            editor->newLine();
            editor->movePosition(QTextCursor::PreviousBlock);

            QTextBlockFormat fmt = editor->blockFormat();
            KoSectionUtils::setSectionEndings(fmt, QList<KoSectionEnd *>());
            QList<KoSection *> firstBlockStartings = KoSectionUtils::sectionStartings(fmt).mid(0, m_splitPosition);
            QList<KoSection *> moveForward = KoSectionUtils::sectionStartings(fmt).mid(m_splitPosition);
            KoSectionUtils::setSectionStartings(fmt, firstBlockStartings);
            editor->setBlockFormat(fmt);
            editor->movePosition(QTextCursor::NextBlock);
            fmt = editor->blockFormat();
            KoSectionUtils::setSectionStartings(fmt, moveForward);
            editor->setBlockFormat(fmt);
            editor->movePosition(QTextCursor::PreviousBlock);
        } else { // Endings
            editor->movePosition(QTextCursor::EndOfBlock);
            editor->newLine();

            QTextBlockFormat fmt = editor->blockFormat();
            QList<KoSectionEnd *> secondBlockEndings = KoSectionUtils::sectionEndings(fmt).mid(m_splitPosition + 1);
            QList<KoSectionEnd *> moveBackward = KoSectionUtils::sectionEndings(fmt).mid(0, m_splitPosition + 1);
            KoSectionUtils::setSectionEndings(fmt, secondBlockEndings);
            editor->setBlockFormat(fmt);
            editor->movePosition(QTextCursor::PreviousBlock);
            fmt = editor->blockFormat();
            KoSectionUtils::setSectionEndings(fmt, moveBackward);
            editor->setBlockFormat(fmt);
            editor->movePosition(QTextCursor::NextBlock);
        }
    }
}
