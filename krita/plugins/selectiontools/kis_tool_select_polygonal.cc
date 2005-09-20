/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
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
#include <qapplication.h>
#include <qpainter.h>
#include <qregion.h>
#include <qwidget.h>

#include <kaction.h>
#include <kdebug.h>
#include <kcommand.h>
#include <klocale.h>

#include <kis_selection_options.h>
#include <kis_canvas_controller.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_tool_select_polygonal.h>
#include <kis_vec.h>
#include <kis_undo_adapter.h>
#include <kis_button_press_event.h>
#include <kis_button_release_event.h>
#include <kis_move_event.h>
#include "kis_selected_transaction.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"

KisToolSelectPolygonal::KisToolSelectPolygonal()
    : super()
{
    setName("tool_select_polygonal");
    setCursor(KisCursor::selectCursor());

    m_subject = 0;
    m_dragging = false;
    m_optWidget = 0;
    m_selectAction = SELECTION_ADD;
}

KisToolSelectPolygonal::~KisToolSelectPolygonal()
{
}

void KisToolSelectPolygonal::activate()
{
    super::activate();

    if (!m_optWidget)
        return;

    m_optWidget -> slotActivated();
}

void KisToolSelectPolygonal::update (KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolSelectPolygonal::buttonPress(KisButtonPressEvent *event)
{
    if (event -> button() == LeftButton) {
        m_dragging = true;

        if (m_points.isEmpty())
        {
            m_dragStart = event -> pos();
            m_dragEnd = event -> pos();
            m_points.append(m_dragStart);
        } else {
            m_dragStart = m_dragEnd;
            m_dragEnd = event -> pos();
            draw();
        }
    } else if (event -> button() == RightButton) {
        // erase old lines on canvas
        draw();
        m_dragging = false;

        KisImageSP img = m_subject -> currentImg();

        if (img) {
            QApplication::setOverrideCursor(KisCursor::waitCursor());
            KisLayerSP layer = img -> activeLayer();
            bool hasSelection = layer -> hasSelection();

            //XXX: Fix string
            KisSelectedTransaction *t = new KisSelectedTransaction(i18n("Selection Polygons"), layer.data());
            KisSelectionSP selection = layer -> selection();

            if (!hasSelection)
            {
                selection -> clear();
            }

            KisPainter painter(selection.data());

            painter.setPaintColor(Qt::black);
            painter.setFillStyle(KisPainter::FillStyleForegroundColor);
            painter.setStrokeStyle(KisPainter::StrokeStyleNone);
            painter.setBrush(m_subject -> currentBrush());
            painter.setOpacity(OPACITY_OPAQUE);
            KisPaintOp * op = KisPaintOpRegistry::instance() -> paintOp("paintbrush", &painter);
            painter.setPaintOp(op); // And now the painter owns the op and will destroy it.

            switch(m_selectAction)
            {
                case SELECTION_ADD:
                    painter.setCompositeOp(COMPOSITE_OVER);
                    break;
                case SELECTION_SUBTRACT:
                    painter.setCompositeOp(COMPOSITE_SUBTRACT);
                    break;
                default:
                    break;
            }

            painter.paintPolygon(m_points);

            if(hasSelection)
                layer->emitSelectionChanged(painter.dirtyRect());
            else
                layer->emitSelectionChanged();

            if (img -> undoAdapter())
                img -> undoAdapter() -> addCommand(t);
                
            QApplication::restoreOverrideCursor();
        }

        m_points.clear();
    }
}

void KisToolSelectPolygonal::move(KisMoveEvent *event)
{
    if (m_dragging) {
        // erase old lines on canvas
        draw();
        // get current mouse position
        m_dragEnd = event -> pos();
        // draw new lines on canvas
        draw();
    }
}

void KisToolSelectPolygonal::buttonRelease(KisButtonReleaseEvent *event)
{
    if (!m_subject)
        return;

    if (m_dragging && event -> button() == LeftButton)  {
        m_dragging = false;
        m_points.append (m_dragEnd);
    }

    if (m_dragging && event -> button() == RightButton) {
        
        }
}

void KisToolSelectPolygonal::paint(QPainter& gc)
{
    draw(gc);
}

void KisToolSelectPolygonal::paint(QPainter& gc, const QRect&)
{
    draw(gc);
}

void KisToolSelectPolygonal::draw()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject -> canvasController();
        QWidget *canvas = controller -> canvas();
        QPainter gc(canvas);

        draw(gc);
    }
}

void KisToolSelectPolygonal::draw(QPainter& gc)
{
    if (!m_subject)
        return;

    QPen pen(Qt::white, 0, Qt::DotLine);

    gc.setPen(pen);
        gc.setRasterOp(Qt::XorROP);

    KisCanvasController *controller = m_subject -> canvasController();
    KisPoint start, end;
    QPoint startPos;
    QPoint endPos;

    if (m_dragging) {
        startPos = controller -> windowToView(m_dragStart.floorQPoint());
        endPos = controller -> windowToView(m_dragEnd.floorQPoint());
        gc.drawLine(startPos, endPos);
    } else {
        for (KisPointVector::iterator it = m_points.begin(); it != m_points.end(); ++it) {

            if (it == m_points.begin())
            {
                start = (*it);
            } else {
                end = (*it);
                
                startPos = controller -> windowToView(start.floorQPoint());
                endPos = controller -> windowToView(end.floorQPoint());
                
                gc.drawLine(startPos, endPos);

                start = end;
            }
        }
    }
}


void KisToolSelectPolygonal::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Polygonal Select"),
                        "tool_polygonal_selection" ,
                        0,
                        this,
                        SLOT(activate()),
                        collection,
                        name());
        Q_CHECK_PTR(m_action);
        m_action -> setExclusiveGroup("tools");
        m_action -> setToolTip(i18n("Select a polygonal area"));
        m_ownAction = true;
    }
}


QWidget* KisToolSelectPolygonal::createOptionWidget(QWidget* parent)
{
    m_optWidget = new KisSelectionOptions(parent, m_subject);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget -> setCaption(i18n("Selection Polygons"));

    connect (m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

    return m_optWidget;
}

QWidget* KisToolSelectPolygonal::optionWidget()
{
        return m_optWidget;
}

void KisToolSelectPolygonal::slotSetAction(int action) {
    if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
        m_selectAction =(enumSelectionMode)action;
}



#include "kis_tool_select_polygonal.moc"
