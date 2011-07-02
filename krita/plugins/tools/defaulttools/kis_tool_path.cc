/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_tool_path.h"
#include <klocale.h>
#include <KoPathShape.h>
#include <KoCanvasBase.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoShapeController.h>

#include <kis_paint_layer.h>
#include <kis_image.h>
#include <kis_painter.h>
#include <kis_paint_information.h>
#include <kis_layer.h>
#include <canvas/kis_canvas2.h>
#include <kis_view2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_paintop_registry.h>
#include <kis_selection.h>
#include <kis_cursor.h>

#include <recorder/kis_action_recorder.h>
#include <recorder/kis_recorded_path_paint_action.h>
#include <recorder/kis_node_query_path.h>


KisToolPath::KisToolPath(KoCanvasBase * canvas)
        : KisToolShape(canvas, Qt::ArrowCursor), m_localTool(new LocalTool(canvas, this))
{
}

KisToolPath::~KisToolPath()
{
    delete m_localTool;
}

void KisToolPath::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisToolShape::activate(toolActivation, shapes);
    m_localTool->activate(toolActivation, shapes);
}

void KisToolPath::deactivate()
{
    KisToolShape::deactivate();
    m_localTool->deactivate();
}

void KisToolPath::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton, Qt::ShiftModifier |
                          Qt::ControlModifier | Qt::AltModifier)) {

        setMode(KisTool::PAINT_MODE);

        Q_ASSERT(m_localTool);
        m_localTool->mousePressEvent(event);
    }
    else {
        KisToolShape::mousePressEvent(event);
    }
}

void KisToolPath::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton, Qt::ShiftModifier |
                          Qt::ControlModifier | Qt::AltModifier)) {

        Q_ASSERT(m_localTool);
        m_localTool->mouseDoubleClickEvent(event);
    }
    else {
        KisToolShape::mouseDoubleClickEvent(event);
    }
}

void KisToolPath::mouseMoveEvent(KoPointerEvent *event)
{
    Q_ASSERT(m_localTool);
    m_localTool->mouseMoveEvent(event);

    KisToolShape::mouseMoveEvent(event);
}

void KisToolPath::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        Q_ASSERT(m_localTool);
        m_localTool->mouseReleaseEvent(event);
    }
    else {
        KisToolShape::mouseReleaseEvent(event);
    }
}

void KisToolPath::addPathShape(KoPathShape* pathShape)
{
    KisNodeSP currentNode =
        canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    KisCanvas2 *kiscanvas = dynamic_cast<KisCanvas2 *>(this->canvas());
    if (!currentNode || !kiscanvas || currentNode->systemLocked()) {
        return;
    }
    // Get painting options
    KisPaintOpPresetSP preset = kiscanvas->resourceManager()->
                                resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
    KoColor paintColor = canvas()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();

    // Compute the outline
    KisImageWSP image = kiscanvas->view()->image();
    QTransform matrix;
    matrix.scale(image->xRes(), image->yRes());
    matrix.translate(pathShape->position().x(), pathShape->position().y());
    QPainterPath mapedOutline = matrix.map(pathShape->outline());

    // Recorde the paint action
    KisRecordedPathPaintAction bezierCurvePaintAction(
            KisNodeQueryPath::absolutePath(currentNode),
            preset );
    bezierCurvePaintAction.setPaintColor(paintColor);
    QPointF lastPoint, nextPoint;
    int elementCount = mapedOutline.elementCount();
    for (int i = 0; i < elementCount; i++) {
        QPainterPath::Element element = mapedOutline.elementAt(i);
        switch (element.type) {
        case QPainterPath::MoveToElement:
            if (i != 0) {
                qFatal("Unhandled"); // XXX: I am not sure the tool can produce such element, deal with it when it can
            }
            lastPoint =  QPointF(element.x, element.y);
            break;
        case QPainterPath::LineToElement:
            nextPoint =  QPointF(element.x, element.y);
            bezierCurvePaintAction.addLine(KisPaintInformation(lastPoint), KisPaintInformation(nextPoint));
            lastPoint = nextPoint;
            break;
        case QPainterPath::CurveToElement:
            nextPoint =  QPointF(mapedOutline.elementAt(i + 2).x, mapedOutline.elementAt(i + 2).y);
            bezierCurvePaintAction.addCurve(KisPaintInformation(lastPoint),
                                             QPointF(mapedOutline.elementAt(i).x,
                                                     mapedOutline.elementAt(i).y),
                                             QPointF(mapedOutline.elementAt(i + 1).x,
                                                     mapedOutline.elementAt(i + 1).y),
                                             KisPaintInformation(nextPoint));
            lastPoint = nextPoint;
            break;
        default:
            continue;
        }
    }
    image->actionRecorder()->addAction(bezierCurvePaintAction);

    if (!currentNode->inherits("KisShapeLayer")) {

        KisPaintDeviceSP dev = currentNode->paintDevice();

        if (!dev) {
            return;
        }

        setCurrentNodeLocked(true);

        KisSelectionSP selection = kiscanvas->view()->selection();

        KisPainter painter(dev, selection);
        painter.beginTransaction(i18n("Path"));

        setupPainter(&painter);
        painter.paintPainterPath(mapedOutline);
        painter.endTransaction(image->undoAdapter());

        dev->setDirty(painter.takeDirtyRegion());
        image->setModified();
        setCurrentNodeLocked(false);

    } else {
        pathShape->normalize();
        addShape(pathShape);
    }
}

void KisToolPath::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_ASSERT(m_localTool);
    m_localTool->paint(painter, converter);
}

QList<QWidget *> KisToolPath::createOptionWidgets()
{
    QList<QWidget *> list = KisToolShape::createOptionWidgets();
    list.append(m_localTool->createOptionWidgets());
    return list;
}

KisToolPath::LocalTool::LocalTool(KoCanvasBase * canvas, KisToolPath* selectingTool)
        : KoCreatePathTool(canvas), m_parentTool(selectingTool) {}

void KisToolPath::LocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    QTransform matrix;
    matrix.scale(kisCanvas->image()->xRes(), kisCanvas->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());
    m_parentTool->paintToolOutline(&painter, m_parentTool->pixelToView(matrix.map(pathShape.outline())));
}

void KisToolPath::LocalTool::addPathShape(KoPathShape* pathShape)
{
    m_parentTool->addPathShape(pathShape);
}

#include "kis_tool_path.moc"
