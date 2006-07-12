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
#include "kis_vec.h"

#include "kis_curves_framework.h"

#include "kis_tool_curves.h"
#include "wdg_tool_curves.h"


class KisCurveExample : public KisCurve {

    typedef KisCurve super;
    
public:

    KisCurveExample() : super() {}

    ~KisCurveExample() {}

    virtual void calculateCurve(const KisPoint&, const KisPoint&, KisCurve::iterator);
    virtual void calculateCurve(const CurvePoint&, const CurvePoint&, KisCurve::iterator);
    virtual void calculateCurve(KisCurve::iterator, KisCurve::iterator, KisCurve::iterator);

};

void KisCurveExample::calculateCurve(KisCurve::iterator pos1, KisCurve::iterator pos2, KisCurve::iterator it)
{
    calculateCurve((*pos1).point(),(*pos2).point(), it);
}

void KisCurveExample::calculateCurve(const CurvePoint& pos1, const CurvePoint& pos2, KisCurve::iterator it)
{
    calculateCurve(pos1.point(),pos2.point(), it);
}

/* Brutally taken from KisPainter::paintLine, sorry :) */
/* And obviously this is just to see if the Framework works :) */
void KisCurveExample::calculateCurve(const KisPoint& pos1, const KisPoint& pos2, KisCurve::iterator it)
{
    double savedDist = 0;
    KisVector2D end(pos2);
    KisVector2D start(pos1);

    KisVector2D dragVec = end - start;
    KisVector2D movement = dragVec;

    // XXX: The spacing should vary as the pressure changes along the line.
    // This is a quick simplification.
    double xSpacing = 1;
    double ySpacing = 1;

    if (xSpacing < 0.5) {
        xSpacing = 0.5;
    }
    if (ySpacing < 0.5) {
        ySpacing = 0.5;
    }

    double xScale = 1;
    double yScale = 1;
    double spacing;
    // Scale x or y so that we effectively have a square brush
    // and calculate distance in that coordinate space. We reverse this scaling
    // before drawing the brush. This produces the correct spacing in both
    // x and y directions, even if the brush's aspect ratio is not 1:1.
    if (xSpacing > ySpacing) {
        yScale = xSpacing / ySpacing;
        spacing = xSpacing;
    }
    else {
        xScale = ySpacing / xSpacing;
        spacing = ySpacing;
    }

    dragVec.setX(dragVec.x() * xScale);
    dragVec.setY(dragVec.y() * yScale);

    double newDist = dragVec.length();
    double dist = savedDist + newDist;
    double l_savedDist = savedDist;

    if (dist < spacing)
        return;

    dragVec.normalize();
    KisVector2D step(0, 0);

    while (dist >= spacing) {
        if (l_savedDist > 0) {
            step += dragVec * (spacing - l_savedDist);
            l_savedDist -= spacing;
        }
        else {
            step += dragVec * spacing;
        }

        KisPoint p(start.x() + (step.x() / xScale), start.y() + (step.y() / yScale));

        double distanceMoved = step.length();
        double t = 0;

        if (newDist > DBL_EPSILON) {
            t = distanceMoved / newDist;
        }
        addPoint (it,p,false,false);
        dist -= spacing;
    }
    return;
}

KisToolCurves::KisToolCurves()
    : super(i18n("Tool for Curves - Example")),
      m_currentImage (0)
{
    setName("tool_curves");
    setCursor(KisCursor::load("tool_curves_cursor.png", 6, 6));

    m_dragging = false;
    m_editing = false;
    m_curve = new KisCurveExample;
}

KisToolCurves::~KisToolCurves()
{
    delete m_curve;
}

void KisToolCurves::update (KisCanvasSubject *subject)
{
    super::update (subject);
    if (m_subject)
        m_currentImage = m_subject->currentImg ();
}

void KisToolCurves::deactivate()
{
    m_curve->clear();
    m_dragging = false;
    m_editing = false;
}

void KisToolCurves::buttonPress(KisButtonPressEvent *event)
{
    if (m_currentImage && event->button() == LeftButton) {
        m_dragging = true;
        if (!m_editing) {
            if (m_curve->isEmpty()) {
                m_end = event->pos();
                m_start = event->pos();
            } else if (m_start != event->pos()) {
                m_start = m_end;
                m_end = event->pos();
            }
            m_curve->calculateCurve(m_start,m_end,m_curve->end());
            m_curve->pushPivot(m_end);
        } else {
            CurvePoint pos(mouseOnHandle(event->pos()),true);
            KisCurve sel = m_curve->selectedPivots();
            if (!sel.isEmpty())
                m_curve->setPivotSelected(sel[0],false);
            if (pos != KisPoint(-1,-1))
                m_curve->setPivotSelected(pos);
        }
        predraw();
    }
}

