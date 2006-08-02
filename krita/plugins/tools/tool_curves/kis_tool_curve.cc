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

#include <qapplication.h>
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
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"
#include "kis_paintop_registry.h"

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
    m_UIName = UIName;
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
    super::update(subject);
    if (m_subject)
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
        commitCurve();
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
    commitCurve();
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

void KisToolCurve::commitCurve()
{
    if (toolType() == TOOL_SHAPE || toolType() == TOOL_FREEHAND)
        paintCurve();
    else if (toolType() == TOOL_SELECT)
        selectCurve();
    else
        kdDebug(0) << "NO SUPPORT FOR THIS TOOL TYPE" << endl;

    m_curve->clear();
    m_curve->endActionOptions();
}

void KisToolCurve::paintCurve()
{
    KisPaintDeviceSP device = m_currentImage->activeDevice ();
    if (!device) return;
    
    KisPainter painter (device);
    if (m_currentImage->undo()) painter.beginTransaction (i18n (m_transactionMessage.latin1()));

    painter.setPaintColor(m_subject->fgColor());
    painter.setBrush(m_subject->currentBrush());
    painter.setOpacity(m_opacity);
    painter.setCompositeOp(m_compositeOp);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintop(), m_subject->currentPaintopSettings(), &painter);
    painter.setPaintOp(op); // Painter takes ownership

// Call paintPoint
    KisCurve::iterator it = m_curve->begin();
    while (it != m_curve->end())
        it = paintPoint(painter,it);
// Finish

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

QValueVector<KisPoint> KisToolCurve::convertCurve()
{
    QValueVector<KisPoint> points;

    for (KisCurve::iterator i = m_curve->begin(); i != m_curve->end(); i++)
        points.append((*i).point());

    return points;
}

void KisToolCurve::selectCurve()
{
    QApplication::setOverrideCursor(KisCursor::waitCursor());
    KisPaintDeviceSP dev = m_currentImage->activeDevice();
    bool hasSelection = dev->hasSelection();
    KisSelectedTransaction *t = 0;
    if (m_currentImage->undo()) t = new KisSelectedTransaction(i18n(m_transactionMessage.latin1()), dev);
    KisSelectionSP selection = dev->selection();

    if (!hasSelection) {
        selection->clear();
    }

    KisPainter painter(selection.data());

    painter.setPaintColor(KisColor(Qt::black, selection->colorSpace()));
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);
    painter.setBrush(m_subject->currentBrush());
    painter.setOpacity(OPACITY_OPAQUE);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("paintbrush", 0, &painter);
    painter.setPaintOp(op);    // And now the painter owns the op and will destroy it.

    switch (m_selectAction) {
    case SELECTION_ADD:
        painter.setCompositeOp(COMPOSITE_OVER);
        break;
    case SELECTION_SUBTRACT:
        painter.setCompositeOp(COMPOSITE_SUBTRACT);
        break;
    default:
        break;
    }

    painter.paintPolygon(convertCurve());


    if(hasSelection) {
        QRect dirty(painter.dirtyRect());
        dev->setDirty(dirty);
        dev->emitSelectionChanged(dirty);
    } else {
        dev->setDirty();
        dev->emitSelectionChanged();
    }

    if (m_currentImage->undo())
        m_currentImage->undoAdapter()->addCommand(t);

    QApplication::restoreOverrideCursor();

    draw();
}

QWidget* KisToolCurve::createOptionWidget(QWidget* parent)
{
    if (toolType() == TOOL_SHAPE || toolType() == TOOL_FREEHAND)
        return super::createOptionWidget(parent);
    else if (toolType() == TOOL_SELECT)
        return createSelectionOptionWidget(parent);
    else
        kdDebug(0) << "NO SUPPORT FOR THIS TOOL TYPE" << endl;
    return 0;
}

void KisToolCurve::slotSetAction(int action) {
    if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
        m_selectAction =(enumSelectionMode)action;
}

QWidget* KisToolCurve::createSelectionOptionWidget(QWidget* parent)
{
    m_optWidget = new KisSelectionOptions(parent, m_subject);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setCaption(m_UIName.latin1());

    connect (m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

    return m_optWidget;
}

QWidget* KisToolCurve::optionWidget()
{
    if (toolType() == TOOL_SELECT)
        return m_optWidget;
    else
        return super::optionWidget();
}

#include "kis_tool_curve.moc"
