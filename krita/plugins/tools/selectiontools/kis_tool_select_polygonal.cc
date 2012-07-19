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

#include <KoPathShape.h>

#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_selection_options.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_shape_tool_helper.h"

#include "kis_system_locker.h"


KisToolSelectPolygonal::KisToolSelectPolygonal(KoCanvasBase *canvas)
    : KisToolPolylineBase(canvas, KisToolPolylineBase::SELECT,
                          KisCursor::load("tool_polygonal_selection_cursor.png", 6, 6)),
      m_widgetHelper(i18n("Polygonal Selection"))
{
    setObjectName("tool_select_polygonal");
}

QWidget* KisToolSelectPolygonal::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
    Q_ASSERT(canvas);

    m_widgetHelper.createOptionWidget(canvas, this->toolId());
    return m_widgetHelper.optionWidget();
}

void KisToolSelectPolygonal::keyPressEvent(QKeyEvent *event)
{
    if (!m_widgetHelper.processKeyPressEvent(event)) {
        KisTool::keyPressEvent(event);
    }
}

void KisToolSelectPolygonal::finishPolyline(const QVector<QPointF> &points)
{
    KisSystemLocker locker(currentNode());
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Polygonal Selection"));

    if (m_widgetHelper.selectionMode() == PIXEL_SELECTION) {
        KisPixelSelectionSP tmpSel = new KisPixelSelection();

        KisPainter painter(tmpSel);
        painter.setBounds(currentImage()->bounds());
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setPaintOpPreset(currentPaintOpPreset(), currentImage()); // And now the painter owns the op and will destroy it.
        painter.setAntiAliasPolygonFill(m_widgetHelper.optionWidget()->antiAliasSelection());
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);

        painter.paintPolygon(points);

        helper.selectPixelSelection(tmpSel, m_widgetHelper.selectionAction());
    } else {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QTransform resolutionMatrix;
        resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
        path->moveTo(resolutionMatrix.map(points[0]));
        for (int i = 1; i < points.count(); i++)
            path->lineTo(resolutionMatrix.map(points[i]));
        path->close();
        path->normalize();

        helper.addSelectionShape(path);
    }
}
