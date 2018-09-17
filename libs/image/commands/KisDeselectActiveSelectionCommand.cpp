/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisDeselectActiveSelectionCommand.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_selection_mask.h"

KisDeselectActiveSelectionCommand::KisDeselectActiveSelectionCommand(KisSelectionSP activeSelection, KisImageWSP image, KUndo2Command *parent)
    : KisDeselectGlobalSelectionCommand(image, parent),
      m_activeSelection(activeSelection)
{
}

KisDeselectActiveSelectionCommand::~KisDeselectActiveSelectionCommand()
{
}

void KisDeselectActiveSelectionCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);

    if (m_activeSelection && m_activeSelection == image->globalSelection()) {
        KisDeselectGlobalSelectionCommand::redo();
    } else if (m_activeSelection) {
        KisNodeSP parentNode = m_activeSelection->parentNode();
        if (!parentNode) return;

        m_deselectedMask = dynamic_cast<KisSelectionMask*>(parentNode.data());
        if (m_deselectedMask) {
            KIS_SAFE_ASSERT_RECOVER(m_deselectedMask->active()) {
                m_deselectedMask.clear();
                return;
            }

            m_deselectedMask->setActive(false);
        }
    }
}

void KisDeselectActiveSelectionCommand::undo()
{
    if (m_deselectedMask) {
        m_deselectedMask->setActive(true);
        m_deselectedMask.clear();
    } else {
        KisDeselectGlobalSelectionCommand::undo();
    }
}
