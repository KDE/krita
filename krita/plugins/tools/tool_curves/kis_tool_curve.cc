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

#include "kis_tool_curve.h"

#include <math.h>
#include <limits.h>

#include <QApplication>
#include <QPainter>
#include <QLayout>
#include <QRect>
#include <QPointF>

#include <kis_debug.h>
#include <klocale.h>

#include "kis_global.h"
//#include "kis_doc2.h"
#include "kis_painter.h"
#include "KoPointerEvent.h"
#include "kis_cursor.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"
#include "kis_paintop_registry.h"

#include "kis_curve_framework.h"


QRect KisToolCurve::pivotRect(const QPointF& pos)
{
    return QRect((pos - QPointF(4, 4)).toPoint(), (pos + QPointF(4, 4)).toPoint());
}

QRect KisToolCurve::selectedPivotRect(const QPointF& pos)
{
    return QRect((pos - QPointF(5, 5)).toPoint(), (pos + QPointF(5, 5)).toPoint());
}

KisToolCurve::KisToolCurve(KoCanvasBase* canvas, const QString& UIName)
        : KisToolPaint(canvas)
{
    m_UIName = UIName;
    m_currentImage = 0;
    m_optWidget = 0;

    m_curve = 0;

    m_dragging = false;
    m_draggingCursor = false;
    m_drawPivots = true;
    m_drawingPen = QPen(Qt::black, 0, Qt::SolidLine);
    m_pivotPen = QPen(Qt::gray, 0, Qt::SolidLine);
    m_selectedPivotPen = QPen(Qt::blue, 0, Qt::SolidLine);
    m_pivotRounding = m_selectedPivotRounding = 55;

    m_actionOptions = NOOPTIONS;
    m_supportMinimalDraw = true;
    m_selectAction = SELECTION_ADD;
}

KisToolCurve::~KisToolCurve()
{

}

void KisToolCurve::deactivate()
{
    if (m_curve) {
        m_curve->clear();
        m_curve->endActionOptions();
    }

    m_actionOptions = NOOPTIONS;
    m_dragging = false;
    m_drawPivots = true;
}

void KisToolCurve::mousePressEvent(KoPointerEvent *event)
{
    updateOptions(QApplication::keyboardModifiers());
    if (!m_currentImage)
        return;
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_currentPoint = event->pos().toPointF();
        PointPair temp = pointUnderMouse(m_subject->canvasController()->windowToView(event->pos()).toPointF());
        if (temp.first == m_curve->end() && !(m_actionOptions)) {
            m_curve->selectAll(false);
            if (!m_curve->isEmpty())
                m_previous = --(m_curve->end());
            m_current = m_curve->pushPivot(event->pos().toPointF());
            if (m_curve->pivots().count() > 1)
                m_curve->calculateCurve(m_previous, m_current, m_current);
            draw(m_current);
        } else {
            if (temp.second)
                m_current = m_curve->selectPivot(temp.first);
            else
                m_current = selectByMouse(temp.first);

            if (!(*m_current).isSelected())
                m_dragging = false;
            draw(true, true);
        }
    }
}

void KisToolCurve::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        m_dragging = false;
        commitCurve();
    } else if (event->key() == Qt::Key_Escape) {
        m_dragging = false;
        m_curve->clear();
    } else if (event->key() == Qt::Key_Delete) {
        m_dragging = false;
        m_curve->deleteSelected();
        m_current = m_curve->lastIterator();
        m_previous = m_curve->selectPivot(m_current);
        draw(false);
    }
}

void KisToolCurve::keyReleaseEvent(QKeyEvent *)
{

}

void KisToolCurve::mouseReleaseEvent(KoPointerEvent *event)
{
    updateOptions(QApplication::keyboardModifiers());
    m_dragging = false;
}

void KisToolCurve::mouseDoubleClick(KoPointerEvent *)
{
    commitCurve();
}

