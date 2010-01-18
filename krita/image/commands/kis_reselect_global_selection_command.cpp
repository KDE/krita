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

KisReselectGlobalSelectionCommand::KisReselectGlobalSelectionCommand(KisImageWSP image, QUndoCommand * parent) :
        QUndoCommand(i18n("Reselect"), parent)
        , m_image(image)
{
}

KisReselectGlobalSelectionCommand::~KisReselectGlobalSelectionCommand()
{
}

void KisReselectGlobalSelectionCommand::redo()
{
    m_oldSelection = m_image->globalSelection();
    m_image->setGlobalSelection(m_image->deselectedGlobalSelection());
    m_image->setDeleselectedGlobalSelection(0);

    m_image->undoAdapter()->emitSelectionChanged();
}

void KisReselectGlobalSelectionCommand::undo()
{
    m_image->setDeleselectedGlobalSelection(m_image->globalSelection());
    m_image->setGlobalSelection(m_oldSelection);

    m_image->undoAdapter()->emitSelectionChanged();
}

