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

#include "RenameSectionCommand.h"
#include <KoSection.h>
#include <KoTextDocument.h>
#include <KoSectionModel.h>

#include <klocalizedstring.h>
#include <kundo2command.h>

RenameSectionCommand::RenameSectionCommand(KoSection *section, const QString &newName, QTextDocument *document)
    : KUndo2Command()
    , m_sectionModel(KoTextDocument(document).sectionModel())
    , m_section(section)
    , m_newName(newName)
    , m_first(true)
{
    setText(kundo2_i18n("Rename Section"));
}

RenameSectionCommand::~RenameSectionCommand()
{
}

void RenameSectionCommand::undo()
{
    KUndo2Command::undo();
    m_sectionModel->setName(m_section, m_oldName);
}

void RenameSectionCommand::redo()
{
    if (!m_first) {
        KUndo2Command::redo();
    }
    m_oldName = m_section->name();
    m_sectionModel->setName(m_section, m_newName);
    m_first = false;
}

int RenameSectionCommand::id() const
{
    //FIXME: extract this to some enum shared accross all commands
    return 34537684;
}

bool RenameSectionCommand::mergeWith(const KUndo2Command *other)
{
    if (other->id() != id()) {
        return false;
    }

    const RenameSectionCommand *command = static_cast<const RenameSectionCommand *>(other);
    if (command->m_section != m_section || m_newName != command->m_oldName) {
        return false;
    }
    m_newName = command->m_oldName;
    return true;
}
