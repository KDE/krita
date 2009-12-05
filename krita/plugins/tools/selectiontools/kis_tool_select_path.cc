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

#include <QApplication>
#include <QPainter>
#include <QPen>
#include <QLayout>
#include <QVBoxLayout>

#include <klocale.h>

#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoPointerEvent.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

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
        : KoCreatePathTool(canvas)
{
    m_optWidget = 0;
    m_selectAction = SELECTION_REPLACE;
    m_selectionMode = PIXEL_SELECTION;
}

KisToolSelectPath::~KisToolSelectPath()
{
}

void KisToolSelectPath::activate(bool tmp)
{
    KoCreatePathTool::activate(tmp);

    if (!m_optWidget)
        return;

    m_optWidget->slotActivated();
}

void KisToolSelectPath::slotSetAction(int action)
{
    if (action >= SELECTION_REPLACE && action <= SELECTION_INTERSECT)
        m_selectAction = (selectionAction)action;
}

void KisToolSelectPath::slotSetSelectionMode(int mode)
{
    m_selectionMode = (selectionMode)mode;

}

QWidget* KisToolSelectPath::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(m_canvas);
    Q_ASSERT(canvas);
    m_optWidget = new KisSelectionOptions(canvas);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");

    m_optWidget->setWindowTitle(i18n("Path Selection"));
    m_optWidget->disableAntiAliasSelectionOption();

    connect(m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));
    connect(m_optWidget, SIGNAL(modeChanged(int)), this, SLOT(slotSetSelectionMode(int)));


    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    if (l) {
        l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    }
    m_optWidget->setFixedHeight(m_optWidget->sizeHint().height());
    return m_optWidget;
}

QMap<QString, QWidget *> KisToolSelectPath::createOptionWidgets()
{
    QMap<QString, QWidget *> map = KoCreatePathTool::createOptionWidgets();
    map.insert(i18n("Tool Options"), createOptionWidget());
    return map;
}

void KisToolSelectPath::addPathShape()
{
    KisNodeSP currentNode =
        m_canvas->resourceProvider()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    if (!currentNode)
        return;

    KisImageWSP image = qobject_cast<KisLayer*>(currentNode->parent().data())->image();

    m_shape->normalize();

    KoPathShape *shape = m_shape;
    shape->close();
    m_shape = 0;

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(m_canvas);
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, currentNode, i18n("Path Selection"));

    if (m_selectionMode == PIXEL_SELECTION) {

        KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

        KisPainter painter(tmpSel);
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);
        painter.setOpacity(OPACITY_OPAQUE);
        painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));

        QMatrix matrix;
        matrix.scale(image->xRes(), image->yRes());
        matrix.translate(shape->position().x(), shape->position().y());
        painter.fillPainterPath(matrix.map(shape->outline()));

        QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectAction);
        m_canvas->addCommand(cmd);

        delete shape;
    } else {
        helper.addSelectionShape(shape);
    }
}

#include "kis_tool_select_path.moc"
