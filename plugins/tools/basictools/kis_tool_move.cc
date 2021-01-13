/*
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
 *  SPDX-FileCopyrightText: 1999 Michael Koch <koch@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_selection_manager.h"
#include "kis_signals_blocker.h"
#include <boost/operators.hpp>
#include "KisMoveBoundsCalculationJob.h"


struct KisToolMoveState : KisToolChangesTrackerData, boost::equality_comparable<KisToolMoveState>
{
    KisToolMoveState(QPoint _accumulatedOffset) : accumulatedOffset(_accumulatedOffset) {}
    KisToolChangesTrackerData* clone() const override { return new KisToolMoveState(*this); }

    bool operator ==(const KisToolMoveState &rhs) {
        return accumulatedOffset == rhs.accumulatedOffset;
    }

    QPoint accumulatedOffset;
};


KisToolMove::KisToolMove(KoCanvasBase *canvas)
    : KisTool(canvas, KisCursor::moveCursor())
    , m_updateCursorCompressor(100, KisSignalCompressor::FIRST_ACTIVE)
{
    setObjectName("tool_move");

    m_showCoordinatesAction = action("movetool-show-coordinates");
    m_showCoordinatesAction = action("movetool-show-coordinates");
    connect(&m_updateCursorCompressor, SIGNAL(timeout()), this, SLOT(resetCursorStyle()));

    m_optionsWidget = new MoveToolOptionsWidget(0, currentImage()->xRes(), toolId());

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(m_optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    m_optionsWidget->layout()->addWidget(specialSpacer);

    m_optionsWidget->setFixedHeight(m_optionsWidget->sizeHint().height());

    m_showCoordinatesAction->setChecked(m_optionsWidget->showCoordinates());

    m_optionsWidget->slotSetTranslate(m_handlesRect.topLeft() + currentOffset());

    connect(this, SIGNAL(moveInNewPosition(QPoint)), m_optionsWidget, SLOT(slotSetTranslate(QPoint)), Qt::UniqueConnection);
}

KisToolMove::~KisToolMove()
{
    endStroke();
}

void KisToolMove::resetCursorStyle()
{
    if (!isActive()) return;

    bool canMove = true;

    if (m_strokeId && m_currentlyUsingSelection) {
        /// noop; whatever the cursor position, we always show move
        /// cursor, because we don't use 'layer under cursor' mode
        /// for moving selections
    } else if (m_strokeId && !m_currentlyUsingSelection) {
        /// we cannot pick layer's pixel data while the stroke is running,
        /// because it may run in lodN mode; therefore, we delegate this
        /// work to the stroke itself
        if (m_currentMode != MoveSelectedLayer &&
            (m_handlesRect.isEmpty() ||
             !m_handlesRect.translated(currentOffset()).contains(m_lastCursorPos))) {

            image()->addJob(m_strokeId, new MoveStrokeStrategy::PickLayerData(m_lastCursorPos));
            return;
        }
    } else {
        KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(this->image(), currentNode(), canvas()->resourceManager());
        KisSelectionSP selection = resources->activeSelection();

        KisPaintLayerSP paintLayer =
            dynamic_cast<KisPaintLayer*>(this->currentNode().data());

        const bool canUseSelectionMode =
                paintLayer && selection &&
                !selection->selectedRect().isEmpty() &&
                !selection->selectedExactRect().isEmpty();

        if (!canUseSelectionMode) {
            KisNodeSelectionRecipe nodeSelection =
                    KisNodeSelectionRecipe(
                        this->selectedNodes(),
                        (KisNodeSelectionRecipe::SelectionMode)moveToolMode(),
                        m_lastCursorPos);

            if (nodeSelection.selectNodesToProcess().isEmpty()) {
                canMove = false;
            }
        }
    }

    if (canMove) {
        KisTool::resetCursorStyle();
    } else {
       useCursor(Qt::ForbiddenCursor);
    }
}

bool KisToolMove::startStrokeImpl(MoveToolMode mode, const QPoint *pos)
{
    KisNodeSP node;
    KisImageSP image = this->image();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image, currentNode(), canvas()->resourceManager());
    KisSelectionSP selection = resources->activeSelection();

    KisPaintLayerSP paintLayer =
        dynamic_cast<KisPaintLayer*>(this->currentNode().data());

    const bool canUseSelectionMode =
            paintLayer && selection &&
            !selection->selectedRect().isEmpty() &&
            !selection->selectedExactRect().isEmpty();

    if (pos) {
        // finish stroke by clicking outside image bounds
        if (m_strokeId && !image->bounds().contains(*pos)) {
            endStroke();
            return false;
        }

        // restart stroke when the mode has changed or the user tried to
        // pick another layer in "layer under cursor" mode.
        if (m_strokeId &&
                (m_currentMode != mode ||
                 m_currentlyUsingSelection != canUseSelectionMode ||
                 (!m_currentlyUsingSelection &&
                  mode != MoveSelectedLayer &&
                  !m_handlesRect.translated(currentOffset()).contains(*pos)))) {

            endStroke();
        }
    }

    if (m_strokeId) return true;

    KisNodeList nodes;

    KisStrokeStrategy *strategy;

    bool isMoveSelection = false;
    if (canUseSelectionMode) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(selection, false);

        MoveSelectionStrokeStrategy *moveStrategy =
            new MoveSelectionStrokeStrategy(paintLayer,
                                            selection,
                                            image.data(),
                                            image.data());

        connect(moveStrategy,
                SIGNAL(sigHandlesRectCalculated(const QRect&)),
                SLOT(slotHandlesRectCalculated(const QRect&)));
        connect(moveStrategy,
                SIGNAL(sigStrokeStartedEmpty()),
                SLOT(slotStrokeStartedEmpty()));

        strategy = moveStrategy;
        isMoveSelection = true;
        nodes = {paintLayer};

    } else {
        KisNodeSelectionRecipe nodeSelection =
            pos ?
                KisNodeSelectionRecipe(
                        this->selectedNodes(),
                        (KisNodeSelectionRecipe::SelectionMode)mode,
                        *pos) :
                KisNodeSelectionRecipe(this->selectedNodes());


        MoveStrokeStrategy *moveStrategy =
            new MoveStrokeStrategy(nodeSelection, image.data(), image.data());
        connect(moveStrategy,
                SIGNAL(sigHandlesRectCalculated(const QRect&)),
                SLOT(slotHandlesRectCalculated(const QRect&)));
        connect(moveStrategy,
                SIGNAL(sigStrokeStartedEmpty()),
                SLOT(slotStrokeStartedEmpty()));
        connect(moveStrategy,
                SIGNAL(sigLayersPicked(const KisNodeList&)),
                SLOT(slotStrokePickedLayers(const KisNodeList&)));

        strategy = moveStrategy;
        nodes = nodeSelection.selectedNodes;
    }

    {
        KConfigGroup group = KSharedConfig::openConfig()->group(toolId());
        const bool forceLodMode = group.readEntry("forceLodMode", true);
        strategy->setForceLodModeIfPossible(forceLodMode);
    }

    // disable outline feedback until the stroke calcualtes
    // correct bounding rect
    m_handlesRect = QRect();
    m_strokeId = image->startStroke(strategy);
    m_currentlyProcessingNodes = nodes;
    m_currentlyUsingSelection = isMoveSelection;
    m_currentMode = mode;
    m_accumulatedOffset = QPoint();

    if (!isMoveSelection) {
        m_asyncUpdateHelper.startUpdateStream(image.data(), m_strokeId);
    }

    KIS_SAFE_ASSERT_RECOVER(m_changesTracker.isEmpty()) {
        m_changesTracker.reset();
    }
    commitChanges();

    return true;
}

QPoint KisToolMove::currentOffset() const
{
    return m_accumulatedOffset + m_dragPos - m_dragStart;
}

void KisToolMove::notifyGuiAfterMove(bool showFloatingMessage)
{
    if (!m_optionsWidget) return;
    if (m_handlesRect.isEmpty()) return;

    const QPoint currentTopLeft = m_handlesRect.topLeft() + currentOffset();

    KisSignalsBlocker b(m_optionsWidget);
    emit moveInNewPosition(currentTopLeft);

    // TODO: fetch this info not from options widget, but from config
    const bool showCoordinates = m_optionsWidget->showCoordinates();

    if (showCoordinates && showFloatingMessage) {
        KisCanvas2 *kisCanvas = static_cast<KisCanvas2*>(canvas());
        kisCanvas->viewManager()->
            showFloatingMessage(
                i18nc("floating message in move tool",
                      "X: %1 px, Y: %2 px",
                      QLocale().toString(currentTopLeft.x()),
                      QLocale().toString(currentTopLeft.y())),
                QIcon(), 1000, KisFloatingMessage::High);
    }
}

bool KisToolMove::tryEndPreviousStroke(const KisNodeList &nodes)
{
    if (!m_strokeId) return false;

    bool strokeEnded = false;

    if (!KritaUtils::compareListsUnordered(nodes, m_currentlyProcessingNodes)) {
        endStroke();
        strokeEnded = true;
    }

    return strokeEnded;
}

void KisToolMove::commitChanges()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_strokeId);

    QSharedPointer<KisToolMoveState> newState(new KisToolMoveState(m_accumulatedOffset));
    KisToolMoveState *lastState = dynamic_cast<KisToolMoveState*>(m_changesTracker.lastState().data());
    if (lastState && *lastState == *newState) return;

    m_changesTracker.commitConfig(newState);
}

void KisToolMove::slotHandlesRectCalculated(const QRect &handlesRect)
{
    m_handlesRect = handlesRect;
    notifyGuiAfterMove(false);
}

void KisToolMove::slotStrokeStartedEmpty()
{
    /**
     * Notify that move-selection stroke ended unexpectedly
     */
    if (m_currentlyUsingSelection) {
        KisCanvas2 *kisCanvas = static_cast<KisCanvas2*>(canvas());
        kisCanvas->viewManager()->
            showFloatingMessage(
                i18nc("floating message in move tool",
                      "Selected area has no pixels"),
                QIcon(), 1000, KisFloatingMessage::High);
    }

    /**
     * Since the choice of nodes for the operation happens in the
     * stroke itself, it may happen that there are no nodes at all.
     * In such a case, we should just cancel already started stroke.
     */
    cancelStroke();
}

