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
#include <QUndoCommand>

#include <klocale.h>

#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "KoColorSpace.h"
#include "KoColor.h"
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

KisToolMove::KisToolMove(KoCanvasBase * canvas)
        :  KisTool(canvas, KisCursor::moveCursor())
{
    setObjectName("tool_move");
    m_dragging = false;
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
        if (cs->alpha(color.data()) == OPACITY_TRANSPARENT) {
            return 0;
        }
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
                    if (cs->alpha(color.data()) != OPACITY_TRANSPARENT) {
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

void KisToolMove::mousePressEvent(KoPointerEvent *e)
{
    if (m_canvas && e->button() == Qt::LeftButton) {
        QPointF pos = convertToPixelCoord(e);

        KisNodeSP node;
        KisImageWSP image = currentImage();
        if (!image || !image->rootLayer() || image->rootLayer()->childCount() == 0) {
            return;
        }
        // shortcut: if the pixel at projection is transparent, it's all transparent
        const KoColorSpace* cs = image->projection()->colorSpace();
        KoColor color(cs);
        image->projection()->pixel(pos.x(), pos.y(), &color);

        KisSelectionSP selection = currentSelection();

        if (cs->alpha(color.data()) == OPACITY_TRANSPARENT
                || m_optionsWidget->radioSelectedLayer->isChecked()
                || e->modifiers() == Qt::ControlModifier) {
            node = currentNode();
        } else {

            // iterate over all layers at the current position, get the pixel at x,y and check whether it's transparent
            node = image->rootLayer()->lastChild();
            node = findNode(node, pos.x(), pos.y());

            // if there is a selection, we cannot move the group
            if (!selection && (m_optionsWidget->radioGroup->isChecked() || e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))) {
                node = node->parent();
            }
        }

        // shouldn't happen
        if (!node) {
            node = currentNode();
        }

        if (currentImage()->undo()) {
            currentImage()->undoAdapter()->beginMacro(i18n("Move"));
        }

        if (selection && !selection->isTotallyUnselected(image->bounds())) {
            selection->convertToQImage(0).save("selection.png");
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
                gc.setOpacity(OPACITY_OPAQUE);
                QRect rc = oldLayer->extent();
                gc.bitBlt(rc.topLeft(), oldLayer->paintDevice(), rc);
                gc.end();

                // clear the old layer
                currentImage()->undoAdapter()->addCommand(new KisTransaction("cut", oldLayer->paintDevice()));
                oldLayer->paintDevice()->clearSelection(selection);

                // deselect away the selection???
                selection->clear();

                KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(m_canvas);
                KisView2* view = 0;
                if (kisCanvas) {
                    view = kisCanvas->view();
                }

                // create the new layer and add it.
                KisPaintLayerSP layer = new KisPaintLayer(currentImage(),
                        node->name() + "(moved)",
                        oldLayer->opacity(),
                        dev);
                currentImage()->undoAdapter()->addCommand(new KisImageLayerAddCommand(currentImage(), layer, node->parent(), node));
                view->nodeManager()->activateNode(layer);
                m_targetLayer = node;
                m_selectedNode = layer;
            }
        } else {
            // No selection
            m_selectedNode = node;
            m_targetLayer = 0;
        }

        m_dragging = true;
        m_dragStart = QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
        m_layerStart.setX(node->x());
        m_layerStart.setY(node->y());
        m_layerPosition = m_layerStart;
    }
}

void KisToolMove::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_canvas && m_dragging) {
        QPointF posf = convertToPixelCoord(e);
        QPoint pos = QPoint(static_cast<int>(posf.x()), static_cast<int>(posf.y()));
        if ((e->modifiers() & Qt::AltModifier) || (e->modifiers() & Qt::ControlModifier)) {
            if (fabs(static_cast<double>(pos.x() - m_dragStart.x())) > fabs(static_cast<double>(pos.y() - m_dragStart.y())))
                pos.setY(m_dragStart.y());
            else
                pos.setX(m_dragStart.x());
        }
        drag(pos);

        notifyModified();
    }
}

void KisToolMove::mouseReleaseEvent(KoPointerEvent *e)
{
    if (m_dragging) {
        if (m_canvas && e->button() == Qt::LeftButton) {
            QPointF pos = convertToPixelCoord(e);
            if (m_selectedNode) {
                drag(QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y())));
                m_dragging = false;

                QUndoCommand *cmd = new KisNodeMoveCommand(m_selectedNode, m_layerStart, m_layerPosition);
                Q_CHECK_PTR(cmd);

                m_canvas->addCommand(cmd);
                currentImage()->undoAdapter()->endMacro();
            }
            currentImage()->setModified();
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
        m_selectedNode->setX(m_selectedNode->x() + pos.x());
        m_selectedNode->setY(m_selectedNode->y() + pos.y());

        rc = rc.unite(m_selectedNode->extent());

        m_layerPosition = QPoint(m_selectedNode->x(), m_selectedNode->y());
        m_dragStart = original;

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
