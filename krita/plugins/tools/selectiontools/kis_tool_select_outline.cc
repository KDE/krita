/*
 *  kis_tool_select_freehand.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <kis_tool_select_outline.h>

#include <QApplication>
#include <QPainter>
#include <QWidget>
#include <QLayout>
#include <QVBoxLayout>

#include <kdebug.h>
#include <klocale.h>

#include <KoPointerEvent.h>

#include <kis_layer.h>
#include <kis_selection_options.h>
#include <kis_cursor.h>
#include <kis_image.h>

#include "kis_selected_transaction.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_canvas2.h"
#include "kis_undo_adapter.h"

KisToolSelectOutline::KisToolSelectOutline(KoCanvasBase * canvas)
    : KisTool(canvas, KisCursor::load("tool_outline_selection_cursor.png", 5, 5))
{
    m_dragging = false;
    m_optWidget = 0;
    m_selectAction = SELECTION_ADD;
}

KisToolSelectOutline::~KisToolSelectOutline()
{
}

void KisToolSelectOutline::activate()
{
    super::activate();

    if (!m_optWidget)
        return;

    m_optWidget->slotActivated();
}


void KisToolSelectOutline::mousePressEvent(KoPointerEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_points.clear();
        m_points.append(convertToPixelCoord(event));
    }
}

void KisToolSelectOutline::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        m_points.append(convertToPixelCoord(event));
        updateFeedback();
    }
}

void KisToolSelectOutline::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!m_canvas)
        return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        m_dragging = false;
        deactivate();

        if (m_currentImage && m_currentImage->activeDevice()) {
            QApplication::setOverrideCursor(KisCursor::waitCursor());
            KisPaintDeviceSP dev = m_currentImage->activeDevice();
            bool hasSelection = dev->hasSelection();
            KisSelectedTransaction *t = 0;
            if (m_currentImage->undo()) t = new KisSelectedTransaction(i18n("Outline Selection"), dev);
            KisSelectionSP selection = dev->selection();

            if (!hasSelection) {
                selection->clear();
            }

            KisPainter painter(selection);

            painter.setPaintColor(KoColor(Qt::black, selection->colorSpace()));
            painter.setFillStyle(KisPainter::FillStyleForegroundColor);
            painter.setStrokeStyle(KisPainter::StrokeStyleNone);
            painter.setOpacity(OPACITY_OPAQUE);
            KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("paintbrush", 0, &painter);
            painter.setPaintOp(op);    // And now the painter owns the op and will destroy it.
            painter.setAntiAliasPolygonFill(m_optWidget->antiAliasSelection());

            switch (m_selectAction) {
            case SELECTION_ADD:
                painter.setCompositeOp(selection->colorSpace()->compositeOp(COMPOSITE_OVER));
                break;
            case SELECTION_SUBTRACT:
                painter.setCompositeOp(selection->colorSpace()->compositeOp(COMPOSITE_SUBTRACT));
                break;
            default:
                break;
            }

            painter.paintPolygon(m_points);

            if(hasSelection) {
                QRegion dirty(painter.dirtyRegion());
                dev->setDirty(dirty);
                dev->emitSelectionChanged(dirty.boundingRect());
            } else {
                dev->setDirty();
                dev->emitSelectionChanged();
            }

            if (m_currentImage->undo())
                m_currentImage->undoAdapter()->addCommand(t);

            QApplication::restoreOverrideCursor();
        }

        m_points.clear();
    }
}

#define FEEDBACK_LINE_WIDTH 1

void KisToolSelectOutline::paint(QPainter& gc, KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (m_dragging && m_points.count() > 1) {

        QPen pen(Qt::white, FEEDBACK_LINE_WIDTH, Qt::DotLine);
        gc.save();
        gc.setPen(pen);

        for (qint32 pointIndex = 0; pointIndex < m_points.count() - 1; pointIndex++) {
            QPointF startPos = pixelToView(m_points[pointIndex]);
            QPointF endPos = pixelToView(m_points[pointIndex + 1]);
            gc.drawLine(startPos, endPos);
        }

        gc.restore();
    }
}

void KisToolSelectOutline::updateFeedback()
{
    if (m_points.count() > 1) {
        qint32 lastPointIndex = m_points.count() - 1;

        QRectF updateRect = QRectF(m_points[lastPointIndex - 1], m_points[lastPointIndex]).normalized();
        updateRect.adjust(-FEEDBACK_LINE_WIDTH, -FEEDBACK_LINE_WIDTH, FEEDBACK_LINE_WIDTH, FEEDBACK_LINE_WIDTH);

        updateCanvasPixelRect(updateRect);
    }
}

void KisToolSelectOutline::deactivate()
{
    if (m_canvas) {
        updateCanvasPixelRect(image()->bounds());
    }
}

QWidget* KisToolSelectOutline::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(m_canvas);
    Q_ASSERT(canvas);
    m_optWidget = new KisSelectionOptions(canvas);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setWindowTitle(i18n("Outline Selection"));

    connect (m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    if (l) {
        l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    }

    return m_optWidget;
}

QWidget* KisToolSelectOutline::optionWidget()
{
        return m_optWidget;
}

void KisToolSelectOutline::slotSetAction(int action) {
    if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
        m_selectAction =(enumSelectionMode)action;
}

#include "kis_tool_select_outline.moc"

