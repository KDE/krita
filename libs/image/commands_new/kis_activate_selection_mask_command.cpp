/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_activate_selection_mask_command.h"

#include "kis_layer.h"
#include "kis_selection_mask.h"


KisActivateSelectionMaskCommand::KisActivateSelectionMaskCommand(KisSelectionMaskSP selectionMask, bool value)
    : m_selectionMask(selectionMask),
      m_value(value)
{
    if (m_previousActiveMask != m_selectionMask) {
        KisLayerSP parent(qobject_cast<KisLayer*>(selectionMask->parent().data()));
        if (parent) {
            m_previousActiveMask = parent->selectionMask();
        }
    }

    m_previousValue = selectionMask->active();
}

void KisActivateSelectionMaskCommand::redo()
{
    m_selectionMask->setActive(m_value);
}

void KisActivateSelectionMaskCommand::undo()
{
    m_selectionMask->setActive(m_previousValue);

    if (m_value && m_previousActiveMask) {
        m_previousActiveMask->setActive(true);
    }
}


