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

#include <QPainter>
#include <QPen>
#include <QLayout>
#include <QVBoxLayout>
#include <QTransform>

#include <klocale.h>

#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoPointerEvent.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoLineBorder.h>
#include <KoViewConverter.h>

#include "kis_image.h"
#include "kis_painter.h"
#include "kis_layer.h"

#include "kis_selection.h"
#include "kis_selection_options.h"
#include "canvas/kis_canvas2.h"
#include "kis_selection_tool_helper.h"
#include "kis_pixel_selection.h"
#include "kis_canvas_resource_provider.h"
#include "kis_paintop_registry.h"


KisToolSelectPath::KisToolSelectPath(KoCanvasBase * canvas)
        : KisToolSelectBase(canvas), m_localTool(new LocalTool(canvas, this))
{
}

KisToolSelectPath::~KisToolSelectPath()
{
    delete m_localTool;
}

void KisToolSelectPath::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisToolSelectBase::activate(toolActivation, shapes);
    m_localTool->activate(toolActivation, shapes);
}

void KisToolSelectPath::deactivate()
{
    KisToolSelectBase::deactivate();
    m_localTool->deactivate();
}

void KisToolSelectPath::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton, Qt::ShiftModifier |
                          Qt::ControlModifier | Qt::AltModifier)) {

        if (!currentNode()) {
            return;
        }

        setMode(KisTool::PAINT_MODE);

        Q_ASSERT(m_localTool);
        m_localTool->mousePressEvent(event);
    }
    else {
        KisToolSelectBase::mousePressEvent(event);
    }
}

void KisToolSelectPath::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton, Qt::ShiftModifier |
                          Qt::ControlModifier | Qt::AltModifier)) {

        Q_ASSERT(m_localTool);
        m_localTool->mouseDoubleClickEvent(event);
    }
    else {
        KisToolSelectBase::mouseDoubleClickEvent(event);
    }
}

void KisToolSelectPath::mouseMoveEvent(KoPointerEvent *event)
{
    Q_ASSERT(m_localTool);
    m_localTool->mouseMoveEvent(event);

    KisToolSelectBase::mouseMoveEvent(event);
}

void KisToolSelectPath::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        Q_ASSERT(m_localTool);
        m_localTool->mouseReleaseEvent(event);
    }
    else {
        KisToolSelectBase::mouseReleaseEvent(event);
    }
}

QWidget* KisToolSelectPath::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    m_optWidget->setWindowTitle(i18n("Path Selection"));
    m_optWidget->disableAntiAliasSelectionOption();

    return m_optWidget;
}

void KisToolSelectPath::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_ASSERT(m_localTool);
    m_localTool->paint(painter, converter);
}

QList<QWidget *> KisToolSelectPath::createOptionWidgets()
{
    QList<QWidget *> list = m_localTool->createOptionWidgets();
    KisToolSelectBase::createOptionWidget()->setWindowTitle(i18n("Tool Options"));
    list.append(KisToolSelectBase::createOptionWidget());
    return list;
}

KisToolSelectPath::LocalTool::LocalTool(KoCanvasBase * canvas, KisToolSelectPath* selectingTool)
        : KoCreatePathTool(canvas), m_selectingTool(selectingTool) {}

void KisToolSelectPath::LocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    QTransform matrix;
    matrix.scale(kisCanvas->image()->xRes(), kisCanvas->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());
    m_selectingTool->paintToolOutline(&painter, m_selectingTool->pixelToView(matrix.map(pathShape.outline())));
}

void KisToolSelectPath::LocalTool::addPathShape(KoPathShape* pathShape)
{
    KisNodeSP currentNode =
        canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    if (!currentNode)
        return;

    pathShape->normalize();
    pathShape->close();

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    KisImageWSP image = kisCanvas->image();

    KisSelectionToolHelper helper(kisCanvas, currentNode, i18n("Path Selection"));

    if (m_selectingTool->m_selectionMode == PIXEL_SELECTION) {

        KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

        KisPainter painter(tmpSel);
        painter.setBounds(m_selectingTool->currentImage()->bounds());
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);
        painter.setOpacity(OPACITY_OPAQUE_U8);
        painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));

        QTransform matrix;
        matrix.scale(image->xRes(), image->yRes());
        matrix.translate(pathShape->position().x(), pathShape->position().y());
        painter.fillPainterPath(matrix.map(pathShape->outline()));

        helper.selectPixelSelection(tmpSel, m_selectingTool->m_selectAction);

        delete pathShape;
    } else {
        helper.addSelectionShape(pathShape);
    }
}

#include "kis_tool_select_path.moc"
