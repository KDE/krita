/*
 *  kis_tool_select_elliptical.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "kis_tool_select_elliptical.h"

#include <QApplication>
#include <QPainter>
#include <QPen>
#include <QLayout>
#include <QVBoxLayout>

#include <kdebug.h>
#include <klocale.h>

#include "KoPointerEvent.h"

#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"
#include "kis_canvas2.h"

KisToolSelectElliptical::KisToolSelectElliptical(KoCanvasBase * canvas)
    : KisTool(canvas, KisCursor::load("tool_elliptical_selection_cursor.png", 6, 6))
{
    m_selecting = false;
    m_centerPos = QPointF(0, 0);
    m_startPos = QPointF(0, 0);
    m_endPos = QPointF(0, 0);
    m_optWidget = 0;
    m_selectAction = SELECTION_ADD;
}

KisToolSelectElliptical::~KisToolSelectElliptical()
{
}

void KisToolSelectElliptical::activate()
{
    super::activate();

    if (!m_optWidget)
        return;

    m_optWidget->slotActivated();
}

void KisToolSelectElliptical::paint(QPainter& gc, KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (m_selecting) {
        gc.save();
        gc.setPen(Qt::DotLine);
        gc.drawEllipse(pixelToView(QRectF(m_startPos, m_endPos)));
        gc.restore();
    }
}

void KisToolSelectElliptical::clearSelection()
{
    if (m_canvas) {

//         if (m_currentImage && m_currentImage->floatingSelection().data() != 0) {
//             m_currentImage->unsetFloatingSelection();
//                         controller->canvas()->update();
//         }

        m_startPos = QPointF(0, 0);
        m_endPos = QPointF(0, 0);
        m_selecting = false;
    }
}

void KisToolSelectElliptical::mousePressEvent(KoPointerEvent *e)
{
    if (m_canvas) {

        if (m_currentImage && m_currentImage->activeDevice() && e->button() == Qt::LeftButton) {
            clearSelection();
            m_startPos = m_endPos = m_centerPos = convertToPixelCoord(e);
            m_selecting = true;
        }
    }
}

void KisToolSelectElliptical::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_canvas && m_selecting) {
        QRectF updateRect(m_startPos, m_endPos);

        // move (alt) or resize ellipse
        if (e->modifiers() & Qt::AltModifier) {
            QPointF trans = convertToPixelCoord(e) - m_endPos;
            m_startPos += trans;
            m_endPos += trans;
        } else {
            QPointF diag = convertToPixelCoord(e) - (e->modifiers() & Qt::ControlModifier
                    ? m_centerPos : m_startPos);
            // circle?
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

void KisToolSelectElliptical::mouseReleaseEvent(KoPointerEvent *e)
{
     if (m_canvas && m_selecting && e->button() == Qt::LeftButton) {

        if (m_startPos == m_endPos) {
            clearSelection();
        } else {
//             QApplication::setOverrideCursor(KisCursor::waitCursor());

            if (!m_currentImage)
                return;

            if (m_currentImage && m_currentImage->activeDevice()) {
                KisPaintDeviceSP dev = m_currentImage->activeDevice();
                KisSelectionSP selection = dev->selection();
                KisSelectedTransaction *t = new KisSelectedTransaction(i18n("Elliptical Selection"), dev);

                bool hasSelection = dev->hasSelection();
                if (!hasSelection)
                {
                    selection->clear();
                    if(m_selectAction == SELECTION_SUBTRACT)
                        selection->invert();
                }

                KisPainter painter(selection);
                painter.setPaintColor(KoColor(Qt::black, selection->colorSpace()));
                painter.setFillStyle(KisPainter::FillStyleForegroundColor);
                painter.setStrokeStyle(KisPainter::StrokeStyleNone);
                painter.setAntiAliasPolygonFill(m_optWidget->antiAliasSelection());
                painter.setOpacity(OPACITY_OPAQUE);
                KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("paintbrush", 0, &painter);
                painter.setPaintOp(op); // And now the painter owns the op and will destroy it.

                switch(m_selectAction)
                {
                    case SELECTION_ADD:
                        painter.setCompositeOp(selection->colorSpace()->compositeOp(COMPOSITE_OVER));
                        break;
                    case SELECTION_SUBTRACT:
                        painter.setCompositeOp(selection->colorSpace()->compositeOp(COMPOSITE_SUBTRACT));
                        break;
                    default:
                        break;
                }

                painter.paintEllipse(QRectF(m_startPos, m_endPos), PRESSURE_DEFAULT/*e->pressure()*/,
                                     e->xTilt(), e->yTilt());

                if(hasSelection) {
                    QRect rect(painter.dirtyRegion().boundingRect());
                    dev->emitSelectionChanged(rect);
                } else {
                    dev->emitSelectionChanged();
                }

                m_canvas->addCommand(t);

//                 QApplication::restoreOverrideCursor();
            }
        }
        m_selecting = false;
    }
}

void KisToolSelectElliptical::slotSetAction(int action) {
    if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
        m_selectAction =(enumSelectionMode)action;
}

QWidget* KisToolSelectElliptical::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(m_canvas);
    Q_ASSERT(canvas);
    m_optWidget = new KisSelectionOptions(canvas);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setWindowTitle(i18n("Elliptical Selection"));

    connect (m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    if (l) {
        l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    }

    return m_optWidget;
}

QWidget* KisToolSelectElliptical::optionWidget()
{
        return m_optWidget;
}



#include "kis_tool_select_elliptical.moc"
