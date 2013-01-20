/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_selection_tool_helper.h"


#include <kundo2command.h>

#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoShapeManager.h>

#include "kis_pixel_selection.h"
#include "kis_shape_selection.h"
#include "kis_image.h"
#include "canvas/kis_canvas2.h"
#include "kis_view2.h"
#include "kis_selection_manager.h"
#include "kis_transaction.h"
#include "commands/kis_selection_commands.h"
#include "kis_shape_controller.h"

KisSelectionToolHelper::KisSelectionToolHelper(KisCanvas2* canvas, KisNodeSP node, const QString& name)
        : m_canvas(canvas)
        , m_layer(0)
        , m_name(name)
{
    m_layer = dynamic_cast<KisLayer*>(node.data());
    while (!m_layer && node->parent()) {
        m_layer = dynamic_cast<KisLayer*>(node->parent().data());
        node = node->parent();
    }
    m_image = m_layer->image();
}

KisSelectionToolHelper::~KisSelectionToolHelper()
{
}

void KisSelectionToolHelper::selectPixelSelection(KisPixelSelectionSP selection, SelectionAction action)
{
    KisUndoAdapter *undoAdapter = m_layer->image()->undoAdapter();
    undoAdapter->beginMacro(m_name);

    bool hasSelection = m_layer->selection();

    if (!hasSelection) {
        undoAdapter->addCommand(new KisSetEmptyGlobalSelectionCommand(m_image));
    }

    KisSelectionTransaction transaction(m_name, m_image->undoAdapter(), m_layer->selection());

    KisPixelSelectionSP pixelSelection = m_layer->selection()->getOrCreatePixelSelection();

    if (!hasSelection && action == SELECTION_SUBTRACT) {
        pixelSelection->invert();
    }

    pixelSelection->applySelection(selection, action);

    QRect dirtyRect = m_image->bounds();
    if (hasSelection && action != SELECTION_REPLACE && action != SELECTION_INTERSECT) {
        dirtyRect = selection->selectedRect();
    }
    m_layer->selection()->updateProjection(dirtyRect);

    transaction.commit(undoAdapter);
    undoAdapter->endMacro();

    pixelSelection->setDirty(dirtyRect);
    m_canvas->view()->selectionManager()->selectionChanged();
}

void KisSelectionToolHelper::addSelectionShape(KoShape* shape)
{
    /**
     * Mark a shape that it belongs to a shape selection
     */
    if(!shape->userData()) {
        shape->setUserData(new KisShapeSelectionMarker);
    }

    KisUndoAdapter *undoAdapter = m_layer->image()->undoAdapter();
    undoAdapter->beginMacro(m_name);

    if (!m_layer->selection()) {
        undoAdapter->addCommand(new KisSetEmptyGlobalSelectionCommand(m_image));
    }

    KisSelectionSP selection = m_layer->selection();
    KisSelectionTransaction transaction(m_name, m_image->undoAdapter(), selection);

    transaction.commit(undoAdapter);

    KUndo2Command *cmd = m_canvas->shapeController()->addShape(shape);
    undoAdapter->addCommand(cmd);
    undoAdapter->endMacro();
}

