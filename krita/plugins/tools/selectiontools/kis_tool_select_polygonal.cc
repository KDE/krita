/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_tool_select_polygonal.h"
#include <QApplication>
#include <QPainter>
#include <QRegion>
#include <QWidget>
#include <QLayout>
#include <QVBoxLayout>

#include <kis_debug.h>
#include <klocale.h>

#include <KoPointerEvent.h>
#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include "kis_layer.h"
#include "kis_selection_options.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "canvas/kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"

KisToolSelectPolygonal::KisToolSelectPolygonal(KoCanvasBase *canvas)
        : KisTool(canvas, KisCursor::load("tool_polygonal_selection_cursor.png", 6, 6))
{
    setObjectName("tool_select_polygonal");
    m_dragging = false;
    m_optWidget = 0;
    m_selectAction = SELECTION_REPLACE;
    m_selectionMode = PIXEL_SELECTION;
}

KisToolSelectPolygonal::~KisToolSelectPolygonal()
{
}

void KisToolSelectPolygonal::activate(bool tmp)
{
    m_points.clear();
    KisTool::activate(tmp);

    if (!m_optWidget)
        return;

    m_optWidget->slotActivated();
}

void KisToolSelectPolygonal::deactivate()
{
    m_points.clear();
    m_dragging = false;
    updateCanvasPixelRect(image()->bounds());
}

void KisToolSelectPolygonal::mousePressEvent(KoPointerEvent *event)
{
    if (currentImage()) {
        if (event->button() == Qt::LeftButton && event->modifiers() != Qt::ShiftModifier) {

            m_dragging = true;

            if (m_points.isEmpty()) {
                m_dragStart = convertToPixelCoord(event);
                m_dragEnd = m_dragStart;
                m_points.append(m_dragStart);
            } else {
                m_dragStart = m_dragEnd;
                m_dragEnd = convertToPixelCoord(event);
            }
        } else if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier) {
            finish();
        }
    }
}

void KisToolSelectPolygonal::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        // erase old lines on canvas
        QRectF updateRect = dragBoundingRect();
        // get current mouse position
        m_dragEnd = convertToPixelCoord(event);
        // draw new lines on canvas
        updateRect |= dragBoundingRect();
        updateCanvasViewRect(updateRect);
    }
}

void KisToolSelectPolygonal::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!m_canvas || !currentImage())
        return;

    if (m_dragging && event->button() == Qt::LeftButton)  {
        m_dragging = false;
        m_points.append(m_dragEnd);
    }

    if (m_dragging && event->button() == Qt::RightButton) {

    }
}

void KisToolSelectPolygonal::mouseDoubleClickEvent(KoPointerEvent *)
{
    finish();
}

void KisToolSelectPolygonal::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        deactivate();
    }
}

void KisToolSelectPolygonal::finish()
{
    if (currentImage() && m_points.count() > 2) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());

        KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(m_canvas);
        if (!kisCanvas)
            return;

        if (!currentNode())
            return;

        KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Polygonal Selection"));

        if (m_selectionMode == PIXEL_SELECTION) {

            KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

            KisPainter painter(tmpSel);
            painter.setBounds(currentImage()->bounds());
            painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
            painter.setFillStyle(KisPainter::FillStyleForegroundColor);
            painter.setStrokeStyle(KisPainter::StrokeStyleNone);
            painter.setAntiAliasPolygonFill(m_optWidget->antiAliasSelection());
            painter.setOpacity(OPACITY_OPAQUE);
            painter.setPaintOpPreset(currentPaintOpPreset(), currentImage()); // And now the painter owns the op and will destroy it.
            painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));
            painter.paintPolygon(m_points);

            QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectAction);
            m_canvas->addCommand(cmd);
        } else {
            KoPathShape* path = new KoPathShape();
            path->setShapeId(KoPathShapeId);

            QMatrix resolutionMatrix;
            resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
            path->moveTo(resolutionMatrix.map(m_points[0]));
            for (int i = 1; i < m_points.count(); i++)
                path->lineTo(resolutionMatrix.map(m_points[i]));
            path->close();
            path->normalize();

            helper.addSelectionShape(path);
        }
        QApplication::restoreOverrideCursor();
    }

    deactivate();
}

#define FEEDBACK_LINE_WIDTH 1

void KisToolSelectPolygonal::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (!m_canvas || !currentImage())
        return;

    gc.save();

    QPen pen(Qt::white, FEEDBACK_LINE_WIDTH, Qt::DotLine);
    pen.setWidth(FEEDBACK_LINE_WIDTH);
    gc.setPen(pen);

    QPointF start, end;
    QPointF startPos;
    QPointF endPos;

    if (m_dragging) {
        startPos = pixelToView(m_dragStart);
        endPos = pixelToView(m_dragEnd);
        gc.drawLine(startPos, endPos);
    }
    for (vQPointF::iterator it = m_points.begin(); it != m_points.end(); ++it) {

        if (it == m_points.begin()) {
            start = (*it);
        } else {
            end = (*it);

            startPos = pixelToView(start);
            endPos = pixelToView(end);
            gc.drawLine(startPos, endPos);
            start = end;
        }
    }
    gc.restore();
}

QRectF KisToolSelectPolygonal::dragBoundingRect()
{
    QRectF rect = pixelToView(QRectF(m_dragStart, m_dragEnd).normalized());
    rect.adjust(-FEEDBACK_LINE_WIDTH, -FEEDBACK_LINE_WIDTH, FEEDBACK_LINE_WIDTH, FEEDBACK_LINE_WIDTH);
    return rect;
}

QWidget* KisToolSelectPolygonal::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(m_canvas);
    Q_ASSERT(canvas);
    m_optWidget = new KisSelectionOptions(canvas);
    m_optWidget->setObjectName(toolId() + " option widget");

    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setWindowTitle(i18n("Polygonal Selection"));

    connect(m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));
    connect(m_optWidget, SIGNAL(modeChanged(int)), this, SLOT(slotSetSelectionMode(int)));

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    if (l) {
        l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    }
    m_optWidget->setFixedHeight(m_optWidget->sizeHint().height());
    return m_optWidget;
}

QWidget* KisToolSelectPolygonal::optionWidget()
{
    return m_optWidget;
}

void KisToolSelectPolygonal::slotSetAction(int action)
{
    if (action >= SELECTION_REPLACE && action <= SELECTION_INTERSECT)
        m_selectAction = (selectionAction)action;
}

void KisToolSelectPolygonal::slotSetSelectionMode(int mode)
{
    m_selectionMode = (selectionMode)mode;
}

#include "kis_tool_select_polygonal.moc"
