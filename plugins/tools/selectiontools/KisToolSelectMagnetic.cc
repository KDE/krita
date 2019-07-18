/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
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

#include "KisToolSelectMagnetic.h"

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


KisToolSelectMagnetic::KisToolSelectMagnetic(KoCanvasBase *canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_magnetic_selection_cursor.svg", 16, 16),
                    i18n("Magnetic Selection")),
      m_continuedMode(false), m_complete(true), m_radius(20), m_threshold(120), m_checkPoint(-1)
{ }

void KisToolSelectMagnetic::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        m_continuedMode = true;
    }

    KisToolSelect::keyPressEvent(event);
}

void KisToolSelectMagnetic::keyReleaseEvent(QKeyEvent *event)
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

//the cursor is still tracked even when no mousebutton is pressed
void KisToolSelectMagnetic::mouseMoveEvent(KoPointerEvent *event)
{
    KisToolSelect::mouseMoveEvent(event);
    if(m_complete)
        return;

    m_lastCursorPos = convertToPixelCoord(event);
    QPoint current((int)m_lastCursorPos.x(), (int)m_lastCursorPos.y());
    vQPointF pointSet = m_worker.computeEdge(m_radius, m_lastAnchor, current);
    m_points.resize(m_checkPoint+1);
    m_points.append(pointSet);
    for(int i=m_points.count()-1; i>= m_checkPoint; i--){
        QPoint pointInQuestion(m_points[i].x(), m_points[i].y());
        if(m_worker.intensity(pointInQuestion) >= m_threshold){
            m_checkPoint = i;
            m_lastAnchor = pointInQuestion;
            break;
        }
    }

    m_paintPath = QPainterPath();
    m_paintPath.moveTo(pixelToView(m_points[0]));

    for(int i=1; i<m_points.count();i++){
        m_paintPath.lineTo(pixelToView(m_points[i]));
    }

    updateFeedback();

    if (m_continuedMode && mode() != PAINT_MODE) {
        updateContinuedMode();
    }
}

//press primary mouse button
void KisToolSelectMagnetic::beginPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::PAINT_MODE);
    QPointF temp(convertToPixelCoord(event));
    m_lastAnchor = QPoint((int)temp.x(), (int)temp.y());
    m_checkPoint = m_points.count() - 1;

    if(m_anchorPoints.count() == 0){
        m_snapBound = QRect(QPoint(0,0), QSize(10,10));
        m_snapBound.moveCenter(m_lastAnchor);
    }

    if(m_anchorPoints.count() > 0 && m_snapBound.contains(m_lastAnchor)){
        m_complete = true;
        finishSelectionAction();
        return;
    }

    m_anchorPoints.push_back(m_lastAnchor);
    m_complete = false;
}

//drag while primary mouse button is pressed
void KisToolSelectMagnetic::continuePrimaryAction(KoPointerEvent *event)
{
    KisToolSelectBase::continuePrimaryAction(event);
}

//release primary mouse button
void KisToolSelectMagnetic::endPrimaryAction(KoPointerEvent *event)
{
    KisToolSelectBase::endPrimaryAction(event);
}

void KisToolSelectMagnetic::finishSelectionAction()
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();
    setMode(KisTool::HOVER_MODE);

    QRectF boundingViewRect =
        pixelToView(KisAlgebra2D::accumulateBounds(m_points));

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select by Outline"));

    if (m_points.count() > 2 &&
        !helper.tryDeselectCurrentSelection(boundingViewRect, selectionAction())) {
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
    m_anchorPoints.clear();
    m_paintPath = QPainterPath();
}

void KisToolSelectMagnetic::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if ((mode() == KisTool::PAINT_MODE || m_continuedMode) &&
        !m_points.isEmpty()) {

        QPainterPath outline = m_paintPath;
        if (m_continuedMode && mode() != KisTool::PAINT_MODE) {
            outline.lineTo(pixelToView(m_lastCursorPos));
        }
        paintToolOutline(&gc, outline);

        Q_FOREACH(const QPoint pt, m_anchorPoints){
            QRect tempRect(QPoint(0,0),QSize(1, 1));
            tempRect.moveTo(pt);
            gc.drawRect(pixelToView(tempRect));
        }
    }
}

void KisToolSelectMagnetic::updateFeedback()
{
    if (m_points.count() > 1) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_points[lastPointIndex]).normalized();
        updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

void KisToolSelectMagnetic::updateContinuedMode()
{
    if (!m_points.isEmpty()) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_lastCursorPos).normalized();
        updateRect = kisGrowRect(updateRect, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

void KisToolSelectMagnetic::activate(KoToolBase::ToolActivation activation, const QSet<KoShape*> &shapes)
{
    m_worker = KisMagneticWorker(image()->projection());
    KisToolSelect::activate(activation, shapes);
}

void KisToolSelectMagnetic::deactivate()
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();

    m_continuedMode = false;
    m_complete = true;

    KisTool::deactivate();
}

void KisToolSelectMagnetic::requestUndoDuringStroke()
{
    if(m_complete) return;
    m_anchorPoints.pop_back();
    m_lastAnchor = m_anchorPoints.last();
    for(int i=m_checkPoint; i>=0; i--){
        if(m_points[i] == m_lastAnchor){
            m_checkPoint = i;
            break;
        }
    }
}

void KisToolSelectMagnetic::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_outline_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_outline_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelect::resetCursorStyle();
    }
}

