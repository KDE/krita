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
        : KisTool(canvas, KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6))
{
    m_selecting = false;
    m_centerPos = QPointF(0, 0);
    m_startPos = QPointF(0, 0);
    m_endPos = QPointF(0, 0);
    m_optWidget = 0;
    m_selectAction = SELECTION_REPLACE;
    m_selectionMode = PIXEL_SELECTION;
}

KisToolSelectRectangular::~KisToolSelectRectangular()
{
}

void KisToolSelectRectangular::activate(bool tmp)
{
    KisTool::activate(tmp);

    if (!m_optWidget)
        return;

    m_optWidget->slotActivated();
}

void KisToolSelectRectangular::paint(QPainter& gc, const KoViewConverter &converter)
{
    qreal sx, sy;
    converter.zoom(&sx, &sy);

    gc.scale(sx / currentImage()->xRes(), sy / currentImage()->yRes());
    if (m_selecting) {
        QPen old = gc.pen();
        gc.setPen(Qt::DashLine);

        QRectF rectangle(m_startPos.x(), m_startPos.y(), m_endPos.x() - m_startPos.x(), m_endPos.y() - m_startPos.y());
        gc.drawRect(rectangle);

        gc.setPen(old);
    }
}

void KisToolSelectRectangular::clearSelection()
{
    if (m_canvas) {
        m_centerPos = QPointF(0, 0);
        m_startPos = QPointF(0, 0);
        m_endPos = QPointF(0, 0);
        m_selecting = false;
    }
}

void KisToolSelectRectangular::mousePressEvent(KoPointerEvent *e)
{
    if (m_canvas) {

        if (!currentNode())
            return;

        if (currentImage() && currentNode()->paintDevice() && e->button() == Qt::LeftButton) {
            clearSelection();
            m_startPos = m_endPos = m_centerPos = convertToPixelCoord(e);
            m_selecting = true;
        }
    }
}

void KisToolSelectRectangular::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_canvas && m_selecting) {
        QRectF updateRect(m_startPos, m_endPos);

        // move (alt) or resize rectangle
        if (e->modifiers() & Qt::AltModifier) {
            QPointF trans = convertToPixelCoord(e) - m_endPos;
            m_startPos += trans;
            m_endPos += trans;
        } else {
            QPointF diag = convertToPixelCoord(e) - (e->modifiers() & Qt::ControlModifier
                           ? m_centerPos : m_startPos);
            // square?
            if (e->modifiers() & Qt::ShiftModifier) {
                double size = qMax(fabs(diag.x()), fabs(diag.y()));
                double w = diag.x() < 0 ? -size : size;
                double h = diag.y() < 0 ? -size : size;
                diag = QPointF(w, h);
            }

            // resize around center point?
            if (e->modifiers() & Qt::ControlModifier) {
                m_startPos = m_centerPos - diag;
                m_endPos = m_centerPos + diag;
            } else {
                m_endPos = m_startPos + diag;
            }
        }

        updateRect |= QRectF(m_startPos, m_endPos);
        updateRect = updateRect.normalized();
        updateRect.adjust(-1, -1, 1, 1);
        m_canvas->updateCanvas(convertToPt(updateRect));

        m_centerPos = QPointF((m_startPos.x() + m_endPos.x()) / 2,
                              (m_startPos.y() + m_endPos.y()) / 2);
    }
}

void KisToolSelectRectangular::mouseReleaseEvent(KoPointerEvent *e)
{
    if (m_canvas && m_selecting && e->button() == Qt::LeftButton) {

        QRectF bound;
        bound.setTopLeft(m_startPos);
        bound.setBottomRight(m_endPos);
        m_canvas->updateCanvas(convertToPt(bound.normalized()));

        if (m_startPos == m_endPos) {
            clearSelection();
            m_selecting = false;
            return;
        }

        if (!currentImage())
            return;

        QRect rc(m_startPos.toPoint(), m_endPos.toPoint());
        rc = rc.intersected(currentImage()->bounds());
        rc = rc.normalized();

        KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(m_canvas);
        if (!kisCanvas)
            return;

        KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Rectangular Selection"));

        if (m_selectionMode == PIXEL_SELECTION) {

            // We don't want the border of the 'rectangle' to be included in our selection
            rc.setSize(rc.size() - QSize(1, 1));
            rc = rc.normalized();
            if (rc.width() > 0 && rc.height() > 0) {
                KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());
                tmpSel->select(rc);

                QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectAction);
                m_canvas->addCommand(cmd);
            }
        } else {
            QRectF documentRect = convertToPt(bound);

            KoShape* shape;
            KoShapeFactory *rectFactory = KoShapeRegistry::instance()->value("KoRectangleShape");
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
        m_selecting = false;
    }
}

void KisToolSelectRectangular::slotSetAction(int action)
{
    if (action >= SELECTION_REPLACE && action <= SELECTION_INTERSECT)
        m_selectAction = (selectionAction)action;
}

void KisToolSelectRectangular::slotSetSelectionMode(int mode)
{
    m_selectionMode = (selectionMode)mode;

}

QWidget* KisToolSelectRectangular::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(m_canvas);
    Q_ASSERT(canvas);
    m_optWidget = new KisSelectionOptions(canvas);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");

    m_optWidget->setWindowTitle(i18n("Rectangular Selection"));
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

QWidget* KisToolSelectRectangular::optionWidget()
{
    return m_optWidget;
}
#include "kis_tool_select_rectangular.moc"
