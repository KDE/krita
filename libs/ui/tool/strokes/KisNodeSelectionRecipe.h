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

#ifndef KISNODESELECTIONRECIPE_H
#define KISNODESELECTIONRECIPE_H

#include "kis_types.h"
#include "kritaui_export.h"


class KRITAUI_EXPORT KisNodeSelectionRecipe
{
public:
    enum SelectionMode {
        SelectedLayer,
        FirstLayer,
        Group
    };

    KisNodeSelectionRecipe(KisNodeList _selectedNodes);
    KisNodeSelectionRecipe(KisNodeList _selectedNodes, SelectionMode _mode, QPoint _pickPoint);
    KisNodeSelectionRecipe(const KisNodeSelectionRecipe &rhs) = default;
    KisNodeSelectionRecipe(const KisNodeSelectionRecipe &rhs, int levelOfDetail);

    KisNodeList selectNodesToProcess() const;

    KisNodeList selectedNodes;
    SelectionMode mode;
    QPoint pickPoint;
};

#endif // KISNODESELECTIONRECIPE_H
