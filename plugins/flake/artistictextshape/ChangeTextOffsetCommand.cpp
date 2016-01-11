/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "ChangeTextOffsetCommand.h"
#include "ArtisticTextShape.h"

#include <klocalizedstring.h>

ChangeTextOffsetCommand::ChangeTextOffsetCommand(ArtisticTextShape *textShape, qreal oldOffset, qreal newOffset, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_textShape(textShape)
    , m_oldOffset(oldOffset)
    , m_newOffset(newOffset)
{
    setText(kundo2_i18n("Change Text Offset"));
}

void ChangeTextOffsetCommand::redo()
{
    KUndo2Command::redo();
    m_textShape->update();
    m_textShape->setStartOffset(m_newOffset);
    m_textShape->update();
}

void ChangeTextOffsetCommand::undo()
{
    m_textShape->update();
    m_textShape->setStartOffset(m_oldOffset);
    m_textShape->update();
    KUndo2Command::undo();
}
