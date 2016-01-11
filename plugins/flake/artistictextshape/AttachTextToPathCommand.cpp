/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "AttachTextToPathCommand.h"
#include "ArtisticTextShape.h"

#include <klocalizedstring.h>

AttachTextToPathCommand::AttachTextToPathCommand(ArtisticTextShape *textShape, KoPathShape *pathShape, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_textShape(textShape)
    , m_pathShape(pathShape)
{
    setText(kundo2_i18n("Attach Path"));
    m_oldMatrix = m_textShape->transformation();
}

void AttachTextToPathCommand::redo()
{
    KUndo2Command::redo();
    m_textShape->update();
    m_textShape->putOnPath(m_pathShape);
    m_textShape->update();
}

void AttachTextToPathCommand::undo()
{
    m_textShape->update();
    m_textShape->removeFromPath();
    m_textShape->setTransformation(m_oldMatrix);
    m_textShape->update();
    KUndo2Command::undo();
}