void KisToolCurves::keyPress(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete && !m_dragging) {
        if (!m_editing) {
            if (m_curve->count() > 1) {
                m_curve->deleteLastPivot();
                m_start = m_end = m_curve->last().point();
            } else // delete the line
                m_curve->clear();
        } else {
            KisCurve sel = m_curve->selectedPivots();
            if (!sel.isEmpty()) {
                m_curve->deletePivot(sel[0]);
                if (!m_curve->count())
                    m_editing = false;
            }
        }
        predraw();
    }
}

void KisToolCurves::move(KisMoveEvent *event)
{
    if (m_dragging) {
        if (!m_editing) {
            if (m_curve->pivots().count() > 1)
                m_curve->deleteLastPivot();
            m_end = event->pos();
            m_curve->calculateCurve(m_start,m_end,m_curve->end());
            m_curve->pushPivot(m_end);
        } else {
            KisCurve sel = m_curve->selectedPivots();
            KisPoint dest = event->pos();
            if (!sel.isEmpty()) {
                KisCurve::iterator newPivot = m_curve->movePivot(sel[0],dest);
                m_curve->setPivotSelected(newPivot); 
            }
        }
        predraw();
    }
}

KisPoint KisToolCurves::mouseOnHandle(KisPoint pos)
{
/* ( toQRect ( endx - m_handleSize / 2.0, starty - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) ) */
    KisCurve pivots = m_curve->pivots();

    for (KisCurve::iterator it = pivots.begin(); it != pivots.end(); it++) {
        if (QRect((*it).point().toQPoint()-QPoint(4,4),
                  (*it).point().toQPoint()+QPoint(4,4)).contains(pos.toQPoint()))
            return (*it).point();
    }

    return KisPoint(-1.0,-1.0);
}

void KisToolCurves::predraw()
{
    KisCanvasPainter *gc;
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        gc = new KisCanvasPainter(canvas);
    } else
        return;

    if (!m_subject || !m_currentImage)
        return;

    QPen pen = QPen(Qt::white, 0, Qt::SolidLine);

    gc->setPen(pen);
    gc->setRasterOp(Qt::XorROP);

    KisCanvasController *controller = m_subject->canvasController();
    KisPoint start, end;
    QPoint pos1, pos2;
    KisCurve::iterator it;

    controller->kiscanvas()->repaint();
    for (it = m_curve->begin(); it != (m_curve->end()); it++) {
        pos1 = controller->windowToView((*it).point().toQPoint());
//      pos2 = controller->windowToView((*it.nextPivot()).point().toQPoint());
        if ((*it).isPivot()) {
            if ((*it).isSelected())
                gc->fillRect(pos1.x()-4,pos1.y()-4,9,9,Qt::white);
            else
                gc->fillRect(pos1.x()-4,pos1.y()-4,9,9,Qt::gray);
        } else
//          gc->drawLine(pos1,pos2);
            gc->drawPoint(pos1);
    }
    
    delete gc;
}

void KisToolCurves::buttonRelease(KisButtonReleaseEvent *)
{
    if (!m_subject || !m_currentImage)
        return;

    m_dragging = false;

/*
    if (m_editing) {
        KisCurve sel = m_curve->selectedPivots();
        if (!sel.isEmpty()) {
            m_curve->setPivotSelected(sel[0],false);
            predraw();
        }
    }
*/
}

void KisToolCurves::doubleClick(KisDoubleClickEvent *)
{
    if (!m_editing)
        m_editing = true;
    else {
        draw();
        m_curve->clear();
        m_editing = false;
    }
}
/*
void KisToolCurves::paint(KisCanvasPainter&)
{
    predraw();
}

void KisToolCurves::paint(KisCanvasPainter&, const QRect&)
{
    predraw();
}
*/
void KisToolCurves::draw()
{
    m_dragging = false;

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

    for( KisCurve::iterator it = m_curve->begin(); it != (m_curve->end()--); it = it.nextPivot() )
        painter.paintLine((*it).point(), PRESSURE_DEFAULT, 0, 0, (*it.nextPivot()).point(), PRESSURE_DEFAULT, 0, 0);
//        painter.paintAt((*it).point(), PRESSURE_DEFAULT, 0, 0);

    device->setDirty( painter.dirtyRect() );
    notifyModified();

    if (m_currentImage->undo()) {
        m_currentImage->undoAdapter()->addCommand(painter.endTransaction());
    }

    m_subject->canvasController()->kiscanvas()->repaint();
}

void KisToolCurves::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KShortcut(Qt::Key_F9));
        m_action = new KRadioAction(i18n("&Curves"),
                                    "tool_curves",
                                    shortcut,
                                    this,
                                    SLOT(activate()),
                                    collection,
                                    name());
        Q_CHECK_PTR(m_action);

        m_action->setToolTip(i18n("Draw curves, like polyline, use Delete to remove lines, double-click to edit, and again to finish."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_curves.moc"
