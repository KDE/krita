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
#include "krita_utils.h"

#include <KisViewManager.h>
#include <KisDocument.h>

#include "kis_node_manager.h"
#include "kis_signals_blocker.h"

KisToolMove::KisToolMove(KoCanvasBase * canvas)
        :  KisTool(canvas, KisCursor::moveCursor())
{
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

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

    m_showCoordinatesAction = actionRegistry->makeQAction("movetool-show-coordinates", this);
    addAction("movetool-show-coordinates", m_showCoordinatesAction);
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

bool KisToolMove::startStrokeImpl(MoveToolMode mode, const QPoint *pos)
{
    if (!currentNode()->isEditable()) return false;

    KisNodeSP node;
    KisNodeList nodes;
    KisImageSP image = this->image();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image, currentNode(), this->canvas()->resourceManager());
    KisSelectionSP selection = resources->activeSelection();

    if (mode != MoveSelectedLayer && pos) {
        bool wholeGroup = !selection &&  mode == MoveGroup;
        node = KisToolUtils::findNode(image->root(), *pos, wholeGroup);
        if (node) {
            nodes = {node};
        }
    }

    if (nodes.isEmpty()) {
        nodes = this->selectedNodes();

        KritaUtils::filterContainer<KisNodeList>(nodes,
                                                 [](KisNodeSP node) {
                                                     return node->isEditable();
                                                 });
    }

    if (nodes.size() == 1) {
        node = nodes.first();
    }

    if (nodes.isEmpty()) {
        return false;
    }

    initHandles(nodes);

    /**
     * If the target node has changed, the stroke should be
     * restarted. Otherwise just continue processing current node.
     */
    if (m_strokeId) {
        if (KritaUtils::compareListsUnordered(nodes, m_currentlyProcessingNodes)) {
            return true;
        } else {
            endStroke();
        }
    }

    KisStrokeStrategy *strategy;

    KisPaintLayerSP paintLayer = node ?
        dynamic_cast<KisPaintLayer*>(node.data()) : 0;

    if (paintLayer && selection &&
        !selection->isTotallyUnselected(image->bounds())) {

        strategy =
            new MoveSelectionStrokeStrategy(paintLayer,
                                            selection,
                                            image.data(),
                                            image.data());
    } else {
        strategy =
            new MoveStrokeStrategy(nodes, image.data(), image.data());
    }

    m_strokeId = image->startStroke(strategy);
    m_currentlyProcessingNodes = nodes;
    m_accumulatedOffset = QPoint();

    return true;
}

void KisToolMove::moveDiscrete(MoveDirection direction, bool big)
{
    if (mode() == KisTool::PAINT_MODE) return;  // Don't interact with dragging
    if (!currentNode()->isEditable()) return; // Don't move invisible nodes

    if (startStrokeImpl(MoveSelectedLayer, 0)) {
        setMode(KisTool::PAINT_MODE);
    }

    // Larger movement if "shift" key is pressed.
    qreal scale = big ? m_optionsWidget->moveScale() : 1.0;
    qreal moveStep = m_optionsWidget->moveStep() * scale;

    QPoint offset = direction == Up   ? QPoint( 0, -moveStep) :
                    direction == Down ? QPoint( 0,  moveStep) :
                    direction == Left ? QPoint(-moveStep,  0) :
                                        QPoint( moveStep,  0) ;

    const bool showCoordinates = m_optionsWidget->showCoordinates();

    if (showCoordinates) {
        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        kisCanvas->viewManager()->
            showFloatingMessage(
                i18nc("floating message in move tool",
                      "X: %1 px, Y: %2 px",
                      (m_startPosition + offset).x(),
                      (m_startPosition + offset).y()),
                QIcon(), 1000, KisFloatingMessage::High);
    }

    KisSignalsBlocker b(m_optionsWidget);
    emit moveInNewPosition(m_startPosition + offset);

    m_startPosition += offset;

    image()->addJob(m_strokeId, new MoveStrokeStrategy::Data(m_accumulatedOffset + offset));
    m_accumulatedOffset += offset;

    m_moveInProgress = false;
    emit moveInProgressChanged();
    setMode(KisTool::HOVER_MODE);
}

