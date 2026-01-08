/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisNodeSelectionRecipe.h"

#include "kis_layer_utils.h"
#include "kis_tool_utils.h"
#include "kis_lod_transform.h"
#include "kis_node.h"

KisNodeSelectionRecipe::KisNodeSelectionRecipe(KisNodeList _selectedNodes)
    : selectedNodes(_selectedNodes),
      mode(SelectedLayer)
{
}

KisNodeSelectionRecipe::KisNodeSelectionRecipe(KisNodeList _selectedNodes, KisNodeSelectionRecipe::SelectionMode _mode, QPoint _pickPoint)
    : selectedNodes(_selectedNodes),
      mode(_mode),
      pickPoint(_pickPoint)
{
}

KisNodeSelectionRecipe::KisNodeSelectionRecipe(const KisNodeSelectionRecipe &rhs, int levelOfDetail)
    : KisNodeSelectionRecipe(rhs)
{
    KisLodTransform t(levelOfDetail);
    pickPoint = t.map(rhs.pickPoint);
}

KisNodeList KisNodeSelectionRecipe::selectNodesToProcess() const
{
    if (selectedNodes.isEmpty() || mode == SelectedLayer) {
        return selectedNodes;
    }

    KisNodeSP activeRoot = KisLayerUtils::findIsolationRoot(selectedNodes.first());
    const bool wholeGroup = mode == Group;

    KisNodeList result;

    if (!activeRoot) {
        activeRoot = KisLayerUtils::findRoot(selectedNodes.first());
    }

    KisNodeSP node = KisToolUtils::findNode(activeRoot, pickPoint, wholeGroup);
    if (node) {
        result = {node};
    }

    return result;
}
