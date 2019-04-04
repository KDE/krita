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
#include <kactioncollection.h>

#include <KoShapeController.h>
#include <KoPathShape.h>

#include "kis_pixel_selection.h"
#include "kis_shape_selection.h"
#include "kis_image.h"
#include "canvas/kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include "kis_transaction.h"
#include "commands/kis_selection_commands.h"
#include "kis_shape_controller.h"

#include <kis_icon.h>
#include "kis_processing_applicator.h"
#include "commands_new/kis_transaction_based_command.h"
#include "kis_gui_context_command.h"
#include "kis_command_utils.h"
#include "commands/kis_deselect_global_selection_command.h"

#include "kis_algebra_2d.h"
#include "kis_config.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include <QMenu>


KisSelectionToolHelper::KisSelectionToolHelper(KisCanvas2* canvas, const KUndo2MagicString& name)
        : m_canvas(canvas)
        , m_name(name)
{
    m_image = m_canvas->viewManager()->image();
}

KisSelectionToolHelper::~KisSelectionToolHelper()
{
}

struct LazyInitGlobalSelection : public KisTransactionBasedCommand {
    LazyInitGlobalSelection(KisView *view) : m_view(view) {}
    KisView *m_view;

    KUndo2Command* paint() override {
        return !m_view->selection() ?
            new KisSetEmptyGlobalSelectionCommand(m_view->image()) : 0;
    }
};

void KisSelectionToolHelper::selectPixelSelection(KisPixelSelectionSP selection, SelectionAction action)
{
    KisView* view = m_canvas->imageView();

    if (selection->selectedExactRect().isEmpty()) {
        m_canvas->viewManager()->selectionManager()->deselect();
        return;
    }

    KisProcessingApplicator applicator(view->image(),
                                       0 /* we need no automatic updates */,
                                       KisProcessingApplicator::SUPPORTS_WRAPAROUND_MODE,
                                       KisImageSignalVector() << ModifiedSignal,
                                       m_name);

    applicator.applyCommand(new LazyInitGlobalSelection(view));

    struct ApplyToPixelSelection : public KisTransactionBasedCommand {
        ApplyToPixelSelection(KisView *view,
                              KisPixelSelectionSP selection,
                              SelectionAction action) : m_view(view),
                                                        m_selection(selection),
                                                        m_action(action) {}
        KisView *m_view;
        KisPixelSelectionSP m_selection;
        SelectionAction m_action;

        KUndo2Command* paint() override {

            KisPixelSelectionSP pixelSelection = m_view->selection()->pixelSelection();
            KIS_ASSERT_RECOVER(pixelSelection) { return 0; }

            bool hasSelection = !pixelSelection->isEmpty();

            KisSelectionTransaction transaction(pixelSelection);

            if (!hasSelection && m_action == SELECTION_SYMMETRICDIFFERENCE) {
                m_action = SELECTION_REPLACE;
            }

            if (!hasSelection && m_action == SELECTION_SUBTRACT) {
                pixelSelection->invert();
            }

            pixelSelection->applySelection(m_selection, m_action);

            QRect dirtyRect = m_view->image()->bounds();
            if (hasSelection &&
                m_action != SELECTION_REPLACE &&
                m_action != SELECTION_INTERSECT &&
                m_action != SELECTION_SYMMETRICDIFFERENCE) {

                dirtyRect = m_selection->selectedRect();
            }
            m_view->selection()->updateProjection(dirtyRect);

            KUndo2Command *savedCommand = transaction.endAndTake();
            pixelSelection->setDirty(dirtyRect);

            if (m_view->selection()->selectedExactRect().isEmpty()) {
                KisCommandUtils::CompositeCommand *cmd = new KisCommandUtils::CompositeCommand();
                cmd->addCommand(savedCommand);
                cmd->addCommand(new KisDeselectGlobalSelectionCommand(m_view->image()));
                savedCommand = cmd;
            }

            return savedCommand;
        }
    };

    applicator.applyCommand(new ApplyToPixelSelection(view, selection, action));
    applicator.end();
}