void KisToolMove::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisTool::activate(toolActivation, shapes);
    QRect totalBounds;

    slotNodeChanged(this->selectedNodes());
}



void KisToolMove::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (m_dragInProgress) {
        QPainterPath handles;
        handles.addRect(m_handlesRect.translated(m_pos));

        QPainterPath path = pixelToView(handles);
        paintToolOutline(&gc, path);
    }
}

void KisToolMove::initHandles(const KisNodeList &nodes)
{
    m_handlesRect = QRect();
    for (KisNodeSP node : nodes) {
        node->exactBounds();
        m_handlesRect |= node->exactBounds();
    }
    if (image()->globalSelection()) {
        m_handlesRect &= image()->globalSelection()->selectedRect();
    }
    if (m_handlesRect.topLeft() != m_startPosition) {
        m_handlesRect.moveTopLeft(m_startPosition);
    }
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

void KisToolMove::requestUndoDuringStroke()
{
    // we shouldn't cancel the stroke on Ctrl+Z, because it will not only
    // cancel the stroke, but also undo the previous command, which we haven't
    // yet pushed to the stack
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
    // Ctrl+Right click toggles between moving current layer and moving layer w/ content
    if (action == PickFgNode || action == PickBgImage) {
        MoveToolMode mode = moveToolMode();

        if (mode == MoveSelectedLayer) {
            mode = MoveFirstLayer;
        } else if (mode == MoveFirstLayer) {
            mode = MoveSelectedLayer;
        }

        startAction(event, mode);
    } else {
        startAction(event, MoveGroup);
    }
}

void KisToolMove::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(action)
    continueAction(event);
}

void KisToolMove::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(action)
    endAction(event);
}

void KisToolMove::startAction(KoPointerEvent *event, MoveToolMode mode)
{
    QPoint pos = convertToPixelCoordAndSnap(event).toPoint();
    m_dragStart = pos;
    m_pos = QPoint();
    m_moveInProgress = true;
    m_dragInProgress = true;
    emit moveInProgressChanged();

    if (startStrokeImpl(mode, &pos)) {
        setMode(KisTool::PAINT_MODE);
    } else {
        event->ignore();
    }
    m_canvas->updateCanvas();
}

void KisToolMove::continueAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    if (!m_strokeId) return;

    QPoint pos = convertToPixelCoordAndSnap(event).toPoint();

    const bool showCoordinates =
        m_optionsWidget ? m_optionsWidget->showCoordinates() : true;

    if (showCoordinates) {
        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        kisCanvas->viewManager()->
                showFloatingMessage(
                    i18nc("floating message in move tool",
                          "X: %1 px, Y: %2 px",
                          (m_startPosition + (pos - m_dragStart)).x(),
                          (m_startPosition + (pos - m_dragStart)).y()),
                    QIcon(), 1000, KisFloatingMessage::High);
    }

    KisSignalsBlocker b(m_optionsWidget);
    emit moveInNewPosition(m_startPosition + (pos - m_dragStart));

    pos = applyModifiers(event->modifiers(), pos);
    drag(pos);
    m_pos = pos - m_dragStart;
    m_canvas->updateCanvas();
}

void KisToolMove::endAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);
    if (!m_strokeId) return;

    QPoint pos = convertToPixelCoordAndSnap(event).toPoint();
    pos = applyModifiers(event->modifiers(), pos);
    drag(pos);
    m_pos = pos - m_dragStart;

    m_dragInProgress = false;
    m_startPosition += pos - m_dragStart;
    m_accumulatedOffset += pos - m_dragStart;
    m_canvas->updateCanvas();
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

    KisImageSP image = currentImage();
    image->endStroke(m_strokeId);
    m_strokeId.clear();
    m_currentlyProcessingNodes.clear();
    m_moveInProgress = false;
    emit moveInProgressChanged();
}

