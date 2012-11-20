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

#include "KoColorSpace.h"

#include "kis_cursor.h"
#include "kis_selection.h"
#include "kis_canvas2.h"
#include "kis_image.h"

#include "kis_paint_layer.h"
#include "strokes/move_stroke_strategy.h"
#include "strokes/move_selection_stroke_strategy.h"

void MoveToolOptionsWidget::connectSignals()
{
    connect(radioSelectedLayer, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationChanged()));
    connect(radioFirstLayer, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationChanged()));
    connect(radioGroup, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationChanged()));
}

KisToolMove::KisToolMove(KoCanvasBase * canvas)
        :  KisTool(canvas, KisCursor::moveCursor())
{
    setObjectName("tool_move");
    m_optionsWidget = 0;
}

KisToolMove::~KisToolMove()
{
    endStroke();
}

void KisToolMove::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
}

void KisToolMove::deactivate()
{
    endStroke();
    KisTool::deactivate();
}

void KisToolMove::requestStrokeEnd()
{
    endStroke();
}

void KisToolMove::requestStrokeCancellation()
{
    cancelStroke();
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

void KisToolMove::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton,
                          Qt::ControlModifier | Qt::ShiftModifier)) {

        setMode(KisTool::PAINT_MODE);

        QPoint pos = convertToPixelCoord(event).toPoint();
        m_dragStart = pos;
        m_lastDragPos = m_dragStart;

        if (m_strokeId) return;

        KisNodeSP node;
        KisImageSP image = this->image();

        KisSelectionSP selection = currentSelection();

        if(!m_optionsWidget->radioSelectedLayer->isChecked() &&
           event->modifiers() != Qt::ControlModifier) {

            bool wholeGroup = !selection &&
                (m_optionsWidget->radioGroup->isChecked() ||
                 event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier));

            node = findNode(image->root(), pos, wholeGroup);
        }

        if((!node && !(node = currentNode())) || !node->isEditable()) return;


        KisStrokeStrategy *strategy;

        KisPaintLayerSP paintLayer =
            dynamic_cast<KisPaintLayer*>(node.data());

        if (paintLayer && selection &&
            !selection->isTotallyUnselected(image->bounds())) {

            strategy =
                new MoveSelectionStrokeStrategy(paintLayer,
                                                selection,
                                                image.data(),
                                                image->postExecutionUndoAdapter());
        } else {
            strategy =
                new MoveStrokeStrategy(node, image.data(),
                                       image->postExecutionUndoAdapter(),
                                       image->undoAdapter());
        }

        m_strokeId = image->startStroke(strategy);
    }
    else {
        KisTool::mousePressEvent(event);
    }
}

void KisToolMove::mouseMoveEvent(KoPointerEvent *event)
{
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        if (!m_strokeId) return;

        QPoint pos = convertToPixelCoord(event).toPoint();
        pos = applyModifiers(event->modifiers(), pos);
        drag(pos);
    }
    else {
        KisTool::mouseMoveEvent(event);
    }
}

void KisToolMove::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);
        if (!m_strokeId) return;

        QPoint pos = convertToPixelCoord(event).toPoint();
        pos = applyModifiers(event->modifiers(), pos);
        drag(pos);
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

void KisToolMove::endStroke()
{
    if (!m_strokeId) return;

    KisImageWSP image = currentImage();
    image->endStroke(m_strokeId);
    m_strokeId.clear();
}

void KisToolMove::cancelStroke()
{
    if (!m_strokeId) return;

    KisImageWSP image = currentImage();
    image->cancelStroke(m_strokeId);
    m_strokeId.clear();
}

QWidget* KisToolMove::createOptionWidget()
{
    m_optionsWidget = new MoveToolOptionsWidget(0);
    m_optionsWidget->setFixedHeight(m_optionsWidget->sizeHint().height());

    connect(m_optionsWidget, SIGNAL(sigConfigurationChanged()), SLOT(endStroke()));

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
