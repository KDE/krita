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

KisSelectionToolHelper::KisSelectionToolHelper(KisCanvas2* canvas, const QString& name)
        : m_canvas(canvas)
        , m_name(name)
{
    m_image = m_canvas->view()->image();
}

KisSelectionToolHelper::~KisSelectionToolHelper()
{
}

void KisSelectionToolHelper::selectPixelSelection(KisPixelSelectionSP selection, SelectionAction action)
{
    KisView2* view = m_canvas->view();
    if (selection->selectedRect().isEmpty()) {
        m_canvas->view()->selectionManager()->deselect();
        return;
    }

    KisUndoAdapter *undoAdapter = view->image()->undoAdapter();
    undoAdapter->beginMacro(m_name);

    bool hasSelection = view->selection();

    if (!hasSelection) {
        undoAdapter->addCommand(new KisSetEmptyGlobalSelectionCommand(m_image));
    }

    KisPixelSelectionSP pixelSelection = view->selection()->getOrCreatePixelSelection();
    KisSelectionTransaction transaction(m_name, pixelSelection);

    if (!hasSelection && action == SELECTION_SUBTRACT) {
        pixelSelection->invert();
    }

    pixelSelection->applySelection(selection, action);

    QRect dirtyRect = m_image->bounds();
    if (hasSelection && action != SELECTION_REPLACE && action != SELECTION_INTERSECT) {
        dirtyRect = selection->selectedRect();
    }
    view->selection()->updateProjection(dirtyRect);

    transaction.commit(undoAdapter);
    undoAdapter->endMacro();

    pixelSelection->setDirty(dirtyRect);
}

void KisSelectionToolHelper::addSelectionShape(KoShape* shape)
{
    KisView2* view = m_canvas->view();
    /**
     * Mark a shape that it belongs to a shape selection
     */
    if(!shape->userData()) {
        shape->setUserData(new KisShapeSelectionMarker);
    }

    KisUndoAdapter *undoAdapter = view->image()->undoAdapter();
    undoAdapter->beginMacro(m_name);

    if (!view->selection()) {
        undoAdapter->addCommand(new KisSetEmptyGlobalSelectionCommand(m_image));
    }

    KisPixelSelectionSP pixelSelection = view->selection()->getOrCreatePixelSelection();
    KisSelectionTransaction transaction(m_name, pixelSelection);
    pixelSelection->clear();
    transaction.commit(undoAdapter);

    undoAdapter->addCommand(m_canvas->shapeController()->addShape(shape));
    undoAdapter->endMacro();
}

