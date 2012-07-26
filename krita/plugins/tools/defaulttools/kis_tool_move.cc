/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tool_move.h"

#include <QPoint>
#include <kundo2command.h>

#include <klocale.h>

#include "KoColorSpace.h"
#include "KoColor.h"
#include "KoCompositeOp.h"

#include "kis_cursor.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_painter.h"
#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_node_manager.h"
#include "kis_image.h"

#include <kis_transaction.h>
#include <commands/kis_image_layer_add_command.h>
#include <commands/kis_deselect_global_selection_command.h>
#include "strokes/move_stroke_strategy.h"

KisToolMove::KisToolMove(KoCanvasBase * canvas)
        :  KisTool(canvas, KisCursor::moveCursor())
{
    setObjectName("tool_move");
    m_optionsWidget = 0;
}

KisToolMove::~KisToolMove()
{
}

void KisToolMove::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
}

// recursively search a node with a non-transparent pixel
KisNodeSP findNode(KisNodeSP node, const QPoint &point, bool wholeGroup)
{
    KisNodeSP foundNode = 0;
    while (node) {
        KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());

        if (!layer || !layer->isEditable()) {
            node = node->prevSibling();
            continue;
        }

        KoColor color(layer->projection()->colorSpace());
        layer->projection()->pixel(point.x(), point.y(), &color);

        if(color.opacityU8() != OPACITY_TRANSPARENT_U8) {
            if (layer->inherits("KisGroupLayer")) {
                // if this is a group and the pixel is transparent,
                // don't even enter it

                foundNode = findNode(node->lastChild(), point, wholeGroup);
            }
            else {
                foundNode = !wholeGroup ? node : node->parent();
            }

        }

        if (foundNode) break;

        node = node->prevSibling();
    }

    return foundNode;
}

KisLayerSP createSelectionCopy(KisLayerSP srcLayer, KisSelectionSP selection, KisImageWSP image, KisStrokeId strokeId)
{
    QRect copyRect = srcLayer->extent() | selection->selectedRect();

    KisPaintDeviceSP device = new KisPaintDevice(srcLayer->colorSpace());
    KisPainter gc(device);
    gc.setSelection(selection);
    gc.setCompositeOp(COMPOSITE_OVER);
    gc.setOpacity(OPACITY_OPAQUE_U8);
    gc.bitBlt(copyRect.topLeft(), srcLayer->paintDevice(), copyRect);
    gc.end();

    KisTransaction transaction("cut", srcLayer->paintDevice());
    srcLayer->paintDevice()->clearSelection(selection);
    image->addJob(strokeId,
                  new KisStrokeStrategyUndoCommandBased::Data(transaction.endAndTake()));

    image->addJob(strokeId,
        new KisStrokeStrategyUndoCommandBased::Data(
            new KisDeselectGlobalSelectionCommand(image)));

    KisPaintLayerSP newLayer =
        new KisPaintLayer(image, srcLayer->name() + " (moved)",
                          srcLayer->opacity(), device);

    newLayer->setCompositeOp(srcLayer->compositeOpId());

    // No updates while we adding a layer, so let's be "exclusive"
    image->addJob(strokeId,
        new KisStrokeStrategyUndoCommandBased::Data(
            new KisImageLayerAddCommand(image, newLayer,
                                        srcLayer->parent(), srcLayer),
            false,
            KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE));

    return newLayer;
}

void KisToolMove::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton,
                          Qt::ControlModifier | Qt::ShiftModifier)) {

        setMode(KisTool::PAINT_MODE);

        KisNodeSP node;
        KisImageWSP image = currentImage();
        if (!image || !image->rootLayer()) {
            return;
        }

        QPoint pos = convertToPixelCoord(event).toPoint();
        KisSelectionSP selection = currentSelection();

        if(!m_optionsWidget->radioSelectedLayer->isChecked() &&
           event->modifiers() != Qt::ControlModifier) {

            bool wholeGroup = !selection &&
                (m_optionsWidget->radioGroup->isChecked() ||
                 event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier));

            node = findNode(image->rootLayer(), pos, wholeGroup);
        }

        if(!node) {
            node = currentNode();
            if (!node) {
                return;
            }
        }

        if (!nodeEditable()) {
            return;
        }

        /**
         * NOTE: we use deferred initialization of the node of
         * the stroke here. First, we set the node to null in
         * the constructor, use jobs of undo-command-based
         * strategy to initialize the stroke and then we set up
         * the node itself.
         */
        MoveStrokeStrategy *strategy =
            new MoveStrokeStrategy(0, image.data(),
                                   image->postExecutionUndoAdapter(),
                                   image->undoAdapter());

        m_strokeId = image->startStroke(strategy);

        if (node->inherits("KisLayer") &&
            !node->inherits("KisGroupLayer") &&
            node->paintDevice() &&
            selection &&
            !selection->isTotallyUnselected(image->bounds())) {

            KisLayerSP oldLayer = dynamic_cast<KisLayer*>(node.data());
            KisLayerSP newLayer = createSelectionCopy(oldLayer, selection, image, m_strokeId);

            node = newLayer;
        }

        // the deferred initialization itself
        strategy->setNode(node);
        strategy = 0;

        m_dragStart = pos;
        m_lastDragPos = m_dragStart;
    }
    else {
        KisTool::mousePressEvent(event);
    }
}

void KisToolMove::mouseMoveEvent(KoPointerEvent *event)
{
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        if (!m_strokeId)
        {
            return;
        }

        QPoint pos = convertToPixelCoord(event).toPoint();
        pos = applyModifiers(event->modifiers(), pos);
        drag(pos);

        notifyModified();
    }
    else {
        KisTool::mouseMoveEvent(event);
    }
}

void KisToolMove::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        if (!m_strokeId)
        {
            return;
        }

        QPoint pos = convertToPixelCoord(event).toPoint();
        pos = applyModifiers(event->modifiers(), pos);
        drag(pos);

        KisImageWSP image = currentImage();
        image->endStroke(m_strokeId);
        m_strokeId.clear();

        currentImage()->setModified();
    }
    else {
        KisTool::mouseReleaseEvent(event);
    }
}

void KisToolMove::drag(const QPoint& newPos)
{
    KisImageWSP image = currentImage();

    QPoint offset = newPos - m_lastDragPos;
    m_lastDragPos = newPos;

    image->addJob(m_strokeId,
                  new MoveStrokeStrategy::Data(offset));
}

QWidget* KisToolMove::createOptionWidget()
{
    m_optionsWidget = new MoveToolOptionsWidget(0);
    m_optionsWidget->setFixedHeight(m_optionsWidget->sizeHint().height());
    return m_optionsWidget;
}

QPoint KisToolMove::applyModifiers(Qt::KeyboardModifiers modifiers, QPoint pos)
{
    QPoint adjustedPos = pos;
    if (modifiers & Qt::AltModifier || modifiers & Qt::ControlModifier) {

        if (qAbs(pos.x() - m_dragStart.x()) > qAbs(pos.y() - m_dragStart.y()))
            adjustedPos.setY(m_dragStart.y());
        else
            adjustedPos.setX(m_dragStart.x());
    }
    return adjustedPos;
}
