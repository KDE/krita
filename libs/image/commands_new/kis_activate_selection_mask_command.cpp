/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_activate_selection_mask_command.h"

#include "kis_layer.h"
#include "kis_selection_mask.h"


KisActivateSelectionMaskCommand::KisActivateSelectionMaskCommand(KisSelectionMaskSP selectionMask, bool value)
    : m_selectionMask(selectionMask),
      m_value(value)
{
    if (m_previousActiveMask != m_selectionMask) {
        KisLayerSP parent = dynamic_cast<KisLayer*>(selectionMask->parent().data());
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