void KisSelectionToolHelper::addSelectionShape(KoShape* shape, SelectionAction action)
{
    QList<KoShape*> shapes;
    shapes.append(shape);
    addSelectionShapes(shapes, action);
}

void KisSelectionToolHelper::addSelectionShapes(QList< KoShape* > shapes, SelectionAction action)
{
    KisView *view = m_canvas->imageView();

    if (view->image()->wrapAroundModePermitted()) {
        view->showFloatingMessage(
            i18n("Shape selection does not fully "
                 "support wraparound mode. Please "
                 "use pixel selection instead"),
                 KisIconUtils::loadIcon("selection-info"));
    }

    KisProcessingApplicator applicator(view->image(),
                                       0 /* we need no automatic updates */,
                                       KisProcessingApplicator::NONE,
                                       KisImageSignalVector() << ModifiedSignal,
                                       m_name);

    applicator.applyCommand(new LazyInitGlobalSelection(view));

    struct ClearPixelSelection : public KisTransactionBasedCommand {
        ClearPixelSelection(KisView *view) : m_view(view) {}
        KisView *m_view;

        KUndo2Command* paint() override {

            KisPixelSelectionSP pixelSelection = m_view->selection()->pixelSelection();
            KIS_ASSERT_RECOVER(pixelSelection) { return 0; }

            KisSelectionTransaction transaction(pixelSelection);
            pixelSelection->clear();
            return transaction.endAndTake();
        }
    };

    if (action == SELECTION_REPLACE || action == SELECTION_DEFAULT) {
        applicator.applyCommand(new ClearPixelSelection(view));
    }

    struct AddSelectionShape : public KisTransactionBasedCommand {
        AddSelectionShape(KisView *view, KoShape* shape, SelectionAction action)
            : m_view(view),
              m_shape(shape),
              m_action(action) {}

        KisView *m_view;
        KoShape* m_shape;
        SelectionAction m_action;

        KUndo2Command* paint() override {
            KUndo2Command *resultCommand = 0;


            KisSelectionSP selection = m_view->selection();
            if (selection) {
                KisShapeSelection * shapeSelection = static_cast<KisShapeSelection*>(selection->shapeSelection());

                if (shapeSelection) {
                    QList<KoShape*> existingShapes = shapeSelection->shapes();

                    if (existingShapes.size() == 1) {
                        KoShape *currentShape = existingShapes.first();
                        QPainterPath path1 = currentShape->absoluteTransformation(0).map(currentShape->outline());
                        QPainterPath path2 = m_shape->absoluteTransformation(0).map(m_shape->outline());

                        QPainterPath path = path2;

                        switch (m_action) {
                        case SELECTION_DEFAULT:
                        case SELECTION_REPLACE:
                            path = path2;
                            break;

                        case SELECTION_INTERSECT:
                            path = path1 & path2;
                            break;

                        case SELECTION_ADD:
                            path = path1 | path2;
                            break;

                        case SELECTION_SUBTRACT:
                            path = path1 - path2;
                            break;
                        case SELECTION_SYMMETRICDIFFERENCE:
                            path = (path1 | path2) - (path1 & path2);
                            break;
                        }

                        KoShape *newShape = KoPathShape::createShapeFromPainterPath(path);
                        newShape->setUserData(new KisShapeSelectionMarker);

                        KUndo2Command *parentCommand = new KUndo2Command();

                        m_view->canvasBase()->shapeController()->removeShape(currentShape, parentCommand);
                        m_view->canvasBase()->shapeController()->addShape(newShape, 0, parentCommand);

                        resultCommand = parentCommand;
                    }
                }
            }


            if (!resultCommand) {
                /**
                 * Mark a shape that it belongs to a shape selection
                 */
                if(!m_shape->userData()) {
                    m_shape->setUserData(new KisShapeSelectionMarker);
                }

                resultCommand = m_view->canvasBase()->shapeController()->addShape(m_shape, 0);
            }

            return resultCommand;
        }
    };

    Q_FOREACH (KoShape* shape, shapes) {
        applicator.applyCommand(
            new KisGuiContextCommand(new AddSelectionShape(view, shape, action), view));
    }
    applicator.end();
}