void KisToolMove::slotStrokePickedLayers(const KisNodeList &nodes)
{
    if (nodes.isEmpty()) {
        useCursor(Qt::ForbiddenCursor);
    } else {
        KisTool::resetCursorStyle();
    }
}

void KisToolMove::moveDiscrete(MoveDirection direction, bool big)
{
    if (mode() == KisTool::PAINT_MODE) return;  // Don't interact with dragging
    if (!currentNode()) return;
    if (!image()) return;
    if (!currentNode()->isEditable()) return; // Don't move invisible nodes

    if (startStrokeImpl(MoveSelectedLayer, 0)) {
        setMode(KisTool::PAINT_MODE);
    }

    // Larger movement if "shift" key is pressed.
    qreal scale = big ? m_optionsWidget->moveScale() : 1.0;
    qreal moveStep = m_optionsWidget->moveStep() * scale;

    const QPoint offset =
        direction == Up   ? QPoint( 0, -moveStep) :
        direction == Down ? QPoint( 0,  moveStep) :
        direction == Left ? QPoint(-moveStep,  0) :
        QPoint( moveStep,  0) ;

    m_accumulatedOffset += offset;
    image()->addJob(m_strokeId, new MoveStrokeStrategy::Data(m_accumulatedOffset));

    notifyGuiAfterMove();
    commitChanges();
    setMode(KisTool::HOVER_MODE);
}

