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


#include "kis_cursor.h"
#include "kis_selection.h"
#include "kis_canvas2.h"
#include "kis_image.h"

#include "kis_tool_utils.h"
#include "kis_paint_layer.h"
#include "strokes/move_stroke_strategy.h"
#include "kis_tool_movetooloptionswidget.h"
#include "strokes/move_selection_stroke_strategy.h"

KisToolMove::KisToolMove(KoCanvasBase * canvas)
        :  KisTool(canvas, KisCursor::moveCursor())
{
    setObjectName("tool_move");
    m_optionsWidget = 0;
    m_moveToolMode = MoveSelectedLayer;
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

void KisToolMove::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton,
                          Qt::ControlModifier | Qt::ShiftModifier)) {

        setMode(KisTool::PAINT_MODE);

        QPoint pos = convertToPixelCoord(event).toPoint();
        m_dragStart = pos;
        m_lastDragPos = m_dragStart;

        KisNodeSP node;
        KisImageSP image = this->image();

        KisSelectionSP selection = currentSelection();

        if(!m_moveToolMode == MoveSelectedLayer &&
           event->modifiers() != Qt::ControlModifier) {

            bool wholeGroup = !selection &&
                (m_moveToolMode == MoveGroup ||
                 event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier));

            node = KisToolUtils::findNode(image->root(), pos, wholeGroup);
        }

        if((!node && !(node = currentNode())) || !node->isEditable()) return;

        /**
         * If the target node has changed, the stroke should be
         * restarted. Otherwise just continue processing current node.
         */
        if (m_strokeId) {
            if (node == m_currentlyProcessingNode) {
                return;
            } else {
                endStroke();
            }
        }

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
        m_currentlyProcessingNode = node;
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
    m_currentlyProcessingNode.clear();
}

void KisToolMove::cancelStroke()
{
    if (!m_strokeId) return;

    KisImageWSP image = currentImage();
    image->cancelStroke(m_strokeId);
    m_strokeId.clear();
    m_currentlyProcessingNode.clear();
}

QWidget* KisToolMove::createOptionWidget()
{
    m_optionsWidget = new MoveToolOptionsWidget(0);
    m_optionsWidget->setFixedHeight(m_optionsWidget->sizeHint().height());

    connect(m_optionsWidget->radioSelectedLayer, SIGNAL(toggled(bool)),
            this, SLOT(slotWidgetRadioToggled(bool)));
    connect(m_optionsWidget->radioFirstLayer, SIGNAL(toggled(bool)),
            this, SLOT(slotWidgetRadioToggled(bool)));
    connect(m_optionsWidget->radioGroup, SIGNAL(toggled(bool)),
            this, SLOT(slotWidgetRadioToggled(bool)));

    //connect(m_optionsWidget, SIGNAL(sigConfigurationChanged()), SLOT(endStroke()));

    return m_optionsWidget;
}

void KisToolMove::setMoveToolMode(KisToolMove::MoveToolMode newMode)
{
    m_moveToolMode = newMode;
}

KisToolMove::MoveToolMode KisToolMove::moveToolMode() const
{
    return m_moveToolMode;
}

void KisToolMove::slotWidgetRadioToggled(bool checked)
{
    Q_UNUSED(checked);
    QObject* from = sender();
    if(from == m_optionsWidget->radioSelectedLayer)
        setMoveToolMode(MoveSelectedLayer);
    else if(from == m_optionsWidget->radioFirstLayer)
        setMoveToolMode(MoveFirstLayer);
    else if(from == m_optionsWidget->radioGroup)
        setMoveToolMode(MoveGroup);
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