void KisToolCurve::mouseMoveEvent(KoPointerEvent *event)
{
    updateOptions(QApplication::keyboardModifiers());
    PointPair temp = pointUnderMouse(m_subject->canvasController()->windowToView(event->pos()).toPointF());
    if (temp.first == m_curve->end() && !m_dragging) {
        if (m_draggingCursor) {
            setCursor(KisCursor::load(m_cursor, 6, 6));
            m_draggingCursor = false;
        }
    } else {
        setCursor(KisCursor::load("tool_curve_dragging.png", 6, 6));
        m_draggingCursor = true;
    }
    if (m_dragging) {
        QPointF trans = event->pos().toPointF() - m_currentPoint;
        m_curve->moveSelected(trans);
        m_currentPoint = event->pos().toPointF();
        draw();
    }
}

double pointToSegmentDistance(const QPointF& pp, const QPointF& pl0, const QPointF& pl1)
{
    QPointF l0 = pl0, l1 = pl1, p = pp;
    double lineLength = sqrt((l1.x() - l0.x()) * (l1.x() - l0.x()) + (l1.y() - l0.y()) * (l1.y() - l0.y()));
    double distance = 0;
    KisVector2D v0(l0), v1(l1), v(p), seg(v0 - v1), dist0(v0 - p), dist1(v1 - p);

    if (seg.length() < dist0.length() ||
            seg.length() < dist1.length()) // the point doesn't perpendicolarly intersecate the segment (or it's too far from the segment)
        return (double)INT_MAX;

    if (lineLength > DBL_EPSILON) {
        distance = ((l0.y() - l1.y()) * p.x() + (l1.x() - l0.x()) * p.y() + l0.x() * l1.y() - l1.x() * l0.y()) / lineLength;
        distance = fabs(distance);
    }

    return distance;
}

PointPair KisToolCurve::pointUnderMouse(const QPointF& pos)
{
    KisCurve::iterator it, next;
    QPointF pos1, pos2;
    it = handleUnderMouse(pos);
    if (it != m_curve->end())
        return PointPair(it, true);

    for (it = m_curve->begin(); it != m_curve->end(); it++) {
        next = it.next();
        if (next == m_curve->end() || it == m_curve->end())
            return PointPair(m_curve->end(), false);
        if ((*it).hint() > LINEHINT || (*next).hint() > LINEHINT)
            continue;
        pos1 = m_subject->canvasController()->windowToView((*it).point()).toPointF();
        pos2 = m_subject->canvasController()->windowToView((*next).point()).toPointF();
        if (pos1 == pos2)
            continue;
        if (pointToSegmentDistance(pos, pos1, pos2) <= MAXDISTANCE)
            break;
    }

    return PointPair(it, false);
}

KisCurve::iterator KisToolCurve::handleUnderMouse(const QPointF& pos)
{
    KisCurve pivs = m_curve->pivots(), inHandle;
    KisCurve::iterator it;
    for (it = pivs.begin(); it != pivs.end(); it++) {
        if (pivotRect(m_subject->canvasController()->windowToView((*it).point()).toPointF()).contains(pos.toPoint()))
            inHandle.pushPoint((*it));
    }
    if (inHandle.isEmpty())
        return m_curve->end();
    return m_curve->find(inHandle.last());
}

KisCurve::iterator KisToolCurve::selectByMouse(KisCurve::iterator it)
{
    KisCurve::iterator prevPivot, nextPivot;

    if ((*it).isPivot())
        prevPivot = it;
    else
        prevPivot = it.previousPivot();
    nextPivot = it.nextPivot();

    m_curve->selectPivot(prevPivot);
    (*nextPivot).setSelected(true);

    return prevPivot;
}

int KisToolCurve::updateOptions(int key)
{
    int options = 0x0000;

    if (key & Qt::ControlModifier)
        options |= CONTROLOPTION;

    if (key & Qt::ShiftModifier)
        options |= SHIFTOPTION;

    if (key & Qt::AltModifier)
        options |= ALTOPTION;

    if (options != m_actionOptions) {
        m_actionOptions = options;
        m_curve->setActionOptions(m_actionOptions);
        draw(false);
    }

    return m_actionOptions;
}

void KisToolCurve::draw(bool m, bool o)
{
    draw(KisCurve::iterator(), o, m);
}

