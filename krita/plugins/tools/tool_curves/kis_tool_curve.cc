/*
 *  kis_tool_curve.cc -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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


#include <qpainter.h>
#include <qlayout.h>
#include <qrect.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>

#include "kis_global.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_point.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_paintop_registry.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_cursor.h"

#include "kis_curve_framework.h"
#include "kis_tool_curve.h"

QRect KisToolCurve::pivotRect (const QPoint& pos)
{
    return QRect (pos-QPoint(4,4),pos+QPoint(4,4));
}

QRect KisToolCurve::selectedPivotRect (const QPoint& pos)
{
    return QRect (pos-QPoint(5,5),pos+QPoint(5,5));
}

KisToolCurve::KisToolCurve(const QString& UIName)
    : super(UIName)
{
    m_curve = 0;
    m_currentImage = 0;
    
    m_dragging = false;

    m_drawPivots = true;
    m_drawingPen = QPen(Qt::white, 0, Qt::SolidLine);
    m_pivotPen = QPen(Qt::gray, 0, Qt::SolidLine);
    m_selectedPivotPen = QPen(Qt::yellow, 0, Qt::SolidLine);
    m_pivotRounding = m_selectedPivotRounding = 55;

    m_pressedKeys = 0x0000;
}

KisToolCurve::~KisToolCurve()
{
    if (m_curve)
        delete m_curve;
}

void KisToolCurve::update (KisCanvasSubject *subject)
{
    super::update (subject);
    if (m_subject)
        m_currentImage = m_subject->currentImg();
}

void KisToolCurve::deactivate()
{
    draw();
    if (m_curve)
        m_curve->clear();

    m_pressedKeys = 0x0000;
    m_dragging = false;
    m_drawPivots = true;
}

void KisToolCurve::buttonPress(KisButtonPressEvent *event)
{
    if (!m_currentImage)
        return;
    if (event->button() == Qt::LeftButton) {
        draw();
        m_dragging = true;
        m_curve->startAction(convertKeysToOptions(m_pressedKeys));
        m_current = selectByHandle (m_subject->canvasController()->windowToView(event->pos().toQPoint()));
        if (m_current == m_curve->end() && !(m_pressedKeys & Qt::ControlButton)) {
            m_previous = m_curve->find(m_curve->last());
            m_current = m_curve->pushPivot(event->pos());
            if (m_curve->pivots().count() > 1)
                m_curve->calculateCurve(m_previous,m_current,m_current);
        } else if (!(*m_current).isSelected())  // It means: if selectByHandle returned an unselected pivot (for KEEPSELOPTION)
            m_dragging = false;
        m_curve->endAction();
        draw();
    }
}

void KisToolCurve::keyPress(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        m_dragging = false;
        paintCurve();
        m_curve->clear();
    } else
    if (event->key() == Qt::Key_Escape) {
        m_dragging = false;
        draw();
        m_curve->clear();
    } else
    if (event->key() == Qt::Key_Delete) {
        draw();
        m_dragging = false;
        m_curve->deleteSelected();
        m_current = m_curve->find(m_curve->last());
        m_previous = m_curve->selectPivot(m_current);
        draw();
    }
    draw();
    if (event->key() == Qt::Key_Control)
        m_pressedKeys |= Qt::ControlButton;
    if (event->key() == Qt::Key_Shift)
        m_pressedKeys |= Qt::ShiftButton;
    if (event->key() == Qt::Key_Alt)
        m_pressedKeys |= Qt::AltButton;
    draw();
}

void KisToolCurve::keyRelease(QKeyEvent *event)
{
    draw();
    if (event->key() == Qt::Key_Control)
        m_pressedKeys &= !Qt::ControlButton;
    if (event->key() == Qt::Key_Shift)
        m_pressedKeys &= !Qt::ShiftButton;
    if (event->key() == Qt::Key_Alt)
        m_pressedKeys &= !Qt::AltButton;
    draw();
}

void KisToolCurve::buttonRelease(KisButtonReleaseEvent *)
{
    m_dragging = false;
}

void KisToolCurve::doubleClick(KisDoubleClickEvent *)
{
    paintCurve();
    m_curve->clear();
}

void KisToolCurve::move(KisMoveEvent *event)
{
    if (m_dragging) {
        draw();
        m_curve->startAction(convertKeysToOptions(m_pressedKeys));
        KisPoint trans = event->pos() - (*m_current).point();
        m_curve->moveSelected(trans);
        m_curve->endAction();
        draw();
    }
}

KisCurve::iterator KisToolCurve::selectByHandle(const QPoint& pos)
{
    KisCurve pivs = m_curve->pivots(), inHandle;
    KisCurve::iterator it;
    for (it = pivs.begin(); it != pivs.end(); it++) {
        if (pivotRect(m_subject->canvasController()->windowToView((*it).point().toQPoint())).contains(pos))
            inHandle.pushPoint((*it));
    }
    if (inHandle.isEmpty())
        return m_curve->end();
    return m_curve->selectPivot(inHandle.last());
}

int KisToolCurve::convertKeysToOptions(int keys)
{
    int options = NOOPTIONS;

    if (keys & Qt::ControlButton)
        options |= KEEPSELECTEDOPTION;

    return options;
}

void KisToolCurve::draw()
{
    KisCanvasPainter *gc;
    KisCanvasController *controller;
    KisCanvas *canvas;
    if (m_subject && m_currentImage) {
        controller = m_subject->canvasController();
        canvas = controller->kiscanvas();
        gc = new KisCanvasPainter(canvas);
    } else
        return;

    gc->setPen(m_drawingPen);
    gc->setRasterOp(Qt::XorROP);

    KisCurve::iterator it = m_curve->begin();
    while (it != m_curve->end()) {
        if ((*it).isPivot()) {
            it = drawPivot (*gc, it);
        } else
            it = drawPoint (*gc, it);
    }
        
    delete gc;
}

KisCurve::iterator KisToolCurve::drawPoint(KisCanvasPainter& gc, KisCurve::iterator point)
{
    KisCanvasController *controller = m_subject->canvasController();

    QPoint pos1, pos2;
    pos1 = controller->windowToView((*point).point().toQPoint());

    switch ((*point).hint()) {
    case POINTHINT:
        gc.drawPoint(pos1);
        point += 1;
        break;
    case LINEHINT:
        if (++point != m_curve->end() && (*point).hint() <= LINEHINT) {
            pos2 = controller->windowToView((*point).point().toQPoint());
            gc.drawLine(pos1,pos2);
        }
        break;
    default:
        point += 1;
    }

    return point;
}

KisCurve::iterator KisToolCurve::drawPivot(KisCanvasPainter& gc, KisCurve::iterator point)
{
    KisCanvasController *controller = m_subject->canvasController();

    if (m_drawPivots) {
        QPoint pos = controller->windowToView((*point).point().toQPoint());
        if ((*point).isSelected()) {
            gc.setPen(m_selectedPivotPen);
            gc.drawRoundRect(selectedPivotRect(pos),m_selectedPivotRounding,m_selectedPivotRounding);
        } else {
            gc.setPen(m_pivotPen);
            gc.drawRoundRect(pivotRect(pos),m_pivotRounding,m_pivotRounding);
        }
        gc.setPen(m_drawingPen);
    }

    point += 1;

    return point;
}

void KisToolCurve::paint(KisCanvasPainter&)
{
    draw();
}

void KisToolCurve::paint(KisCanvasPainter&, const QRect&)
{
    draw();
}

void KisToolCurve::paintCurve()
{
    KisPaintDeviceSP device = m_currentImage->activeDevice ();
    if (!device) return;
    
    KisPainter painter (device);
    if (m_currentImage->undo()) painter.beginTransaction (i18n ("Example of curve"));

    painter.setPaintColor(m_subject->fgColor());
    painter.setBrush(m_subject->currentBrush());
    painter.setOpacity(m_opacity);
    painter.setCompositeOp(m_compositeOp);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintop(), m_subject->currentPaintopSettings(), &painter);
    painter.setPaintOp(op); // Painter takes ownership

    KisCurve::iterator it = m_curve->begin();
    while (it != m_curve->end())
        it = paintPoint(painter,it);

    device->setDirty( painter.dirtyRect() );
    notifyModified();

    if (m_currentImage->undo()) {
        m_currentImage->undoAdapter()->addCommand(painter.endTransaction());
    }

    draw();
}

KisCurve::iterator KisToolCurve::paintPoint (KisPainter& painter, KisCurve::iterator point)
{
    KisCurve::iterator next = point; next+=1;
    switch ((*point).hint()) {
    case POINTHINT:
        painter.paintAt((*point++).point(), PRESSURE_DEFAULT, 0, 0);
        break;
    case LINEHINT:
        if (next != m_curve->end() && (*next).hint() <= LINEHINT)
            painter.paintLine((*point++).point(), PRESSURE_DEFAULT, 0, 0, (*next).point(), PRESSURE_DEFAULT, 0, 0);
        else
            painter.paintAt((*point++).point(), PRESSURE_DEFAULT, 0, 0);
        break;
    default:
        point += 1;
    }

    return point;
}

#include "kis_tool_curve.moc"
