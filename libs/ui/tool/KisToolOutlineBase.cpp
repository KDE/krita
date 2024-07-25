/*
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QAction>

#include <KoPointerEvent.h>
#include <KoShapeController.h>
#include <KoViewConverter.h>
#include <KisViewManager.h>
#include <KoCanvasBase.h>
#include <kis_icon.h>
#include <kis_canvas2.h>
#include <input/kis_input_manager.h>

#include "KisToolOutlineBase.h"
#include "input/KisInputActionGroup.h"

KisToolOutlineBase::KisToolOutlineBase(KoCanvasBase * canvas, ToolType type, const QCursor & cursor)
    : KisToolShape(canvas, cursor)
    , m_continuedMode(false)
    , m_type(type)
    , m_numberOfContinuedModePoints(0)
    , m_hasUserInteractionRunning(false)
{}

KisToolOutlineBase::~KisToolOutlineBase()
{}

void KisToolOutlineBase::keyPressEvent(QKeyEvent *event)
{
    // Allow to enter continued mode only if we started drawing the shape
    if (mode() == PAINT_MODE && event->key() == Qt::Key_Control) {
        m_continuedMode = true;
        installBlockActionGuard();
    }
    KisToolShape::keyPressEvent(event);
}

void KisToolOutlineBase::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control ||
        !(event->modifiers() & Qt::ControlModifier)) {
        m_continuedMode = false;
        if (mode() != PAINT_MODE) {
            endStroke();
        }
    }
    KisToolShape::keyReleaseEvent(event);
}

void KisToolOutlineBase::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_continuedMode && mode() != PAINT_MODE) {
        updateContinuedMode();
        m_lastCursorPos = convertToPixelCoordAndSnap(event);
    } else {
        m_lastCursorPos = convertToPixelCoord(event);
    }
    if (mode() == PAINT_MODE) {
        KisToolShape::requestUpdateOutline(event->point, event);
    }

    KisToolShape::mouseMoveEvent(event);
}

void KisToolOutlineBase::activate(const QSet<KoShape *> &shapes)
{
    KisToolShape::activate(shapes);
    connect(action("undo_polygon_selection"), SIGNAL(triggered()), SLOT(undoLastPoint()), Qt::UniqueConnection);

    KisInputManager *inputManager = (static_cast<KisCanvas2*>(canvas()))->globalInputManager();
    if (inputManager) {
        inputManager->attachPriorityEventFilter(this);
    }
}

void KisToolOutlineBase::deactivate()
{
    cancelStroke();
    
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();

    m_continuedMode = false;

    KisInputManager *inputManager = kisCanvas->globalInputManager();
    if (inputManager) {
        inputManager->detachPriorityEventFilter(this);
    }

    KisToolShape::deactivate();
}

KisPopupWidgetInterface* KisToolOutlineBase::popupWidget()
{
    return !m_points.isEmpty() || m_type == SELECT ? nullptr : KisToolShape::popupWidget();
}

// Install an event filter to catch right-click events.
// The simplest way to accommodate the popup palette binding.
bool KisToolOutlineBase::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if (m_points.isEmpty()) {
        return false;
    }
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            undoLastPoint();
            return true;
        }
    } else if (event->type() == QEvent::TabletPress) {
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        if (tabletEvent->button() == Qt::RightButton) {
            undoLastPoint();
            return true;
        }
    }
    return false;
}

void KisToolOutlineBase::undoLastPoint()
{
    if(!m_points.isEmpty() && m_continuedMode && mode() != PAINT_MODE && m_numberOfContinuedModePoints > 0) {
        // Initialize with the dragging segment's rect
        QRectF updateRect = dragBoundingRect();

        if (m_points.size() > 1) {
            // Add the rect for the last segment
            const QRectF lastSegmentRect =
                pixelToView(QRectF(m_points.last(), m_points.at(m_points.size() - 2)).normalized())
                .adjusted(-FEEDBACK_LINE_WIDTH, -FEEDBACK_LINE_WIDTH, FEEDBACK_LINE_WIDTH, FEEDBACK_LINE_WIDTH);
            updateRect = updateRect.united(lastSegmentRect);

            m_points.pop_back();
            --m_numberOfContinuedModePoints;
        }

        // Add the new dragging segment's rect
        updateRect = updateRect.united(dragBoundingRect());
        updateCanvasViewRect(updateRect);
    }
}

void KisToolOutlineBase::beginPrimaryAction(KoPointerEvent *event)
{
    NodePaintAbility paintability = nodePaintAbility();
    if ((m_type == PAINT && (!nodeEditable() || paintability == UNPAINTABLE || paintability  == KisToolPaint::CLONE || paintability == KisToolPaint::MYPAINTBRUSH_UNPAINTABLE)) || (m_type == SELECT && !selectionEditable())) {

        if (paintability == KisToolPaint::CLONE){
            KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
            QString message = i18n("This tool cannot paint on clone layers.  Please select a paint or vector layer or mask.");
            kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        }

        if (paintability == KisToolPaint::MYPAINTBRUSH_UNPAINTABLE) {
            KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
            QString message = i18n("The MyPaint Brush Engine is not available for this colorspace");
            kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        }

        event->ignore();
        return;
    }

    setMode(KisTool::PAINT_MODE);

    if (!m_continuedMode || m_points.isEmpty()) {
        m_hasUserInteractionRunning = true;
        beginShape();
    }

    if (m_continuedMode) {
        m_points.append(convertToPixelCoordAndSnap(event));
        ++m_numberOfContinuedModePoints;
    } else {
        m_numberOfContinuedModePoints = 0;
        m_points.append(convertToPixelCoord(event));
    }
}

void KisToolOutlineBase::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    QPointF point = convertToPixelCoord(event);
    m_points.append(point);
    updateFeedback();
}

void KisToolOutlineBase::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);
    if (!m_continuedMode) {
        // If the event was not originated by the user releasing the button
        // (for example due to the canvas loosing focus), then we just cancel
        // the operation. This prevents some issues with shapes being added
        // after the image was closed while the shape was being made
        if (event->spontaneous()) {
            endStroke();
        } else {
            cancelStroke();
        }
        event->accept();
    }
}

void KisToolOutlineBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    if ((mode() == KisTool::PAINT_MODE || m_continuedMode) && !m_points.isEmpty()) {
        QPainterPath outline;
        outline.moveTo(pixelToView(m_points.first()));
        for (qint32 i = 1; i < m_points.size(); ++i) {
            outline.lineTo(pixelToView(m_points[i]));
        }
        if (m_continuedMode && mode() != KisTool::PAINT_MODE) {
            outline.lineTo(pixelToView(m_lastCursorPos));
        }
        paintToolOutline(&gc, outline);
    }

    KisToolShape::paint(gc, converter);
}

void KisToolOutlineBase::updateFeedback()
{
    if (m_points.count() > 1) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_points[lastPointIndex]).normalized();
        updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

QRectF KisToolOutlineBase::dragBoundingRect()
{
    QRectF updateRect = pixelToView(QRectF(m_points.last(), m_lastCursorPos).normalized());
    updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);
    return updateRect;
}

void KisToolOutlineBase::updateContinuedMode()
{
    if (!m_points.isEmpty()) {
        updateCanvasViewRect(dragBoundingRect());
    }
}

bool KisToolOutlineBase::hasUserInteractionRunning() const
{
    return m_hasUserInteractionRunning;
}

void KisToolOutlineBase::endStroke()
{
    if (!hasUserInteractionRunning()) {
        return;
    }
    uninstallBlockActionGuard();
    setMode(KisTool::HOVER_MODE);
    m_hasUserInteractionRunning = false;

    finishOutline(m_points);
    m_points.clear();
    endShape();
}

void KisToolOutlineBase::cancelStroke()
{
    if (!hasUserInteractionRunning()) {
        return;
    }
    uninstallBlockActionGuard();
    setMode(KisTool::HOVER_MODE);
    m_hasUserInteractionRunning = false;

    m_points.clear();
    endShape();
}

void KisToolOutlineBase::requestStrokeEnd()
{
    endStroke();
}

void KisToolOutlineBase::requestStrokeCancellation()
{
    cancelStroke();
}

void KisToolOutlineBase::installBlockActionGuard()
{
    if (m_blockModifyingActionsGuard)
        return;
    m_blockModifyingActionsGuard.reset(new KisInputActionGroupsMaskGuard(
        static_cast<KisCanvas2*>(canvas())->inputActionGroupsMaskInterface(),
                                 ViewTransformActionGroup | ToolInvoactionActionGroup
                                ));
}

void KisToolOutlineBase::uninstallBlockActionGuard()
{
    m_blockModifyingActionsGuard.reset();
}