void KisToolCurve::draw(KisCurve::iterator inf, bool pivotonly, bool minimal)
{
    /*
        if (m_curve->isEmpty())
            return;
        QPainter *gc;
        KisCanvasController *controller;
        KisCanvas *canvas;
        if (m_subject && m_currentImage) {
            controller = m_subject->canvasController();
            canvas = controller->kiscanvas();
            gc = new QPainter(canvas->canvasWidget());
        } else
            return;

        gc->setPen(m_drawingPen);
    //    gc->setRasterOp(Qt::XorROP);

        KisCurve::iterator it, finish;

        if (minimal && m_supportMinimalDraw && 0) {
            if (pivotonly) {
                KisCurve p = m_curve->pivots();
                for (KisCurve::iterator i = p.begin(); i != p.end(); i++)
                    drawPivotHandle(*gc, i);
                delete gc;
                return;
            }
            if (inf.target() != 0) {
                if (inf != m_curve->end()) {
                    it = inf.previousPivot();
                    finish = inf.nextPivot();
                } else {
                    it = --m_curve->end();
                    finish = m_curve->end();
                }
            } else {
                KisCurve sel = m_curve->selectedPivots();
                if (sel.isEmpty()) {
                    delete gc;
                    return;
                }
                for (KisCurve::iterator i = sel.begin(); i != sel.end(); i++) {
                    it = m_curve->find(*i).previousPivot();
                    finish = m_curve->find(*i).nextPivot();
                    if ((*finish).isSelected())
                        finish = finish.previousPivot();
                    while (it != finish) {
                        if ((*it).isPivot())
                            drawPivotHandle(*gc, it);
                        it = drawPoint(*gc, it);
                    }
                }
                delete gc;
                return;
            }
        } else {
            it = m_curve->begin();
            finish = m_curve->end();
        }
        while (it != finish) {
            if ((*it).isPivot())
                drawPivotHandle(*gc, it);
            it = drawPoint(*gc, it);
        }

        delete gc;
    */
}

KisCurve::iterator KisToolCurve::drawPoint(QPainter& gc, KisCurve::iterator point)
{
    /*
        KisCanvasController *controller = m_subject->canvasController();

        QPointF pos1, pos2;
        pos1 = controller->windowToView((*point).point()).toPointF();

        switch ((*point).hint()) {
        case POINTHINT:
            gc.drawPoint(pos1);
            point += 1;
            break;
        case LINEHINT:
            gc.drawPoint(pos1);
            if (++point != m_curve->end() && (*point).hint() <= LINEHINT) {
                pos2 = controller->windowToView((*point).point()).toPointF();
                gc.drawLine(pos1, pos2);
            }
            break;
        default:
            point += 1;
        }

        return point;
    */
}

void KisToolCurve::drawPivotHandle(QPainter& gc, KisCurve::iterator point)
{
    /*
        KisCanvasController *controller = m_subject->canvasController();

        if (m_drawPivots) {
            QPointF pos = controller->windowToView((*point).point()).toPointF();
            if ((*point).isSelected()) {
                gc.setPen(m_selectedPivotPen);
                gc.drawRoundRect(selectedPivotRect(pos), m_selectedPivotRounding, m_selectedPivotRounding);
            } else {
                gc.setPen(m_pivotPen);
                gc.drawRoundRect(pivotRect(pos), m_pivotRounding, m_pivotRounding);
            }
            gc.setPen(m_drawingPen);
        }
    */
}

void KisToolCurve::paint(QPainter&)
{
    draw(false);
}

void KisToolCurve::paint(QPainter&, const QRect&)
{
    draw(false);
}

void KisToolCurve::commitCurve()
{
    if (toolType() == TOOL_SHAPE || toolType() == TOOL_FREEHAND)
        paintCurve();
    else if (toolType() == TOOL_SELECT)
        selectCurve();
    else
        dbgKrita << "NO SUPPORT FOR THIS TYPE OF TOOL";

    m_curve->clear();
    m_curve->endActionOptions();
}

