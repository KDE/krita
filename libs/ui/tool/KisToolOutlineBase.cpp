/*
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoPointerEvent.h>
#include <KoShapeController.h>
#include <KoViewConverter.h>
#include <KisViewManager.h>
#include <KoCanvasBase.h>
#include <kis_icon.h>
#include <kis_canvas2.h>
#include <input/kis_input_manager.h>

#include "KisToolOutlineBase.h"

KisToolOutlineBase::KisToolOutlineBase(KoCanvasBase * canvas, ToolType type, const QCursor & cursor)
    : KisToolShape(canvas, cursor)
    , m_continuedMode(false)
    , m_type(type)
{}

KisToolOutlineBase::~KisToolOutlineBase()
{}

void KisToolOutlineBase::keyPressEvent(QKeyEvent *event)
{
    // Allow to enter continued mode only if we started drawing the shape
    if (mode() == PAINT_MODE && event->key() == Qt::Key_Control) {
        m_continuedMode = true;
    }
    KisToolShape::keyPressEvent(event);
}

void KisToolOutlineBase::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control ||
        !(event->modifiers() & Qt::ControlModifier)) {
        m_continuedMode = false;
        if (mode() != PAINT_MODE) {
            finishOutlineAction();
        }
    }
    KisToolShape::keyReleaseEvent(event);
}

void KisToolOutlineBase::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_continuedMode && mode() != PAINT_MODE) {
        updateContinuedMode();
    }
    if (mode() == PAINT_MODE) {
        KisToolShape::requestUpdateOutline(event->point, event);
    }

    m_lastCursorPos = convertToPixelCoord(event);
    KisToolShape::mouseMoveEvent(event);
}

void KisToolOutlineBase::deactivate()
{
    finishOutlineAction();
    
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();

    m_continuedMode = false;

    KisToolShape::deactivate();
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

    if (m_continuedMode && !m_points.isEmpty()) {
        m_paintPath.lineTo(pixelToView(convertToPixelCoord(event)));
    } else {
        beginShape();
        m_paintPath.moveTo(pixelToView(convertToPixelCoord(event)));
    }

    m_points.append(convertToPixelCoord(event));
}

void KisToolOutlineBase::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    QPointF point = convertToPixelCoord(event);
    m_paintPath.lineTo(pixelToView(point));
    m_points.append(point);
    updateFeedback();
}

void KisToolOutlineBase::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);

    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    if (!m_continuedMode) {
        finishOutlineAction();
    }
}

void KisToolOutlineBase::finishOutlineAction()
{
    if (m_points.isEmpty()) {
        return;
    }
    finishOutline(m_points);
    m_points.clear();
    m_paintPath = QPainterPath();
    endShape();
}

void KisToolOutlineBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    if ((mode() == KisTool::PAINT_MODE || m_continuedMode) && !m_points.isEmpty()) {

        QPainterPath outline = m_paintPath;
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

void KisToolOutlineBase::updateContinuedMode()
{
    if (!m_points.isEmpty()) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex], m_lastCursorPos).normalized();
        updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

bool KisToolOutlineBase::hasUserInteractionRunning() const
{
    return !m_points.isEmpty();
}
