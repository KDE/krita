/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
