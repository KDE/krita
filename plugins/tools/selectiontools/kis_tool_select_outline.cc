/*
 *  kis_tool_select_freehand.h - part of Krayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_outline.h"

#include <QApplication>
#include <QPainter>
#include <QWidget>
#include <QPainterPath>
#include <QLayout>
#include <QVBoxLayout>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoPointerEvent.h>
#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoViewConverter.h>

#include <kis_layer.h>
#include <kis_selection_options.h>
#include <kis_cursor.h>
#include <kis_image.h>

#include "kis_painter.h"
#include <brushengine/kis_paintop_registry.h>
#include "canvas/kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"

#include "kis_algebra_2d.h"


#define FEEDBACK_LINE_WIDTH 2


KisToolSelectOutline::KisToolSelectOutline(KoCanvasBase * canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_outline_selection_cursor.png", 5, 5),
                    i18n("Freehand Selection")),
      m_continuedMode(false)
{
}

KisToolSelectOutline::~KisToolSelectOutline()
{
}

void KisToolSelectOutline::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        m_continuedMode = true;
    }
    KisToolSelect::keyPressEvent(event);
}

void KisToolSelectOutline::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control ||
        !(event->modifiers() & Qt::ControlModifier)) {

        m_continuedMode = false;
        if (mode() != PAINT_MODE && !m_points.isEmpty()) {
            finishSelectionAction();
        }
    }

    KisToolSelect::keyReleaseEvent(event);
}

void KisToolSelectOutline::mouseMoveEvent(KoPointerEvent *event)
{
    if (selectionDragInProgress()) return;

    m_lastCursorPos = convertToPixelCoord(event);
    if (m_continuedMode && mode() != PAINT_MODE) {
        updateContinuedMode();
    } else {
        KisToolSelect::mouseMoveEvent(event);
    }

    KisToolSelect::mouseMoveEvent(event);
}

void KisToolSelectOutline::beginPrimaryAction(KoPointerEvent *event)
{
    KisToolSelectBase::beginPrimaryAction(event);
    if (selectionDragInProgress()) {
        return;
    }

    if (!selectionEditable()) {
        event->ignore();
        return;
    }

    setMode(KisTool::PAINT_MODE);

    if (m_continuedMode && !m_points.isEmpty()) {
        m_paintPath.lineTo(pixelToView(convertToPixelCoord(event)));
    } else {
        m_paintPath.moveTo(pixelToView(convertToPixelCoord(event)));
    }

    m_points.append(convertToPixelCoord(event));
}

void KisToolSelectOutline::continuePrimaryAction(KoPointerEvent *event)
{    
    if (selectionDragInProgress()) {
        KisToolSelectBase::continuePrimaryAction(event);
        return;
    }

    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    QPointF point = convertToPixelCoord(event);
    m_paintPath.lineTo(pixelToView(point));
    m_points.append(point);
    updateFeedback();
}

void KisToolSelectOutline::endPrimaryAction(KoPointerEvent *event)
{
    if (selectionDragInProgress()) {
        KisToolSelectBase::endPrimaryAction(event);
        return;
    }

    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    if (!m_continuedMode) {
        finishSelectionAction();
        m_points.clear(); // ensure points are always cleared
    }
}

void KisToolSelectOutline::finishSelectionAction()
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();

    const QRectF boundingRect = KisAlgebra2D::accumulateBounds(m_points);
    const QRectF boundingViewRect = pixelToView(boundingRect);

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Freehand Selection"));

    if (helper.tryDeselectCurrentSelection(boundingViewRect, selectionAction())) {
        return;
    }

    if (m_points.count() > 2) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());

        const SelectionMode mode =
            helper.tryOverrideSelectionMode(kisCanvas->viewManager()->selection(),
                                            selectionMode(),
                                            selectionAction());

        if (mode == PIXEL_SELECTION) {

            KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

            KisPainter painter(tmpSel);
            painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
            painter.setAntiAliasPolygonFill(antiAliasSelection());
            painter.setFillStyle(KisPainter::FillStyleForegroundColor);
            painter.setStrokeStyle(KisPainter::StrokeStyleNone);

            painter.paintPolygon(m_points);

            QPainterPath cache;
            cache.addPolygon(m_points);
            cache.closeSubpath();
            tmpSel->setOutlineCache(cache);

            helper.selectPixelSelection(tmpSel, selectionAction());
        } else {

            KoPathShape* path = new KoPathShape();
            path->setShapeId(KoPathShapeId);

            QTransform resolutionMatrix;
            resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
            path->moveTo(resolutionMatrix.map(m_points[0]));
            for (int i = 1; i < m_points.count(); i++)
                path->lineTo(resolutionMatrix.map(m_points[i]));
            path->close();
            path->normalize();

            helper.addSelectionShape(path, selectionAction());
        }
        QApplication::restoreOverrideCursor();
    }

    m_points.clear();
    m_paintPath = QPainterPath();
}

void KisToolSelectOutline::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if ((mode() == KisTool::PAINT_MODE || m_continuedMode) &&
        !m_points.isEmpty()) {

        QPainterPath outline = m_paintPath;
        if (m_continuedMode && mode() != KisTool::PAINT_MODE) {
            outline.lineTo(pixelToView(m_lastCursorPos));
        }
        paintToolOutline(&gc, outline);
    }
}

bool KisToolSelectOutline::primaryActionSupportsHiResEvents() const
{
    return !selectionDragInProgress();
}

bool KisToolSelectOutline::alternateActionSupportsHiResEvents(AlternateAction action) const
{
    /**
     * In selection tools we abuse alternate actions to switch different
     * selection modes. So we should notify input manager that we need a
     * good precision for them
     */

    Q_UNUSED(action);
    return !selectionDragInProgress();
}

void KisToolSelectOutline::updateFeedback()
{
    if (m_points.count() > 1) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_points[lastPointIndex]).normalized();
        updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

void KisToolSelectOutline::updateContinuedMode()
{
    if (!m_points.isEmpty()) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_lastCursorPos).normalized();
        updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

void KisToolSelectOutline::deactivate()
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();

    m_continuedMode = false;

    KisTool::deactivate();
}

void KisToolSelectOutline::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_outline_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_outline_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelect::resetCursorStyle();
    }
}