void KisToolMove::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisTool::activate(toolActivation, shapes);

    m_actionConnections.addConnection(action("movetool-move-up"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteUp()));
    m_actionConnections.addConnection(action("movetool-move-down"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteDown()));
    m_actionConnections.addConnection(action("movetool-move-left"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteLeft()));
    m_actionConnections.addConnection(action("movetool-move-right"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteRight()));

    m_actionConnections.addConnection(action("movetool-move-up-more"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteUpMore()));
    m_actionConnections.addConnection(action("movetool-move-down-more"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteDownMore()));
    m_actionConnections.addConnection(action("movetool-move-left-more"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteLeftMore()));
    m_actionConnections.addConnection(action("movetool-move-right-more"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteRightMore()));

    m_canvasConnections.addUniqueConnection(qobject_cast<KisCanvas2*>(canvas())->viewManager()->nodeManager(), SIGNAL(sigUiNeedChangeSelectedNodes(KisNodeList)), this, SLOT(slotNodeChanged(KisNodeList)));
    m_canvasConnections.addUniqueConnection(qobject_cast<KisCanvas2*>(canvas())->viewManager()->selectionManager(), SIGNAL(currentSelectionChanged()), this, SLOT(slotSelectionChanged()));

    connect(m_showCoordinatesAction, SIGNAL(triggered(bool)), m_optionsWidget, SLOT(setShowCoordinates(bool)), Qt::UniqueConnection);
    connect(m_optionsWidget, SIGNAL(showCoordinatesChanged(bool)), m_showCoordinatesAction, SLOT(setChecked(bool)), Qt::UniqueConnection);
    connect(m_optionsWidget, SIGNAL(sigSetTranslateX(int)), SLOT(moveBySpinX(int)), Qt::UniqueConnection);
    connect(m_optionsWidget, SIGNAL(sigSetTranslateY(int)), SLOT(moveBySpinY(int)), Qt::UniqueConnection);
    connect(m_optionsWidget, SIGNAL(sigRequestCommitOffsetChanges()), this, SLOT(commitChanges()), Qt::UniqueConnection);

    connect(&m_changesTracker,
            SIGNAL(sigConfigChanged(KisToolChangesTrackerDataSP)),
            SLOT(slotTrackerChangedConfig(KisToolChangesTrackerDataSP)));


    slotNodeChanged(this->selectedNodes());
}



