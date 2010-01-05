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
        : KisToolSelectBase(canvas, KisCursor::load("tool_polygonal_selection_cursor.png", 6, 6)),
        m_lokalTool(canvas, this)
{
    setObjectName("tool_select_polygonal");
}

KisToolSelectPolygonal::~KisToolSelectPolygonal()
{
}

void KisToolSelectPolygonal::keyPressEvent(QKeyEvent *e)
{
    m_lokalTool.keyPressEvent(e);
    KisToolSelectBase::keyPressEvent(e);
}

QWidget* KisToolSelectPolygonal::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    m_optWidget->setWindowTitle(i18n("Polygonal Selection"));
    return m_optWidget;
}

void KisToolSelectPolygonal::LokalTool::finishPolygon(const QVector<QPointF> &points)
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(m_canvas);
    Q_ASSERT(kisCanvas);
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Polygonal Selection"));

    if (m_selectingTool->m_selectionMode == PIXEL_SELECTION) {

        KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

        KisPainter painter(tmpSel);
        painter.setBounds(currentImage()->bounds());
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);
        painter.setAntiAliasPolygonFill(m_selectingTool->m_optWidget->antiAliasSelection());
        painter.setOpacity(OPACITY_OPAQUE);
        painter.setPaintOpPreset(m_selectingTool->currentPaintOpPreset(), currentImage()); // And now the painter owns the op and will destroy it.
        painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));
        painter.paintPolygon(points);

        QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectingTool->m_selectAction);
        m_canvas->addCommand(cmd);
    } else {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QMatrix resolutionMatrix;
        resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
        path->moveTo(resolutionMatrix.map(points[0]));
        for (int i = 1; i < points.count(); i++)
            path->lineTo(resolutionMatrix.map(points[i]));
        path->close();
        path->normalize();

        helper.addSelectionShape(path);
    }
}

#include "kis_tool_select_polygonal.moc"