bool KisSelectionToolHelper::canShortcutToDeselect(const QRect &rect, SelectionAction action)
{
    return rect.isEmpty() && (action == SELECTION_INTERSECT || action == SELECTION_REPLACE);
}

bool KisSelectionToolHelper::canShortcutToNoop(const QRect &rect, SelectionAction action)
{
    return rect.isEmpty() && action == SELECTION_ADD;
}

bool KisSelectionToolHelper::tryDeselectCurrentSelection(const QRectF selectionViewRect, SelectionAction action)
{
    bool result = false;

    if (KisAlgebra2D::maxDimension(selectionViewRect) < KisConfig(true).selectionViewSizeMinimum() &&
        (action == SELECTION_INTERSECT || action == SELECTION_SYMMETRICDIFFERENCE || action == SELECTION_REPLACE)) {

        // Queueing this action to ensure we avoid a race condition when unlocking the node system
        QTimer::singleShot(0, m_canvas->viewManager()->selectionManager(), SLOT(deselect()));
        result = true;
    }

    return result;
}


QMenu* KisSelectionToolHelper::getSelectionContextMenu(KisCanvas2* canvas)
{
    QMenu *m_contextMenu = new QMenu();

    KActionCollection *actionCollection = canvas->viewManager()->actionCollection();

    m_contextMenu->addAction(actionCollection->action("deselect"));
    m_contextMenu->addAction(actionCollection->action("invert"));
    m_contextMenu->addAction(actionCollection->action("select_all"));

    m_contextMenu->addSeparator();

    m_contextMenu->addAction(actionCollection->action("cut_selection_to_new_layer"));
    m_contextMenu->addAction(actionCollection->action("copy_selection_to_new_layer"));

    m_contextMenu->addSeparator();

    KisSelectionSP selection = canvas->viewManager()->selection();
    if (selection && canvas->viewManager()->selectionEditable()) {
        m_contextMenu->addAction(actionCollection->action("edit_selection"));

        if (!selection->hasShapeSelection()) {
            m_contextMenu->addAction(actionCollection->action("convert_to_vector_selection"));
        } else {
            m_contextMenu->addAction(actionCollection->action("convert_to_raster_selection"));
        }

        QMenu *transformMenu = m_contextMenu->addMenu(i18n("Transform"));
        transformMenu->addAction(actionCollection->action("KisToolTransform"));
        transformMenu->addAction(actionCollection->action("selectionscale"));
        transformMenu->addAction(actionCollection->action("growselection"));
        transformMenu->addAction(actionCollection->action("shrinkselection"));
        transformMenu->addAction(actionCollection->action("borderselection"));
        transformMenu->addAction(actionCollection->action("smoothselection"));
        transformMenu->addAction(actionCollection->action("featherselection"));
        transformMenu->addAction(actionCollection->action("stroke_selection"));

        m_contextMenu->addSeparator();
    }

    m_contextMenu->addAction(actionCollection->action("resizeimagetoselection"));

    m_contextMenu->addSeparator();

    m_contextMenu->addAction(actionCollection->action("toggle_display_selection"));
    m_contextMenu->addAction(actionCollection->action("show-global-selection-mask"));

    return m_contextMenu;
}

SelectionMode KisSelectionToolHelper::tryOverrideSelectionMode(KisSelectionSP activeSelection, SelectionMode currentMode, SelectionAction currentAction) const
{
    if (currentAction != SELECTION_DEFAULT && currentAction != SELECTION_REPLACE) {
        if (activeSelection) {
            currentMode = activeSelection->hasShapeSelection() ? SHAPE_PROTECTION : PIXEL_SELECTION;
        }
    }

    return currentMode;
}
