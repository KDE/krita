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

#include <recorder/kis_action_recorder.h>
#include <recorder/kis_recorded_path_paint_action.h>
#include <recorder/kis_node_query_path.h>

KisToolPath::KisToolPath(KoCanvasBase * canvas)
        : KoCreatePathTool(canvas)
{
}

KisToolPath::~KisToolPath()
{
}

void KisToolPath::addPathShape()
{
    KisNodeSP currentNode =
        canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    KisCanvas2 *kiscanvas = dynamic_cast<KisCanvas2 *>(this->canvas());
    if (!currentNode || !kiscanvas) {
        delete m_shape;
        m_shape = 0;
        return;
    }
    // Get painting options
    KisPaintOpPresetSP preset = kiscanvas->resourceManager()->
                                resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
    KoColor paintColor = canvas()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();

    // Compute the outline
    KisImageWSP image = kiscanvas->view()->image();
    QMatrix matrix;
    matrix.scale(image->xRes(), image->yRes());
    matrix.translate(m_shape->position().x(), m_shape->position().y());
    QPainterPath mapedOutline = matrix.map(m_shape->outline());

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
            delete m_shape;
            m_shape = 0;
            return;
        }

        KisSelectionSP selection = kiscanvas->view()->selection();

        KisPainter painter(dev, selection);
        painter.beginTransaction(i18n("Path"));

        if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(currentNode.data())) {
            painter.setChannelFlags(l->channelFlags());
            if (l->alphaLocked()) {
                painter.setLockAlpha(l->alphaLocked());
            }
        }
        painter.setPaintColor(paintColor);
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);
        painter.setOpacity(OPACITY_OPAQUE);
        painter.setCompositeOp(dev->colorSpace()->compositeOp(COMPOSITE_OVER));
        painter.setPaintOpPreset(preset, image);

        painter.paintPainterPath(mapedOutline);
        QRegion dirtyRegion = painter.dirtyRegion();
        dev->setDirty(dirtyRegion);
        image->setModified();

        kiscanvas->addCommand(painter.endTransaction());
        delete m_shape;

    } else {
        m_shape->normalize();
        QUndoCommand * cmd = canvas()->shapeController()->addShape(m_shape);
        canvas()->addCommand(cmd);
    }
    m_shape = 0;
}

#include "kis_tool_path.moc"
