#ifndef KISSELECTIONACTIONSADAPTER_H
#define KISSELECTIONACTIONSADAPTER_H

#include "kritaui_export.h"
#include "kis_types.h"
#include "KisSelectionTags.h"

class KisSelectionManager;


class KRITAUI_EXPORT KisSelectionActionsAdapter
{
public:
    KisSelectionActionsAdapter(KisSelectionManager *selectionManager);
    void selectOpaqueOnNode(KisNodeSP node, SelectionAction action);

private:
    KisSelectionManager *m_selectionManager;
};

#endif // KISSELECTIONACTIONSADAPTER_H
