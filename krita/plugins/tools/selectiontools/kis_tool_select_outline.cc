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

#include "kis_selected_transaction.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "canvas/kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"


KisToolSelectOutline::KisToolSelectOutline(KoCanvasBase * canvas)
        : KisToolSelectBase(canvas, KisCursor::load("tool_outline_selection_cursor.png", 5, 5)), m_paintPath(new QPainterPath())
{
    m_dragging = false;
}

KisToolSelectOutline::~KisToolSelectOutline()
{
    delete m_paintPath;
}


void KisToolSelectOutline::mousePressEvent(KoPointerEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_points.clear();
        m_points.append(convertToPixelCoord(event));

        m_paintPath->moveTo(pixelToView(convertToPixelCoord(event)));
    }
}

void KisToolSelectOutline::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        QPointF point = convertToPixelCoord(event);
        //don't add the point, if the distance is very small
        if (m_points.size()>0) {
            QPointF diff = point-m_points.at(m_points.size()-1);
            if(fabs(diff.x())<3 && fabs(diff.y())<3)
                return;
        }
        m_paintPath->lineTo(pixelToView(point));
        m_points.append(point);
        updateFeedback();
    }
}

void KisToolSelectOutline::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!canvas())
        return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        m_dragging = false;
        deactivate();

        if (currentImage() && m_points.count() > 2) {
            QApplication::setOverrideCursor(KisCursor::waitCursor());

            KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
            if (!kisCanvas)
                return;

            if (!currentNode())
                return;

            KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Outline Selection"));

            if (m_selectionMode == PIXEL_SELECTION) {

                KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

                KisPainter painter(tmpSel);
                painter.setBounds(currentImage()->bounds());
                painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
                painter.setFillStyle(KisPainter::FillStyleForegroundColor);
                painter.setStrokeStyle(KisPainter::StrokeStyleNone);
                painter.setOpacity(OPACITY_OPAQUE);
                painter.setPaintOpPreset(currentPaintOpPreset(), currentImage());
                painter.setAntiAliasPolygonFill(m_optWidget->antiAliasSelection());
                painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));
                painter.paintPolygon(m_points);

                QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectAction);
                canvas()->addCommand(cmd);
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

        m_points.clear();
        delete m_paintPath;
        m_paintPath = new QPainterPath();
    }
}


void KisToolSelectOutline::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (m_dragging && m_points.count() > 1) {

 #if defined(HAVE_OPENGL)
        if (isCanvasOpenGL()) {
            beginOpenGL();

            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_COLOR_LOGIC_OP);
            glLogicOp(GL_XOR);

            glBegin(GL_LINE_STRIP);
            glColor3f(0.5, 1.0, 0.5);

            for (qint32 pointIndex = 0; pointIndex < m_points.count(); pointIndex++) {
                QPointF point = pixelToView(m_points[pointIndex]);
                glVertex2f(point.x(), point.y());
            }

            glEnd();

            glDisable(GL_COLOR_LOGIC_OP);
            glDisable(GL_LINE_SMOOTH);

            endOpenGL();
        } else
#endif
        {
            paintToolOutline(&gc, *m_paintPath);
        }
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
}

QWidget* KisToolSelectOutline::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    m_optWidget->setWindowTitle(i18n("Outline Selection"));
    return m_optWidget;
}


#include "kis_tool_select_outline.moc"

