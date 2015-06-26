/*
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

#include "kis_tool_select_path.h"

#include <KoPathShape.h>

#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_selection_options.h"
#include "kis_canvas_resource_provider.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include <KisView.h>


KisToolSelectPath::KisToolSelectPath(KoCanvasBase * canvas)
    : SelectionActionHandler<KisDelegatedSelectPathWrapper>(canvas,
                              KisCursor::load("tool_polygonal_selection_cursor.png", 6, 6),
			      i18n("Select path"),
                              (KisTool*) (new __KisToolSelectPathLocalTool(canvas, this)))
{
}

void KisToolSelectPath::requestStrokeEnd()
{
     localTool()->endPathWithoutLastPoint();
}

void KisToolSelectPath::requestStrokeCancellation()
{
 localTool()->cancelPath();
}

void KisToolSelectPath::mousePressEvent(KoPointerEvent* event)
{
    if (!selectionEditable()) return;
    DelegatedSelectPathTool::mousePressEvent(event);
}

QList<QPointer<QWidget> > KisToolSelectPath::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgetsList =
        DelegatedSelectPathTool::createOptionWidgets();
    selectionOptionWidget()->disableAntiAliasSelectionOption();
    return widgetsList;
}

void KisToolSelectPath::setAlternateSelectionAction(SelectionAction action)
{
    // We will turn off the ability to change the selection in the middle of drawing a path.
    if (!m_localTool->listeningToModifiers()) {
      SelectionActionHandler<KisDelegatedSelectPathWrapper>::setAlternateSelectionAction(action);
    }
}


bool KisDelegatedSelectPathWrapper::listeningToModifiers() {
  return m_localTool->listeningToModifiers();
}

void KisDelegatedSelectPathWrapper::beginPrimaryAction(KoPointerEvent *event) {
 mousePressEvent(event);
}

void KisDelegatedSelectPathWrapper::continuePrimaryAction(KoPointerEvent *event){
 mouseMoveEvent(event);
}

void KisDelegatedSelectPathWrapper::endPrimaryAction(KoPointerEvent *event) {
 mouseReleaseEvent(event);
}

 __KisToolSelectPathLocalTool::__KisToolSelectPathLocalTool(KoCanvasBase * canvas, KisToolSelectPath* parentTool)
     : KoCreatePathTool(canvas), m_selectionTool(parentTool)
{
}

void __KisToolSelectPathLocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    QTransform matrix;
    matrix.scale(kisCanvas->image()->xRes(), kisCanvas->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());
    m_selectionTool->paintToolOutline(&painter, m_selectionTool->pixelToView(matrix.map(pathShape.outline())));
}

void __KisToolSelectPathLocalTool::addPathShape(KoPathShape* pathShape)
{
    pathShape->normalize();
    pathShape->close();

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    KisImageWSP image = kisCanvas->image();

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select by Bezier Curve"));

    if (m_selectionTool->selectionMode() == PIXEL_SELECTION) {

        KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

        KisPainter painter(tmpSel);
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);

        QTransform matrix;
        matrix.scale(image->xRes(), image->yRes());
        matrix.translate(pathShape->position().x(), pathShape->position().y());

        QPainterPath path = matrix.map(pathShape->outline());
        painter.fillPainterPath(path);
        tmpSel->setOutlineCache(path);

        helper.selectPixelSelection(tmpSel, m_selectionTool->selectionAction());

        delete pathShape;
    } else {
        helper.addSelectionShape(pathShape);
    }
}
