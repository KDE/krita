/*
 *  kis_tool_star.cc -- part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_tool_star.h"

#include <math.h>

#include <QPainter>
#include <QSpinBox>
#include <QLayout>
#include <QGridLayout>

#include <klocale.h>
#include <knuminput.h>

#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include <KoCanvasController.h>
#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoLineBorder.h>

#include <opengl/kis_opengl.h>
#include <kis_debug.h>
#include <canvas/kis_canvas2.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_cursor.h>
#include <kis_paint_device.h>

#include "kis_selection.h"

KisToolStar::KisToolStar(KoCanvasBase * canvas)
        : KisToolShape(canvas, KisCursor::load("tool_star_cursor.png", 6, 6)),
        m_dragging(false)
{
    setObjectName("tool_star");
    m_innerOuterRatio = 40;
    m_vertices = 5;
}

KisToolStar::~KisToolStar()
{
}
void KisToolStar::mousePressEvent(KoPointerEvent *event)
{
    if (canvas() && event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = convertToPixelCoord(event);
        m_dragEnd = convertToPixelCoord(event);
        m_vertices = m_optWidget->verticesSpinBox->value();
        m_innerOuterRatio = m_optWidget->ratioSpinBox->value();
    }
}

void KisToolStar::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        //Erase old lines
        canvas()->updateCanvas(convertToPt(boundingRect()));
        if (event->modifiers() & Qt::AltModifier) {
            QPointF trans = convertToPixelCoord(event) - m_dragEnd;
            m_dragStart += trans;
            m_dragEnd += trans;
        } else {
            m_dragEnd = convertToPixelCoord(event);
        }
        canvas()->updateCanvas(convertToPt(boundingRect()));
    }
}

void KisToolStar::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!canvas())
        return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        m_dragging = false;

        if (m_dragStart == m_dragEnd)
            return;

        if (!currentImage())
            return;

        if (!currentNode())
            return;

        vQPointF coord = starCoordinates(m_vertices, m_dragStart.x(), m_dragStart.y(), m_dragEnd.x(), m_dragEnd.y());

        if (!currentNode()->inherits("KisShapeLayer")) {

            if (!currentNode()->paintDevice())
                return;

            KisPaintDeviceSP device = currentNode()->paintDevice();
            KisPainter painter(device, currentSelection());
            painter.beginTransaction(i18n("Star"));
            setupPainter(&painter);
            painter.setOpacity(m_opacity);
            painter.setCompositeOp(m_compositeOp);

            painter.paintPolygon(coord);

            device->setDirty(painter.dirtyRegion());
            notifyModified();
            canvas()->updateCanvas(convertToPt(boundingRect()));

            canvas()->addCommand(painter.endTransaction());
        } else {
            KoPathShape* path = new KoPathShape();
            path->setShapeId(KoPathShapeId);

            QMatrix resolutionMatrix;
            resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
            path->moveTo(resolutionMatrix.map(coord[0]));
            for (int i = 1; i < coord.count(); i++)
                path->lineTo(resolutionMatrix.map(coord[i]));
            path->close();
            path->normalize();

            KoLineBorder* border = new KoLineBorder(1.0, currentFgColor().toQColor());
            path->setBorder(border);

            QUndoCommand * cmd = canvas()->shapeController()->addShape(path);
            canvas()->addCommand(cmd);
        }
    }
}

void KisToolStar::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (!m_dragging)
        return;

    if (!canvas())
        return;

    vQPointF points = starCoordinates(m_vertices, m_dragStart.x(), m_dragStart.y(), m_dragEnd.x(), m_dragEnd.y());

#if defined(HAVE_OPENGL)
    if (isCanvasOpenGL()) {
        beginOpenGL();

        QPointF begin, end;

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
        glColor3f(0.501961, 1.0, 0.501961);

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < points.count() - 1; i++) {
            begin = pixelToView(points[i]);
            end = pixelToView(points[i + 1]);

            glVertex2f(begin.x(), begin.y());
            glVertex2f(end.x(), end.y());

        }
        glEnd();

        glDisable(GL_COLOR_LOGIC_OP);
        glDisable(GL_LINE_SMOOTH);

        endOpenGL();
    } else
#endif

#ifdef INDEPENDENT_CANVAS
    {
        QPainterPath path;
        for (int i = 0; i < points.count() - 1; i++) {
            path.moveTo(pixelToView(points[i]));
            path.lineTo(pixelToView(points[i + 1]));
        }
        path.moveTo(pixelToView(points[points.count() - 1]));
        path.lineTo(pixelToView(points[0]));
        paintToolOutline(&gc, path);
    }
#else
    {
        QPen pen(Qt::SolidLine);
        gc.setPen(pen);

        for (int i = 0; i < points.count() - 1; i++) {
            gc.drawLine(pixelToView(points[i]), pixelToView(points[i + 1]));
        }
        gc.drawLine(pixelToView(points[points.count() - 1]), pixelToView(points[0]));
    }
#endif


}

vQPointF KisToolStar::starCoordinates(int N, double mx, double my, double x, double y)
{
    double R = 0, r = 0;
    qint32 n = 0;
    double angle;

    vQPointF starCoordinatesArray(2*N);

    // the radius of the outer edges
    R = sqrt((x - mx) * (x - mx) + (y - my) * (y - my));

    // the radius of the inner edges
    r = R * m_innerOuterRatio / 100.0;

    // the angle
    angle = -atan2((x - mx), (y - my));

    //set outer edges
    for (n = 0; n < N; n++) {
        starCoordinatesArray[2*n] = QPointF(mx + R * cos(n * 2.0 * M_PI / N + angle), my + R * sin(n * 2.0 * M_PI / N + angle));
    }

    //set inner edges
    for (n = 0; n < N; n++) {
        starCoordinatesArray[2*n+1] = QPointF(mx + r * cos((n + 0.5) * 2.0 * M_PI / N + angle), my + r * sin((n + 0.5) * 2.0 * M_PI / N + angle));
    }

    return starCoordinatesArray;
}

QRectF KisToolStar::boundingRect()
{
    //Calculating the radius
    double radius = sqrt((m_dragEnd.x() - m_dragStart.x()) * (m_dragEnd.x() - m_dragStart.x()) + (m_dragEnd.y() - m_dragStart.y()) * ((m_dragEnd.y() - m_dragStart.y())));
    return QRectF(m_dragStart.x() - radius, m_dragStart.y() - radius, 2*radius, 2*radius);
}

QWidget* KisToolStar::createOptionWidget()
{
    QWidget *widget = KisToolShape::createOptionWidget();

    m_optWidget = new WdgToolStar(widget);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");

    m_optWidget->ratioSpinBox->setValue(m_innerOuterRatio);

    QGridLayout *optionLayout = new QGridLayout(widget);
    KisToolShape::addOptionWidgetLayout(optionLayout);

    optionLayout->addWidget(m_optWidget, 0, 0);
    widget->setFixedHeight(widget->sizeHint().height());
    return widget;
}

#include "kis_tool_star.moc"
