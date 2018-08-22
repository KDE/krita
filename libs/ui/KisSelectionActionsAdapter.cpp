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
