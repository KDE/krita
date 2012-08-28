/*
 *  kis_tool_select_freehand.h - part of Krayon^WKrita
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

#include "kis_tool_select_outline.h"

#include <QApplication>
#include <QPainter>
#include <QWidget>
#include <QPainterPath>
#include <QLayout>
#include <QVBoxLayout>

#include <kis_debug.h>
#include <klocale.h>

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
#include "kis_paintop_registry.h"
#include "canvas/kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"


KisToolSelectOutline::KisToolSelectOutline(KoCanvasBase * canvas)
        : KisToolSelectBase(canvas,
                            KisCursor::load("tool_outline_selection_cursor.png", 5, 5),
                            i18n("Outline Selection")),
          m_paintPath(new QPainterPath())
{
}

KisToolSelectOutline::~KisToolSelectOutline()
{
    delete m_paintPath;
}


void KisToolSelectOutline::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::NoModifier)) {

        if (!currentNode()) {
            return;
        }

        if (!selectionEditable()) {
            return;
        }

        setMode(KisTool::PAINT_MODE);

        m_points.clear();
        m_points.append(convertToPixelCoord(event));
        m_paintPath->moveTo(pixelToView(convertToPixelCoord(event)));
    }
    else {
        KisToolSelectBase::mousePressEvent(event);
    }
}

void KisToolSelectOutline::mouseMoveEvent(KoPointerEvent *event)
{
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        QPointF point = convertToPixelCoord(event);
        //don't add the point, if the distance is very small
        if (!m_points.isEmpty()) {
            QPointF diff = point - m_points.last();
            if(fabs(diff.x())<3 && fabs(diff.y())<3)
                return;
        }
        m_paintPath->lineTo(pixelToView(point));
        m_points.append(point);
        updateFeedback();
    }
    else {
        KisToolSelectBase::mouseMoveEvent(event);
    }
}

void KisToolSelectOutline::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);
        deactivate();

        if (m_points.count() > 2) {
            KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
            if (!kisCanvas || !currentNode())
                return;

            QApplication::setOverrideCursor(KisCursor::waitCursor());

            KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Outline Selection"));

            if (selectionMode() == PIXEL_SELECTION) {

                KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

                KisPainter painter(tmpSel);
                painter.setBounds(currentImage()->bounds());
                painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
                painter.setPaintOpPreset(currentPaintOpPreset(), currentImage());
                painter.setAntiAliasPolygonFill(selectionOptionWidget()->antiAliasSelection());
                painter.setFillStyle(KisPainter::FillStyleForegroundColor);
                painter.setStrokeStyle(KisPainter::StrokeStyleNone);

                painter.paintPolygon(m_points);

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

                helper.addSelectionShape(path);
            }
            QApplication::restoreOverrideCursor();
        }

        m_points.clear();
        delete m_paintPath;
        m_paintPath = new QPainterPath();
    }
    else {
        KisToolSelectBase::mouseReleaseEvent(event);
    }
}


void KisToolSelectOutline::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (mode() == KisTool::PAINT_MODE && !m_points.isEmpty()) {
            paintToolOutline(&gc, *m_paintPath);
    }
}

#define FEEDBACK_LINE_WIDTH 2

void KisToolSelectOutline::updateFeedback()
{
    if (m_points.count() > 1) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_points[lastPointIndex]).normalized();
        updateRect.adjust(-FEEDBACK_LINE_WIDTH, -FEEDBACK_LINE_WIDTH, FEEDBACK_LINE_WIDTH, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

void KisToolSelectOutline::deactivate()
{
    if (canvas()) {
        updateCanvasPixelRect(image()->bounds());
    }

    KisToolSelectBase::deactivate();
}

#include "kis_tool_select_outline.moc"