void KisToolCurve::paintCurve()
{
    if (!currentNode())
        return;

    KisPaintDeviceSP device = currentNode()->paintDevice();
    if (!device) return;

    KisPainter painter(device);
    if (m_currentImage->undo()) painter.beginTransaction(m_transactionMessage);
    painter.setBounds(m_currentImage->bounds());
    painter.setPaintColor(m_subject->fgColor());
    painter.setBrush(m_subject->currentBrush());
    painter.setOpacity(m_opacity);
    painter.setCompositeOp(m_compositeOp);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintOp(), m_subject->currentPaintOpSettings(), &painter);
    painter.setPaintOp(op); // Painter takes ownership

// Call paintPoint
    KisCurve::iterator it = m_curve->begin();
    while (it != m_curve->end())
        it = paintPoint(painter, it);
// Finish

    device->setDirty(painter.dirtyRegion());
    notifyModified();

    if (m_currentImage->undo()) {
        m_currentImage->undoAdapter()->addCommandOld(painter.endTransaction());
    }

    draw(false);
}

KisCurve::iterator KisToolCurve::paintPoint(KisPainter& painter, KisCurve::iterator point)
{
    KisCurve::iterator next = point; next += 1;
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

QVector<QPointF> KisToolCurve::convertCurve()
{
    QVector<QPointF> points;

    for (KisCurve::iterator i = m_curve->begin(); i != m_curve->end(); i++)
        if ((*i).hint() != NOHINTS)
            points.append((*i).point());

    return points;
}

void KisToolCurve::selectCurve()
{
    QApplication::setOverrideCursor(KisCursor::waitCursor());
    KisPaintDeviceSP dev = currentNode()->paintDevice();
    bool hasSelection = dev->hasSelection();
    KisSelectedTransaction *t = 0;
    if (m_currentImage->undo() && hasSelection) t = new KisSelectedTransaction(m_transactionMessage, dev);
    KisSelectionSP selection = dev->selection();

    if (!hasSelection) {
        selection->clear();
    }

    KisPainter painter(selection.data());

    painter.setPaintColor(KoColor(Qt::black, selection->colorSpace()));
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);
    painter.setBrush(m_subject->currentBrush());
    painter.setOpacity(OPACITY_OPAQUE);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("paintbrush", 0, &painter);
    painter.setPaintOp(op);    // And now the painter owns the op and will destroy it.

    switch (m_selectAction) {
    case SELECTION_ADD:
        painter.setCompositeOp(dev->colorSpace()->compositeOp(COMPOSITE_OVER));
        break;
    case SELECTION_SUBTRACT:
        painter.setCompositeOp(dev->colorSpace()->compositeOp(COMPOSITE_SUBTRACT));
        break;
    default:
        break;
    }

    painter.paintPolygon(convertCurve());


    if (hasSelection) {
        QRect dirty(painter.dirtyRegion());
        dev->setDirty(dirty);
    } else {
        dev->setDirty();
    }
    /*
        if (m_currentImage->undo())
            m_currentImage->undoAdapter()->addCommandOld(t);
    */
    QApplication::restoreOverrideCursor();

    draw(false);
}

QWidget* KisToolCurve::createOptionWidget()
{
    /*
        if (toolType() == TOOL_SHAPE || toolType() == TOOL_FREEHAND)
            return KisToolPaint::createOptionWidget(parent);
        else if (toolType() == TOOL_SELECT)
            return createSelectionOptionWidget(parent);
        else
            dbgKrita << "NO SUPPORT FOR THIS TOOL TYPE";
    */
    return 0;
}

void KisToolCurve::slotSetAction(int action)
{
    if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
        m_selectAction = (selectionAction)action;
}

QWidget* KisToolCurve::createSelectionOptionWidget(QWidget* parent)
{
    m_optWidget = new KisSelectionOptions(parent, m_subject);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setCaption(m_UIName);
    m_optWidget->setObjectName(toolId() + "option widget");
    connect(m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

    QVBoxLayout * l = static_cast<QVBoxLayout*>(m_optWidget->layout());
    l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    m_optWidget->setFixedHeight(m_optWidget->sizeHint().height());
    return m_optWidget;
}

QWidget* KisToolCurve::optionWidget()
{
    /*
        if (toolType() == TOOL_SELECT)
            return m_optWidget;
        else
            return KisToolPaint::optionWidget();
    */
}

#include "kis_tool_curve.moc"