void KisToolMove::cancelStroke()
{
    if (!m_strokeId) return;

    KisImageSP image = currentImage();
    image->cancelStroke(m_strokeId);
    m_strokeId.clear();
    m_currentlyProcessingNodes.clear();
    m_moveInProgress = false;
    emit moveInProgressChanged();

    // we should reset m_startPosition into the original value when
    // the stroke is cancelled
    KisCanvas2 *canvas = dynamic_cast<KisCanvas2*>(this->canvas());
    canvas->viewManager()->blockUntilOperationsFinishedForced(image);
    slotNodeChanged(this->selectedNodes());
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

    connect(m_showCoordinatesAction, SIGNAL(triggered(bool)), m_optionsWidget, SLOT(setShowCoordinates(bool)));
    connect(m_optionsWidget, SIGNAL(showCoordinatesChanged(bool)), m_showCoordinatesAction, SLOT(setChecked(bool)));

    m_showCoordinatesAction->setChecked(m_optionsWidget->showCoordinates());

    m_optionsWidget->slotSetTranslate(m_startPosition);

    connect(m_optionsWidget, SIGNAL(sigSetTranslateX(int)), SLOT(moveBySpinX(int)));
    connect(m_optionsWidget, SIGNAL(sigSetTranslateY(int)), SLOT(moveBySpinY(int)));

    connect(this, SIGNAL(moveInNewPosition(QPoint)), m_optionsWidget, SLOT(slotSetTranslate(QPoint)));

    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());

    connect(kisCanvas->viewManager()->nodeManager(), SIGNAL(sigUiNeedChangeSelectedNodes(KisNodeList)),
            this, SLOT(slotNodeChanged(KisNodeList)));

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
    QPoint move = pos - m_dragStart;

    // Snap to axis
    if (modifiers & Qt::ShiftModifier) {
        move = snapToClosestAxis(move);
    }

    // "Precision mode" - scale down movement by 1/5
    if (modifiers & Qt::AltModifier) {
        const qreal SCALE_FACTOR = .2;
        move = SCALE_FACTOR * move;
    }

    return m_dragStart + move;
}

void KisToolMove::moveBySpinX(int newX)
{
    if (mode() == KisTool::PAINT_MODE) return;  // Don't interact with dragging
    if (!currentNode()->isEditable()) return; // Don't move invisible nodes

    if (startStrokeImpl(MoveSelectedLayer, 0)) {
        setMode(KisTool::PAINT_MODE);
    }

    int offsetX = newX - m_startPosition.x();
    QPoint offset = QPoint(offsetX, 0);

    KisSignalsBlocker b(m_optionsWidget);
    emit moveInNewPosition(m_startPosition + offset);

    image()->addJob(m_strokeId, new MoveStrokeStrategy::Data(m_accumulatedOffset + offset));
    m_accumulatedOffset += offset;
    m_startPosition += offset;

    m_moveInProgress = false;
    emit moveInProgressChanged();
    setMode(KisTool::HOVER_MODE);
}

void KisToolMove::moveBySpinY(int newY)
{
    if (mode() == KisTool::PAINT_MODE) return;  // Don't interact with dragging
    if (!currentNode()->isEditable()) return; // Don't move invisible nodes

    if (startStrokeImpl(MoveSelectedLayer, 0)) {
        setMode(KisTool::PAINT_MODE);
    }

    int offsetY = newY - m_startPosition.y();
    QPoint offset = QPoint(0, offsetY);

    KisSignalsBlocker b(m_optionsWidget);
    emit moveInNewPosition(m_startPosition + offset);

    image()->addJob(m_strokeId, new MoveStrokeStrategy::Data(m_accumulatedOffset + offset));
    m_accumulatedOffset += offset;
    m_startPosition += offset;

    m_moveInProgress = false;
    emit moveInProgressChanged();
    setMode(KisTool::HOVER_MODE);
}

void KisToolMove::slotNodeChanged(KisNodeList nodes)
{
    QRect totalBounds;

    Q_FOREACH (KisNodeSP node, nodes) {
        if (node && node->projection()) {
            totalBounds |= node->projection()->nonDefaultPixelArea();
        }
    }

    if (image()->globalSelection()) {
        totalBounds &= image()->globalSelection()->selectedRect();
    }

    m_startPosition = totalBounds.topLeft();

    if (m_optionsWidget)
    {
        KisSignalsBlocker b(m_optionsWidget);
        m_optionsWidget->slotSetTranslate(m_startPosition);
    }
}
