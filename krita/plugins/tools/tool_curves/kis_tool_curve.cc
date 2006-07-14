/*
 *  kis_tool_star.cc -- part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

/* Just an initial commit. Emanuele Tamponi */


#include <qpainter.h>
#include <qlayout.h>
#include <qrect.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

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

KisToolCurve::KisToolCurve(const QString& UIName)
    : super(UIName)
{
    m_curve = 0;
    m_currentImage = 0;
    m_drawPivots = true;
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
    if (m_curve)
        m_curve->clear();
    m_drawPivots = true;
}

void KisToolCurve::buttonPress(KisButtonPressEvent *)
{

}

void KisToolCurve::buttonRelease(KisButtonReleaseEvent *)
{

}

void KisToolCurve::doubleClick(KisDoubleClickEvent *)
{

}

void KisToolCurve::move(KisMoveEvent *)
{

}

KisPoint KisToolCurve::mouseOnHandle(KisPoint pos)
{
    KisCurve pivots = m_curve->pivots();

    for (KisCurve::iterator it = pivots.begin(); it != pivots.end(); it++) {
        if (QRect((*it).point().toQPoint()-QPoint(4,4),
                  (*it).point().toQPoint()+QPoint(4,4)).contains(pos.toQPoint()))
            return (*it).point();
    }

    return KisPoint(-1.0,-1.0);
}

void KisToolCurve::draw()
{
    draw (*m_curve);
}

void KisToolCurve::draw(const KisCurve& curve)
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

    QPen pen = QPen(Qt::white, 0, Qt::SolidLine);
    gc->setPen(pen);
    gc->setRasterOp(Qt::XorROP);

    KisCurve::iterator it = curve.begin();
    while (it != curve.end()) // increasing is done by drawPoint
        it = drawPoint(*gc, it, curve);
    
    delete gc;
}

KisCurve::iterator KisToolCurve::drawPoint(KisCanvasPainter& gc, KisCurve::iterator point, const KisCurve& curve)
{
    KisCanvasController *controller = m_subject->canvasController();

    QPoint pos1, pos2;
    QRect pivotRect;

    pos1 = controller->windowToView((*point).point().toQPoint());

    if ((*point).isPivot() && m_drawPivots) {
        pivotRect = QRect(pos1-QPoint(4,4),
                          pos1+QPoint(4,4));
        if ((*point).isSelected())
            gc.fillRect(pivotRect,Qt::white);
        else
            gc.fillRect(pivotRect,Qt::gray);
    } else
        gc.drawPoint(pos1);
    
    switch ((*point).hint()) {
    case POINTHINT:
        point+=1;
        break;
    case LINEHINT:
        if (++point != curve.end()) {
            pos2 = controller->windowToView((*point).point().toQPoint());
            gc.drawLine(pos1,pos2);
        }
        break;
    }

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
    switch ((*point).hint()) {
    case POINTHINT:
        painter.paintAt((*point++).point(), PRESSURE_DEFAULT, 0, 0);
        break;
    case LINEHINT:
        KisCurve::iterator next = point; next+=1;
        if (next != m_curve->end())
            painter.paintLine((*point++).point(), PRESSURE_DEFAULT, 0, 0, (*next).point(), PRESSURE_DEFAULT, 0, 0);
        else
            painter.paintAt((*point++).point(), PRESSURE_DEFAULT, 0, 0);
        break;
    }

    return point;
}

#include "kis_tool_curve.moc"
