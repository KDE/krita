/*
 *  kis_tool_star.cc -- part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include <KoPathShape.h>
#include <KoLineBorder.h>

#include <kis_debug.h>
#include <canvas/kis_canvas2.h>
#include <kis_paintop_registry.h>
#include <kis_cursor.h>
#include <kis_paint_information.h>
#include "kis_figure_painting_tool_helper.h"
#include <kis_system_locker.h>
#include "widgets/kis_slider_spin_box.h"

#include <recorder/kis_action_recorder.h>
#include <recorder/kis_recorded_path_paint_action.h>
#include <recorder/kis_node_query_path.h>


KisToolStar::KisToolStar(KoCanvasBase * canvas)
        : KisToolShape(canvas, KisCursor::load("tool_star_cursor.png", 6, 6))
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
    if(PRESS_CONDITION(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::NoModifier)) {

        if (nodePaintAbility() == NONE)
            return;

        setMode(KisTool::PAINT_MODE);

        m_dragStart = convertToPixelCoord(event);
        m_dragEnd = convertToPixelCoord(event);
        m_vertices = m_verticesSlider->value();
        m_innerOuterRatio = m_ratioSlider->value();
    }
    else {
        KisToolShape::mousePressEvent(event);
    }
}

void KisToolStar::mouseMoveEvent(KoPointerEvent *event)
{
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        //Erase old lines
        updatePreview();
        if (event->modifiers() & Qt::AltModifier) {
            QPointF trans = convertToPixelCoord(event) - m_dragEnd;
            m_dragStart += trans;
            m_dragEnd += trans;
        } else {
            m_dragEnd = convertToPixelCoord(event);
        }
        updatePreview();
    }
    else {
        KisToolShape::mouseMoveEvent(event);
    }
}

void KisToolStar::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        if (m_dragStart == m_dragEnd)
            return;

        if (!currentImage())
            return;

        if (!currentNode())
            return;

        vQPointF coord = starCoordinates(m_vertices, m_dragStart.x(), m_dragStart.y(), m_dragEnd.x(), m_dragEnd.y());
        if (image()) {
            KisRecordedPathPaintAction linePaintAction(KisNodeQueryPath::absolutePath(currentNode()), currentPaintOpPreset());
            setupPaintAction(&linePaintAction);
            linePaintAction.addPolyLine(coord.toList());
            linePaintAction.addLine(KisPaintInformation(coord.last()), KisPaintInformation(coord.first()));
            image()->actionRecorder()->addAction(linePaintAction);
        }

        if (!currentNode()->inherits("KisShapeLayer")) {
            KisSystemLocker locker(currentNode());

            KisFigurePaintingToolHelper helper(i18n("Star"),
                                               image(),
                                               canvas()->resourceManager());
            helper.paintPolygon(coord);
        } else {
            KoPathShape* path = new KoPathShape();
            path->setShapeId(KoPathShapeId);

            QTransform resolutionMatrix;
            resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
            path->moveTo(resolutionMatrix.map(coord[0]));
            for (int i = 1; i < coord.count(); i++)
                path->lineTo(resolutionMatrix.map(coord[i]));
            path->close();
            path->normalize();

            KoLineBorder* border = new KoLineBorder(1.0, currentFgColor().toQColor());
            path->setBorder(border);

            addShape(path);
        }
        notifyModified();
    }
    else {
        KisToolShape::mouseReleaseEvent(event);
    }
}

void KisToolStar::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (mode() != KisTool::PAINT_MODE)
        return;

    vQPointF points = starCoordinates(m_vertices, m_dragStart.x(), m_dragStart.y(), m_dragEnd.x(), m_dragEnd.y());

    QPainterPath path;
    for (int i = 0; i < points.count() - 1; i++) {
        path.moveTo(pixelToView(points[i]));
        path.lineTo(pixelToView(points[i + 1]));
    }
    path.moveTo(pixelToView(points[points.count() - 1]));
    path.lineTo(pixelToView(points[0]));
    paintToolOutline(&gc, path);
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

void KisToolStar::updatePreview() {
    //Calculating the radius
    double radius = sqrt((m_dragEnd.x() - m_dragStart.x()) * (m_dragEnd.x() - m_dragStart.x()) + (m_dragEnd.y() - m_dragStart.y()) * ((m_dragEnd.y() - m_dragStart.y())));
    canvas()->updateCanvas(convertToPt(QRectF(m_dragStart.x() - radius, m_dragStart.y() - radius, 2*radius, 2*radius)));
}


QWidget* KisToolStar::createOptionWidget()
{
    QWidget *widget = KisToolShape::createOptionWidget();
    widget->setObjectName(toolId() + "option widget");

    QLabel *lblVertices = new QLabel(i18n("Vertices:"), widget);
    m_verticesSlider = new KisSliderSpinBox(widget);
    m_verticesSlider->setRange(2, 100);
    m_verticesSlider->setValue(5);
    addOptionWidgetOption(m_verticesSlider, lblVertices);

    QLabel *lblRatio = new QLabel(i18n("Ratio:"), widget);
    m_ratioSlider = new KisSliderSpinBox(widget);
    m_ratioSlider->setRange(0, 100);
    m_ratioSlider->setValue(m_innerOuterRatio);

    addOptionWidgetOption(m_ratioSlider, lblRatio);

    return widget;
}

#include "kis_tool_star.moc"
