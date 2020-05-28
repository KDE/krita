/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

    KisNodeList result;

    const bool wholeGroup = mode == Group;
    KisNodeSP node = KisToolUtils::findNode(KisLayerUtils::findRoot(selectedNodes.first()), pickPoint, wholeGroup);
    if (node) {
        result = {node};
    }

    return result;
}
