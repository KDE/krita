/*
 * This file is part of the KDE project
 * Copyright (C) 2014 Denis Kuplyakov <dener.kup@gmail.com>
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

#include "NewSectionCommand.h"
#include <KoSection.h>
#include <KoSectionEnd.h>
#include <KoTextDocument.h>
#include <KoParagraphStyle.h>
#include <KoSectionManager.h>
#include <KoTextEditor.h>
#include <KoSectionUtils.h>

#include <klocale.h>
#include <kundo2command.h>

NewSectionCommand::NewSectionCommand(QTextDocument *document)
    : KUndo2Command ()
    , m_first(true)
    , m_document(document)
{
    setText(kundo2_i18n("New Section"));
}

NewSectionCommand::~NewSectionCommand()
{
}

void NewSectionCommand::undo()
{
    KUndo2Command::undo();
    KoTextDocument(m_document).sectionManager()->invalidate();
}

void NewSectionCommand::redo()
{
    KoTextDocument(m_document).sectionManager()->invalidate();

    if (!m_first) {
        KUndo2Command::redo();
    } else {
        m_first = false;

        KoTextEditor *editor = KoTextDocument(m_document).textEditor();

        editor->newLine();

        KoSection *start = new KoSection(editor->constCursor());
        KoSectionEnd *end = new KoSectionEnd(start);
        QTextBlockFormat fmt = editor->blockFormat();

        QList<KoSection *> sectionStartings = KoSectionUtils::sectionStartings(fmt);
        QList<KoSectionEnd *> sectionEndings = KoSectionUtils::sectionEndings(fmt);

        sectionStartings.append(start);
        sectionEndings.prepend(end);

        KoSectionUtils::setSectionStartings(fmt, sectionStartings);
        KoSectionUtils::setSectionEndings(fmt, sectionEndings);

        editor->setBlockFormat(fmt);
    }
}
