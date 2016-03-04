/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *                2016 Michael Abrahams <miabraha@gmail.com>
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
#include "kis_resources_snapshot.h"
#include "kis_action_registry.h"

KisToolMove::KisToolMove(KoCanvasBase * canvas)
        :  KisTool(canvas, KisCursor::moveCursor())
{
    setObjectName("tool_move");
    m_optionsWidget = 0;
    m_moveInProgress = false;
    QAction *a;

    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    a = actionRegistry->makeQAction("movetool-move-up", this);
    addAction("movetool-move-up", a);
    connect(a, &QAction::triggered, [&](){moveDiscrete(MoveDirection::Up, false);});

    a = actionRegistry->makeQAction("movetool-move-down", this);
    addAction("movetool-move-down", a);
    connect(a, &QAction::triggered, [&](){moveDiscrete(MoveDirection::Down, false);});

    a = actionRegistry->makeQAction("movetool-move-left", this);
    addAction("movetool-move-left", a);
    connect(a, &QAction::triggered, [&](){moveDiscrete(MoveDirection::Left, false);});

    a = actionRegistry->makeQAction("movetool-move-right", this);
    addAction("movetool-move-right", a);
    connect(a, &QAction::triggered, [&](){moveDiscrete(MoveDirection::Right, false);});

    a = actionRegistry->makeQAction("movetool-move-up-more", this);
    addAction("movetool-move-up-more", a);
    connect(a, &QAction::triggered, [&](){moveDiscrete(MoveDirection::Up, true);});

    a = actionRegistry->makeQAction("movetool-move-down-more", this);
    addAction("movetool-move-down-more", a);
    connect(a, &QAction::triggered, [&](){moveDiscrete(MoveDirection::Down, true);});

    a = actionRegistry->makeQAction("movetool-move-left-more", this);
    addAction("movetool-move-left-more", a);
    connect(a, &QAction::triggered, [&](){moveDiscrete(MoveDirection::Left, true);});

    a = actionRegistry->makeQAction("movetool-move-right-more", this);
    addAction("movetool-move-right-more", a);
    connect(a, &QAction::triggered, [&](){moveDiscrete(MoveDirection::Right, true);});

}

KisToolMove::~KisToolMove()
{
    endStroke();
}

void KisToolMove::resetCursorStyle()
{
    KisTool::resetCursorStyle();

    overrideCursorIfNotEditable();
}

void KisToolMove::moveDiscrete(MoveDirection direction, bool big)
{
    if (mode() == KisTool::PAINT_MODE) return;  // Don't interact with dragging

    setMode(KisTool::PAINT_MODE);

    KisNodeSP node;
    KisImageSP image = this->image();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image, currentNode(), 0, this->canvas()->resourceManager());
    KisSelectionSP selection = resources->activeSelection();

    // Move current layer by default.
    if ((!node && !(node = resources->currentNode())) || !node->isEditable()) {
        return;
    }


    // If node has changed, end current stroke.
    if (m_strokeId && (node != m_currentlyProcessingNode)) {
        endStroke();
    }


    /**
     * Begin a new stroke if necessary.
     */
    if (!m_strokeId) {
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
                                       image->postExecutionUndoAdapter());
        }

        m_strokeId = image->startStroke(strategy);
        m_currentlyProcessingNode = node;
        m_accumulatedOffset = QPoint();
    }

    // Larger movement if "shift" key is pressed.
    qreal scale = big ? m_optionsWidget->moveScale() : 1.0;
    qreal moveStep = m_optionsWidget->moveStep() * scale;

    QPoint offset = direction == Up   ? QPoint( 0, -moveStep) :
                    direction == Down ? QPoint( 0,  moveStep) :
                    direction == Left ? QPoint(-moveStep,  0) :
                                        QPoint( moveStep,  0) ;

    image->addJob(m_strokeId, new MoveStrokeStrategy::Data(m_accumulatedOffset + offset));
    m_accumulatedOffset += offset;


    m_moveInProgress = false;
    emit moveInProgressChanged();
    setMode(KisTool::HOVER_MODE);
}

