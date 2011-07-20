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

#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "KoColorSpace.h"
#include "KoColor.h"
#include "KoProperties.h"
#include "KoCompositeOp.h"

#include "commands/kis_node_commands.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_undo_adapter.h"
#include "kis_node.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_types.h"
#include "kis_painter.h"
#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_node_manager.h"
#include "kis_selection_manager.h"
#include <commands/kis_image_layer_add_command.h>
#include <kis_transaction.h>
#include <commands/kis_deselect_global_selection_command.h>

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

// recursive search a node with a non-  transparent pixel
KisNodeSP findNode(KisNodeSP node, int x, int y)
{

    // if this is a group and the pixel is transparent, don't even enter it
    if (node->inherits("KisGroupLayer")) {
        KisGroupLayerSP layer = dynamic_cast<KisGroupLayer*>(node.data());
        const KoColorSpace* cs = layer->projection()->colorSpace();
        KoColor color(cs);
        layer->projection()->pixel(x, y, &color);
        if (cs->opacityU8(color.data()) == OPACITY_TRANSPARENT_U8) {
            return 0;
        }
        node = node->lastChild();
    }

    KisNodeSP foundNode = 0;
    while (node) {
        if (node->isEditable()) {
            if (node->inherits("KisGroupLayer")) {
                foundNode = findNode(node, x, y);
            } else if (node->inherits("KisLayer")) {
                KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());
                if (layer) {
                    const KoColorSpace* cs = layer->projection()->colorSpace();
                    KoColor color(layer->projection()->colorSpace());
                    layer->projection()->pixel(x, y, &color);

                    // XXX:; have threshold here? Like, only a little bit transparent, we don't select it?
                    if (cs->opacityU8(color.data()) != OPACITY_TRANSPARENT_U8) {
                        foundNode = node;
                    }
                }
            }
            if (foundNode) {
                return foundNode;
            }
        }
        node = node->prevSibling();
    }
    return 0;
}

void KisToolMove::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton,
                          Qt::ControlModifier | Qt::ShiftModifier)) {

        setMode(KisTool::PAINT_MODE);

        KisNodeSP node;
        KisImageWSP image = currentImage();
        if (!image || !image->rootLayer() || image->rootLayer()->childCount() == 0) {
            return;
        }

        QPointF pos = convertToPixelCoord(event);

        // shortcut: if the pixel at projection is transparent, it's all transparent
        const KoColorSpace* cs = image->projection()->colorSpace();
        KoColor color(cs);
        image->projection()->pixel(pos.x(), pos.y(), &color);

        KisSelectionSP selection = currentSelection();

        if (cs->opacityU8(color.data()) == OPACITY_TRANSPARENT_U8 ||
            m_optionsWidget->radioSelectedLayer->isChecked() ||
            event->modifiers() == Qt::ControlModifier) {

            node = currentNode();
        } else {

            // iterate over all layers at the current position, get the pixel at x,y and check whether it's transparent
            node = image->rootLayer()->lastChild();
            node = findNode(node, pos.x(), pos.y());

            // if there is a selection, we cannot move the group
            if (node && !selection && (m_optionsWidget->radioGroup->isChecked() || event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))) {
                node = node->parent();
            }
        }
        Q_ASSERT(node);
        // shouldn't happen
        if (!node) {
            node = currentNode();
        }

        image->undoAdapter()->beginMacro(i18n("Move"));

        if (selection && !selection->isTotallyUnselected(image->bounds()) && !selection->isDeselected() && !node->inherits("KisSelectionMask")) {
            // Create a temporary layer with the contents of the selection of the current layer.
            Q_ASSERT(!node->inherits("KisGroupLayer"));

            KisLayerSP oldLayer = dynamic_cast<KisPaintLayer*>(node.data());

            // we can only do the selection thing if the source layer was a paint layer, not a mask
            if (oldLayer) {

                KisPaintDeviceSP dev = new KisPaintDevice(oldLayer->colorSpace());

                // copy the contents to the new device
                KisPainter gc(dev);
                gc.setSelection(selection);
                gc.setCompositeOp(COMPOSITE_OVER);
                gc.setOpacity(OPACITY_OPAQUE_U8);
                QRect rc = oldLayer->extent();
                gc.bitBlt(rc.topLeft(), oldLayer->paintDevice(), rc);
                gc.end();

                {
                    KisTransaction transaction("cut", oldLayer->paintDevice());
                    // clear the old layer
                    oldLayer->paintDevice()->clearSelection(selection);
                    transaction.commit(image->undoAdapter());

                    if (image->globalSelection()) {
                        KisDeselectGlobalSelectionCommand* cmd = new KisDeselectGlobalSelectionCommand(image);
                        image->undoAdapter()->addCommand(cmd);
                    }
                }

                KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
                KisView2* view = 0;
                if (kisCanvas) {
                    view = kisCanvas->view();
                }

                // create the new layer and add it.
                KisPaintLayerSP layer = new KisPaintLayer(image,
                        node->name() + "(moved)",
                        oldLayer->opacity(),
                        dev);
                image->undoAdapter()->addCommand(new KisImageLayerAddCommand(image, layer, node->parent(), node));
                view->nodeManager()->activateNode(layer);
                m_targetLayer = node;
                m_selectedNode = layer;
            }
        } else {
            // No selection
            m_selectedNode = node;
            m_targetLayer = 0;
        }

        /**
         * FIXME: Hack alert:
         * Our iterators don't have guarantees on thread-safety
         * when the offset varies. When it is fixed, remove the locking
         * see: KisIterator::stressTest()
         */
        image->lock();
        m_layerStart.setX(node->x());
        m_layerStart.setY(node->y());
        image->unlock();

        m_layerPosition = m_layerStart;
        m_dragStart = pos.toPoint();
    }
    else {
        KisTool::mousePressEvent(event);
    }
}

