/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_selection_commands.h"

#include <klocale.h>

#include "kis_image.h"
#include "kis_selection.h"
#include "kis_undo_adapter.h"
#include "kis_selection_mask.h"
#include "kis_pixel_selection.h"

KisSetGlobalSelectionCommand::KisSetGlobalSelectionCommand(KisImageWSP image, QUndoCommand * parent, KisSelectionSP selection) :
        QUndoCommand(parent)
        , m_image(image)
{
    m_oldSelection = m_image->globalSelection();

    m_image->setGlobalSelection(selection);
    m_newSelection = m_image->globalSelection();
}

KisSetGlobalSelectionCommand::~KisSetGlobalSelectionCommand()
{
}

void KisSetGlobalSelectionCommand::redo()
{
    m_image->setGlobalSelection(m_newSelection);
}

void KisSetGlobalSelectionCommand::undo()
{
    if (m_oldSelection)
        m_image->setGlobalSelection(m_oldSelection);
    else
        m_image->removeGlobalSelection();
    m_image->undoAdapter()->emitSelectionChanged();
}