void KisToolMove::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisTool::activate(toolActivation, shapes);
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

void KisToolMove::beginPrimaryAction(KoPointerEvent *event)
{
    startAction(event, moveToolMode());
}

void KisToolMove::continuePrimaryAction(KoPointerEvent *event)
{
    continueAction(event);
}

void KisToolMove::endPrimaryAction(KoPointerEvent *event)
{
    endAction(event);
}

void KisToolMove::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action == PickFgNode) {
        MoveToolMode mode = moveToolMode();

        if (mode == MoveSelectedLayer || mode == MoveGroup) {
            mode = MoveFirstLayer;
        } else if (mode == MoveFirstLayer) {
            mode = MoveSelectedLayer;
        }

        startAction(event, mode);
    } else if (action == PickFgImage) {
        startAction(event, MoveGroup);
    } else {
        KisTool::beginAlternateAction(event, action);
    }
}

void KisToolMove::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action == PickFgNode || action == PickFgImage) {
        continueAction(event);
    } else {
        KisTool::continueAlternateAction(event, action);
    }
}

void KisToolMove::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action == PickFgNode || action == PickFgImage) {
        endAction(event);
    } else {
        KisTool::endAlternateAction(event, action);
    }
}

void KisToolMove::startAction(KoPointerEvent *event, MoveToolMode mode)
{
    QPoint pos = convertToPixelCoordAndSnap(event).toPoint();
    m_dragStart = pos;
    m_moveInProgress = true;
    emit moveInProgressChanged();

    KisNodeSP node;
    KisImageSP image = this->image();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image, currentNode(), 0, this->canvas()->resourceManager());
    KisSelectionSP selection = resources->activeSelection();

    if (mode != MoveSelectedLayer) {
        bool wholeGroup = !selection &&  mode == MoveGroup;
        node = KisToolUtils::findNode(image->root(), pos, wholeGroup);
    }

    if ((!node && !(node = resources->currentNode())) || !node->isEditable()) {
        event->ignore();
        return;
    }

    setMode(KisTool::PAINT_MODE);

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
                                   image->postExecutionUndoAdapter());
    }

    m_strokeId = image->startStroke(strategy);
    m_currentlyProcessingNode = node;
    m_accumulatedOffset = QPoint();
}

void KisToolMove::continueAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    if (!m_strokeId) return;

    QPoint pos = convertToPixelCoordAndSnap(event).toPoint();
    pos = applyModifiers(event->modifiers(), pos);
    drag(pos);
}

void KisToolMove::endAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);
    if (!m_strokeId) return;

    QPoint pos = convertToPixelCoordAndSnap(event).toPoint();
    pos = applyModifiers(event->modifiers(), pos);
    drag(pos);

    m_accumulatedOffset += pos - m_dragStart;
}

void KisToolMove::drag(const QPoint& newPos)
{
    KisImageWSP image = currentImage();

    QPoint offset = m_accumulatedOffset + newPos - m_dragStart;

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
    m_moveInProgress = false;
    emit moveInProgressChanged();
}

void KisToolMove::cancelStroke()
{
    if (!m_strokeId) return;

    KisImageWSP image = currentImage();
    image->cancelStroke(m_strokeId);
    m_strokeId.clear();
    m_currentlyProcessingNode.clear();
    m_moveInProgress = false;
    emit moveInProgressChanged();
}

QWidget* KisToolMove::createOptionWidget()
{
    if (!currentImage())
        return 0;

    m_optionsWidget = new MoveToolOptionsWidget(0, currentImage()->xRes(), toolId());
    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(m_optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    m_optionsWidget->layout()->addWidget(specialSpacer);

    m_optionsWidget->setFixedHeight(m_optionsWidget->sizeHint().height());

    return m_optionsWidget;
}

KisToolMove::MoveToolMode KisToolMove::moveToolMode() const
{
    if (m_optionsWidget)
        return m_optionsWidget->mode();
    return MoveSelectedLayer;
}

bool KisToolMove::moveInProgress() const
{
    return m_moveInProgress;
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
