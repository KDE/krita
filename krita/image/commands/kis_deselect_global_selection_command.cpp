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

KisDeselectGlobalSelectionCommand::KisDeselectGlobalSelectionCommand(KisImageWSP image, QUndoCommand * parent) :
        QUndoCommand(i18n("Deselect"), parent)
        , m_image(image)
{
}

KisDeselectGlobalSelectionCommand::~KisDeselectGlobalSelectionCommand()
{
}

void KisDeselectGlobalSelectionCommand::redo()
{
    m_oldDeselectedSelection = m_image->deselectedGlobalSelection();
    m_image->setDeleselectedGlobalSelection(m_image->globalSelection());

    if (!m_newSelection) {
        m_image->setGlobalSelection();
        m_newSelection = m_image->globalSelection();
        m_newSelection->getOrCreatePixelSelection()->select(m_image->bounds());
        m_newSelection->setDeselected(true);
    } else
        m_image->setGlobalSelection(m_newSelection);

    m_image->undoAdapter()->emitSelectionChanged();
}

void KisDeselectGlobalSelectionCommand::undo()
{
    m_image->setGlobalSelection(m_image->deselectedGlobalSelection());
    m_image->setDeleselectedGlobalSelection(m_oldDeselectedSelection);

    m_image->undoAdapter()->emitSelectionChanged();
}
