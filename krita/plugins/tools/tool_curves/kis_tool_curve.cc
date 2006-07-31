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

#include <math.h>

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
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_cursor.h"
#include "kis_tool_controller.h"
#include "kis_vec.h"

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
    m_subject = 0;
    m_currentImage = 0;

    m_curve = 0;
    
    m_dragging = false;
    m_drawPivots = true;
    m_drawingPen = QPen(Qt::white, 0, Qt::SolidLine);
    m_pivotPen = QPen(Qt::gray, 0, Qt::SolidLine);
    m_selectedPivotPen = QPen(Qt::yellow, 0, Qt::SolidLine);
    m_pivotRounding = m_selectedPivotRounding = 55;

    m_actionOptions = NOOPTIONS;
}

KisToolCurve::~KisToolCurve()
{
    if (m_curve)
        delete m_curve;
}

void KisToolCurve::update (KisCanvasSubject *subject)
{
    m_subject = subject;
    m_currentImage = m_subject->currentImg();
}

void KisToolCurve::deactivate()
{
    draw();
    if (m_curve) {
        m_curve->clear();
        m_curve->endActionOptions();
    }

    m_actionOptions = NOOPTIONS;
    m_dragging = false;
    m_drawPivots = true;
}

void KisToolCurve::buttonPress(KisButtonPressEvent *event)
{
    updateOptions(event->state());
    if (!m_currentImage)
        return;
    if (event->button() == Qt::LeftButton) {
        draw();
        m_dragging = true;
        m_currentPoint = event->pos();
        m_current = selectByMouse (m_subject->canvasController()->windowToView(event->pos().toQPoint()));
        if (m_current == m_curve->end() && !(m_actionOptions)) {
            m_previous = m_curve->find(m_curve->last());
            m_current = m_curve->pushPivot(event->pos());
            if (m_curve->pivots().count() > 1)
                m_curve->calculateCurve(m_previous,m_current,m_current);
        } else if (!(*m_current).isSelected())
            m_dragging = false;
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
}

void KisToolCurve::keyRelease(QKeyEvent *)
{

}

void KisToolCurve::buttonRelease(KisButtonReleaseEvent *event)
{
    updateOptions(event->state());
    m_dragging = false;
}

void KisToolCurve::doubleClick(KisDoubleClickEvent *)
{
    paintCurve();
    m_curve->clear();
    m_curve->endActionOptions();
}

void KisToolCurve::move(KisMoveEvent *event)
{
    updateOptions(event->state());
    if (m_dragging) {
        draw();
        KisPoint trans = event->pos() - m_currentPoint;
        m_curve->moveSelected(trans);
        m_currentPoint = event->pos();
        draw();
    }
}

double pointToSegmentDistance(const KisPoint& p, const KisPoint& l0, const KisPoint& l1)
{
    double lineLength = sqrt((l1.x() - l0.x()) * (l1.x() - l0.x()) + (l1.y() - l0.y()) * (l1.y() - l0.y()));
    double distance = 0;
    KisVector2D v0(l0), v1(l1), v(p), seg(v0-v1), dist(v0-p);

    if (seg.length() < dist.length()) // the point doesn't perpendicolarly intersecate the segment (or it's too far from the segment)
        return 1000.0; // Return a too big integer - FIXME is there a MAX_DOUBLE_VALUE or something like that?

    if (lineLength > DBL_EPSILON) {
        distance = ((l0.y() - l1.y()) * p.x() + (l1.x() - l0.x()) * p.y() + l0.x() * l1.y() - l1.x() * l0.y()) / lineLength;
        distance = fabs(distance);
    }

    return distance;
}

KisCurve::iterator KisToolCurve::selectByMouse(const QPoint& pos)
{
    KisCurve::iterator it, next, prevPivot, nextPivot;
    QPoint pos1, pos2;
    it = selectByHandle(pos);
    if (it != m_curve->end())
        return it;

    for (it = m_curve->begin(); it != m_curve->end(); it++) {
        next = it.next();
        if (next == m_curve->end() || it == m_curve->end())
            return m_curve->end();
        if ((*it).hint() > LINEHINT || (*next).hint() > LINEHINT)
            continue;
        pos1 = m_subject->canvasController()->windowToView((*it).point().toQPoint());
        pos2 = m_subject->canvasController()->windowToView((*next).point().toQPoint());
        if (pos1 == pos2)
            continue;
        if (pointToSegmentDistance(pos,pos1,pos2) <= MAXDISTANCE)
            break;
    }

    if ((*it).isPivot())
        prevPivot = it;
    else
        prevPivot = it.previousPivot();
    nextPivot = it.nextPivot();

    m_curve->selectPivot(prevPivot);
    (*nextPivot).setSelected(true);

    return prevPivot;
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

int KisToolCurve::updateOptions(int key)
{
    int options = 0x0000;
    
    if (key & Qt::ControlButton)
            options |= CONTROLOPTION;

    if (key & Qt::ShiftButton)
            options |= SHIFTOPTION;

    if (key & Qt::AltButton)
            options |= ALTOPTION;

    if (options != m_actionOptions) {
        draw();
        m_actionOptions = options;
        m_curve->setActionOptions(m_actionOptions);
        draw();
    }

    return m_actionOptions;
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
        if ((*it).isPivot())
            drawPivotHandle (*gc, it);
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

void KisToolCurve::drawPivotHandle(KisCanvasPainter& gc, KisCurve::iterator point)
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
}

void KisToolCurve::paint(KisCanvasPainter&)
{
    draw();
}

void KisToolCurve::paint(KisCanvasPainter&, const QRect&)
{
    draw();
}

QCursor KisToolCurve::cursor()
{
    return m_cursor;
}

void KisToolCurve::setCursor(const QCursor& cursor)
{
    m_cursor = cursor;

    if (m_subject) {
        KisToolControllerInterface *controller = m_subject->toolController();

        if (controller && controller->currentTool() == this) {
            m_subject->canvasController()->setCanvasCursor(m_cursor);
        }
    }
}

void KisToolCurve::activate()
{
    if (m_subject) {
        KisToolControllerInterface *controller = m_subject->toolController();

        if (controller)
            controller->setCurrentTool(this);
    }
}

#include "kis_tool_curve.moc"