void KisToolMove::mouseMoveEvent(KoPointerEvent *event)
{
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        QPoint pos = convertToPixelCoord(event).toPoint();
        if ((event->modifiers() & Qt::AltModifier) || (event->modifiers() & Qt::ControlModifier)) {
            if (fabs(static_cast<double>(pos.x() - m_dragStart.x())) > fabs(static_cast<double>(pos.y() - m_dragStart.y())))
                pos.setY(m_dragStart.y());
            else
                pos.setX(m_dragStart.x());
        }
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

        if (m_selectedNode) {
            QPoint pos = convertToPixelCoord(event).toPoint();
            drag(pos);

            KUndo2Command *cmd = new KisNodeMoveCommand(m_selectedNode, m_layerStart, m_layerPosition, currentImage());
            Q_CHECK_PTR(cmd);

            canvas()->addCommand(cmd);
            currentImage()->undoAdapter()->endMacro();
        }
        currentImage()->setModified();
    }
    else {
        KisTool::mouseReleaseEvent(event);
    }
}

void KisToolMove::moveNode(KisNodeSP node, int x, int y) 
{
    node->setX(node->x() + x);
    node->setY(node->y() + y);
    if (node->childCount() > 0 ) {
        // Move all child nodes as well
        KoProperties props;
        foreach(KisNodeSP node, m_selectedNode->childNodes(QStringList(), props)) {
            moveNode(node, x, y);
        }
        
    }    
}

void KisToolMove::drag(const QPoint& original)
{
    // original is the position of the user chosen handle point
    if (m_selectedNode) {
        QPoint pos = original;
        QRect rc;

        pos -= m_dragStart; // convert to delta
        rc = m_selectedNode->extent();

        // FIXME: see comment in KisToolMove::mousePressEvent()
        KisImageWSP image = currentImage();
        image->lock();
        moveNode(m_selectedNode, pos.x(), pos.y());
        image->unlock();

        rc = rc.unite(m_selectedNode->extent());

        m_layerPosition = QPoint(m_selectedNode->x(), m_selectedNode->y());
        m_dragStart = original;

        if (m_selectedNode->inherits("KisSelectionMask")) {
            currentImage()->undoAdapter()->emitSelectionChanged();
        }

        m_selectedNode->setDirty(rc);
    }
}

QWidget* KisToolMove::createOptionWidget()
{
    m_optionsWidget = new MoveToolOptionsWidget(0);
    m_optionsWidget->setFixedHeight(m_optionsWidget->sizeHint().height());
    return m_optionsWidget;
}


QWidget* KisToolMove::optionWidget()
{
    return m_optionsWidget;
}


#include "kis_tool_move.moc"
