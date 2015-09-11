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

#include "NewSectionCommand.h"
#include <KoSection.h>
#include <KoSectionEnd.h>
#include <KoTextDocument.h>
#include <KoParagraphStyle.h>
#include <KoTextEditor.h>
#include <KoSectionUtils.h>
#include <KoSectionModel.h>

#include <klocalizedstring.h>
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
    //FIXME: if it will go to KoTextCommandBase, place UndoRedoFinalizer here

    // All formatting changes will be undone automatically.
    // Lets handle Model Level (see KoSectionModel).
    KoTextDocument(m_document).sectionModel()->deleteFromModel(m_section);
}

void NewSectionCommand::redo()
{
    KoTextDocument koDocument(m_document);
    KoSectionModel *sectionModel = koDocument.sectionModel();

    if (!m_first) {
        KUndo2Command::redo();
        //FIXME: if it will go to KoTextCommandBase, place UndoRedoFinalizer here

        // All formatting changes will be redone automatically.
        // Lets handle Model Level (see KoSectionModel).
        sectionModel->insertToModel(m_section, m_childIdx);
    } else {
        m_first = false;

        KoTextEditor *editor = koDocument.textEditor();
        editor->newLine();

        m_section = sectionModel->createSection(
            editor->constCursor(),
            sectionModel->sectionAtPosition(editor->constCursor().position())
        );
        m_childIdx = sectionModel->findRowOfChild(m_section);

        KoSectionEnd *sectionEnd = sectionModel->createSectionEnd(m_section);
        QTextBlockFormat fmt = editor->blockFormat();

        QList<KoSection *> sectionStartings = KoSectionUtils::sectionStartings(fmt);
        QList<KoSectionEnd *> sectionEndings = KoSectionUtils::sectionEndings(fmt);

        sectionStartings.append(m_section);
        sectionEndings.prepend(sectionEnd);

        KoSectionUtils::setSectionStartings(fmt, sectionStartings);
        KoSectionUtils::setSectionEndings(fmt, sectionEndings);

        editor->setBlockFormat(fmt);
    }
}