void KisToolMove::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (m_strokeId && !m_handlesRect.isEmpty() && !m_currentlyUsingSelection) {
        QPainterPath handles;
        handles.addRect(m_handlesRect.translated(currentOffset()));

        QPainterPath path = pixelToView(handles);
        paintToolOutline(&gc, path);
    }
}

void KisToolMove::deactivate()
{
    m_actionConnections.clear();
    m_canvasConnections.clear();

    disconnect(m_showCoordinatesAction, 0, this, 0);
    disconnect(m_optionsWidget, 0, this, 0);

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
    if (!m_strokeId) return;

    if (m_changesTracker.isEmpty()) {
        cancelStroke();
    } else {
        m_changesTracker.requestUndo();
    }
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
    Q_UNUSED(action);
    continueAction(event);
}

void KisToolMove::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(action);
    endAction(event);
}

void KisToolMove::mouseMoveEvent(KoPointerEvent *event)
{
    m_lastCursorPos = convertToPixelCoord(event).toPoint();
    KisTool::mouseMoveEvent(event);

    if (moveToolMode() != MoveSelectedLayer ||
            (m_strokeId && m_currentMode != MoveSelectedLayer)) {

        m_updateCursorCompressor.start();
    }
}

void KisToolMove::startAction(KoPointerEvent *event, MoveToolMode mode)
{
    QPoint pos = convertToPixelCoordAndSnap(event).toPoint();
    m_dragStart = pos;
    m_dragPos = pos;

    if (startStrokeImpl(mode, &pos)) {
        setMode(KisTool::PAINT_MODE);

        if (m_currentlyUsingSelection) {
            KisImageSP image = currentImage();
            image->addJob(m_strokeId,
                          new MoveSelectionStrokeStrategy::ShowSelectionData(false));
        }

    } else {
        event->ignore();
        m_dragPos = QPoint();
        m_dragStart = QPoint();
    }
    qobject_cast<KisCanvas2*>(canvas())->updateCanvas();
}

