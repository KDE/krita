/*
 *  kis_tool_select_rectangular.cc -- part of Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *                2001 John Califf <jcaliff@compuzone.net>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#include "kis_tool_select_rectangular.h"

#include <QApplication>
#include <QPainter>
#include <QPen>
#include <QLayout>
#include <QVBoxLayout>
#include <QUndoCommand>

#include <kis_debug.h>
#include <klocale.h>

#include <KoShapeController.h>
#include <KoPathShape.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>

#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_layer.h"

#include "KoPointerEvent.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "canvas/kis_canvas2.h"
#include "flake/kis_shape_selection.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"

KisToolSelectRectangular::KisToolSelectRectangular(KoCanvasBase * canvas)
        : KisToolSelectBase(canvas, KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6)),
        m_lokalTool(canvas, this)
{
}

KisToolSelectRectangular::~KisToolSelectRectangular()
{
}

QWidget* KisToolSelectRectangular::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    m_optWidget->setWindowTitle(i18n("Rectangular Selection"));
    m_optWidget->disableAntiAliasSelectionOption();
    return m_optWidget;
}

void KisToolSelectRectangular::LokalTool::finishRect(const QRectF& rect)
{
    QRect rc(rect.toRect());
    rc = rc.intersected(currentImage()->bounds());
    rc = rc.normalized();

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Rectangular Selection"));

    if (m_selectingTool->m_selectionMode == PIXEL_SELECTION) {

        // We don't want the border of the 'rectangle' to be included in our selection
        rc.setSize(rc.size() - QSize(1, 1));
        if (rc.width() > 0 && rc.height() > 0) {
            KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());
            tmpSel->select(rc);

            QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectingTool->m_selectAction);
            canvas()->addCommand(cmd);
        }
    } else {
        QRectF documentRect = convertToPt(rect);

        KoShape* shape;
        KoShapeFactory *rectFactory = KoShapeRegistry::instance()->value("RectangleShape");
        if (rectFactory) {
            // it is ok to use a empty map here as the data is not needed.
            QMap<QString, KoDataCenter *> dataCenterMap;
            shape = rectFactory->createDefaultShapeAndInit(dataCenterMap);
            shape->setSize(documentRect.size());
            shape->setPosition(documentRect.topLeft());
        } else {
            //Fallback if the plugin wasn't found
            KoPathShape* path = new KoPathShape();
            path->setShapeId(KoPathShapeId);
            path->moveTo(documentRect.topLeft());
            path->lineTo(documentRect.topLeft() + QPointF(documentRect.width(), 0));
            path->lineTo(documentRect.bottomRight());
            path->lineTo(documentRect.topLeft() + QPointF(0, documentRect.height()));
            path->close();
            path->normalize();
            shape = path;
        }

        helper.addSelectionShape(shape);
    }
}

#include "kis_tool_select_rectangular.moc"
