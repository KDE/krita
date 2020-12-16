/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSelectionActionsAdapter.h"

#include "kis_selection_manager.h"


KisSelectionActionsAdapter::KisSelectionActionsAdapter(KisSelectionManager *selectionManager)
    : m_selectionManager(selectionManager)
{
}

void KisSelectionActionsAdapter::selectOpaqueOnNode(KisNodeSP node, SelectionAction action)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_selectionManager);
    m_selectionManager->selectOpaqueOnNode(node, action);
}