void KisToolMove::continueAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    if (!m_strokeId) return;

    QPoint pos = convertToPixelCoordAndSnap(event).toPoint();
    pos = applyModifiers(event->modifiers(), pos);
    m_dragPos = pos;

    drag(pos);
    notifyGuiAfterMove();

    qobject_cast<KisCanvas2*>(canvas())->updateCanvas();
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
    m_dragStart = QPoint();
    m_dragPos = QPoint();
    commitChanges();

    if (m_currentlyUsingSelection) {
        KisImageSP image = currentImage();
        image->addJob(m_strokeId,
                      new MoveSelectionStrokeStrategy::ShowSelectionData(true));
    }

    notifyGuiAfterMove();

    qobject_cast<KisCanvas2*>(canvas())->updateCanvas();
}

void KisToolMove::drag(const QPoint& newPos)
{
    KisImageSP image = currentImage();

    QPoint offset = m_accumulatedOffset + newPos - m_dragStart;

    image->addJob(m_strokeId,
                  new MoveStrokeStrategy::Data(offset));
}

void KisToolMove::endStroke()
{
    if (!m_strokeId) return;

    if (m_asyncUpdateHelper.isActive()) {
        m_asyncUpdateHelper.endUpdateStream();
    }

    KisImageSP image = currentImage();
    image->endStroke(m_strokeId);
    m_strokeId.clear();
    m_changesTracker.reset();
    m_currentlyProcessingNodes.clear();
    m_currentlyUsingSelection = false;
    m_currentMode = MoveSelectedLayer;
    m_accumulatedOffset = QPoint();
    qobject_cast<KisCanvas2*>(canvas())->updateCanvas();
}

void KisToolMove::slotTrackerChangedConfig(KisToolChangesTrackerDataSP state)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_strokeId);

    KisToolMoveState *newState = dynamic_cast<KisToolMoveState*>(state.data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(newState);

    if (mode() == KisTool::PAINT_MODE) return;  // Don't interact with dragging
    m_accumulatedOffset = newState->accumulatedOffset;
    image()->addJob(m_strokeId, new MoveStrokeStrategy::Data(m_accumulatedOffset));
    notifyGuiAfterMove();
}

void KisToolMove::slotMoveDiscreteLeft()
{
    moveDiscrete(MoveDirection::Left, false);
}

void KisToolMove::slotMoveDiscreteRight()
{
    moveDiscrete(MoveDirection::Right, false);
}

void KisToolMove::slotMoveDiscreteUp()
{
    moveDiscrete(MoveDirection::Up, false);
}

void KisToolMove::slotMoveDiscreteDown()
{
    moveDiscrete(MoveDirection::Down, false);
}

void KisToolMove::slotMoveDiscreteLeftMore()
{
    moveDiscrete(MoveDirection::Left, true);
}

void KisToolMove::slotMoveDiscreteRightMore()
{
    moveDiscrete(MoveDirection::Right, true);
}

void KisToolMove::slotMoveDiscreteUpMore()
{
    moveDiscrete(MoveDirection::Up, true);
}

void KisToolMove::slotMoveDiscreteDownMore()
{
    moveDiscrete(MoveDirection::Down, true);
}

void KisToolMove::cancelStroke()
{
    if (!m_strokeId) return;

    if (m_asyncUpdateHelper.isActive()) {
        m_asyncUpdateHelper.cancelUpdateStream();
    }

    KisImageSP image = currentImage();
    image->cancelStroke(m_strokeId);
    m_strokeId.clear();
    m_changesTracker.reset();
    m_currentlyProcessingNodes.clear();
    m_currentlyUsingSelection = false;
    m_currentMode = MoveSelectedLayer;
    m_accumulatedOffset = QPoint();
    notifyGuiAfterMove();
    qobject_cast<KisCanvas2*>(canvas())->updateCanvas();
}

QWidget* KisToolMove::createOptionWidget()
{
    return m_optionsWidget;
}

KisToolMove::MoveToolMode KisToolMove::moveToolMode() const
{
    if (m_optionsWidget)
        return m_optionsWidget->mode();
    return MoveSelectedLayer;
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
    if (mode() == KisTool::PAINT_MODE ||    // Don't interact with dragging
            !currentNode()->isEditable() || // Don't move invisible nodes
            m_handlesRect.isEmpty()) {
        return;
    }

    // starting a new stroke resets m_handlesRect and it gets updated asynchronously,
    // but in this case no change is expected
    int handlesRectX = m_handlesRect.x();

    if (startStrokeImpl(MoveSelectedLayer, 0)) {
        setMode(KisTool::PAINT_MODE);
    }

    m_accumulatedOffset.rx() =  newX - handlesRectX;

    image()->addJob(m_strokeId, new MoveStrokeStrategy::Data(m_accumulatedOffset));

    notifyGuiAfterMove(false);
    setMode(KisTool::HOVER_MODE);
}

void KisToolMove::moveBySpinY(int newY)
{
    if (mode() == KisTool::PAINT_MODE ||    // Don't interact with dragging
            !currentNode()->isEditable() || // Don't move invisible nodes
            m_handlesRect.isEmpty()) {
        return;
    }

    // starting a new stroke resets m_handlesRect and it gets updated asynchronously,
    // but in this case no change is expected
    int handlesRectY = m_handlesRect.y();

    if (startStrokeImpl(MoveSelectedLayer, 0)) {
        setMode(KisTool::PAINT_MODE);
    }

    m_accumulatedOffset.ry() =  newY - handlesRectY;

    image()->addJob(m_strokeId, new MoveStrokeStrategy::Data(m_accumulatedOffset));

    notifyGuiAfterMove(false);
    setMode(KisTool::HOVER_MODE);
}

void KisToolMove::requestHandlesRectUpdate()
{
    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image(), currentNode(), canvas()->resourceManager());
    KisSelectionSP selection = resources->activeSelection();

    KisMoveBoundsCalculationJob *job = new KisMoveBoundsCalculationJob(this->selectedNodes(),
                                                                       selection, this);
    connect(job,
            SIGNAL(sigCalcualtionFinished(const QRect&)),
            SLOT(slotHandlesRectCalculated(const QRect &)));

    KisImageSP image = this->image();
    image->addSpontaneousJob(job);

    notifyGuiAfterMove(false);
}

void KisToolMove::slotNodeChanged(const KisNodeList &nodes)
{
    if (m_strokeId && !tryEndPreviousStroke(nodes)) {
        return;
    }
    requestHandlesRectUpdate();
}

void KisToolMove::slotSelectionChanged()
{
    if (m_strokeId) return;
    requestHandlesRectUpdate();
}

QList<QAction *> KisToolMoveFactory::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions = KisToolPaintFactoryBase::createActionsImpl();

    actions << actionRegistry->makeQAction("movetool-move-up");
    actions << actionRegistry->makeQAction("movetool-move-down");
    actions << actionRegistry->makeQAction("movetool-move-left");
    actions << actionRegistry->makeQAction("movetool-move-right");
    actions << actionRegistry->makeQAction("movetool-move-up-more");
    actions << actionRegistry->makeQAction("movetool-move-down-more");
    actions << actionRegistry->makeQAction("movetool-move-left-more");
    actions << actionRegistry->makeQAction("movetool-move-right-more");
    actions << actionRegistry->makeQAction("movetool-show-coordinates");

    return actions;

}
