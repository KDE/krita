/*
 *  kis_tool_example.cc -- part of Krita
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

#include "kis_curve_framework.h"

#include "kis_tool_example.h"


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
/* And obviously this is just to see if the Framework works, it hasn't no other senses :) */
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

    double xScale = 1;
    double yScale = 1;
    double spacing;
    // Scale x or y so that we effectively have a square brush
    // and calculate distance in that coordinate space. We reverse this scaling
    // before drawing the brush. This produces the correct spacing in both
    // x and y directions, even if the brush's aspect ratio is not 1:1.
    xScale = ySpacing / xSpacing;
    spacing = ySpacing;

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
        addPoint (it,p);
        spacing += 1;
        dist -= spacing;
    }
    return;
}

KisToolExample::KisToolExample()
    : super(i18n("Tool for Curves - Example"))
{
    setName("tool_example");
    setCursor(KisCursor::load("tool_example_cursor.png", 6, 6));

    m_dragging = false;
    m_editing = false;
    m_curve = new KisCurveExample;
}

KisToolExample::~KisToolExample()
{

}


void KisToolExample::update (KisCanvasSubject *subject)
{
    super::update (subject);
    if (m_subject)
        m_currentImage = m_subject->currentImg ();
}


void KisToolExample::deactivate()
{
    super::deactivate();
    m_dragging = false;
    m_editing = false;
}

void KisToolExample::buttonPress(KisButtonPressEvent *event)
{
    if (m_currentImage && event->button() == LeftButton) {
        draw();
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
            m_curve->pushPivot(m_end,false,0);
        } else {
            CurvePoint pos(mouseOnHandle(event->pos()),true);
            KisCurve sel = m_curve->selectedPivots();
            if (!sel.isEmpty())
                m_curve->selectPivot(sel[0],false);
            if (pos != KisPoint(-1,-1))
                m_curve->selectPivot(pos);
        }
        draw();
    }
}

void KisToolExample::keyPress(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete && !m_dragging) {
        draw();
        if (!m_editing) {
            if (m_curve->count() > 1) {
                m_curve->deleteLastPivot();
                m_start = m_end = m_curve->last().point();
            } else
                m_curve->clear();
        } else {
            KisCurve sel = m_curve->selectedPivots();
            if (!sel.isEmpty()) {
                m_curve->deletePivot(sel[0]);
                if (!m_curve->count())
                    m_editing = false;
            }
        }
        draw();
    }
}

void KisToolExample::move(KisMoveEvent *event)
{
    if (m_dragging) {
        draw();
        if (!m_editing) {
            if (m_curve->pivots().count() > 1)
                m_curve->deleteLastPivot();
            m_end = event->pos();
            m_curve->calculateCurve(m_start,m_end,m_curve->end());
            m_curve->pushPivot(m_end,false,0);
        } else {
            KisCurve sel = m_curve->selectedPivots();
            KisPoint dest = event->pos();
            if (!sel.isEmpty()) {
                KisCurve::iterator it = m_curve->find(sel[0]);
                it = m_curve->movePivot(it,dest);
                m_curve->selectPivot(it);
            }
        }
        draw();
    }
}

void KisToolExample::buttonRelease(KisButtonReleaseEvent *)
{
    if (!m_subject || !m_currentImage)
        return;

    m_dragging = false;
}

void KisToolExample::doubleClick(KisDoubleClickEvent *)
{
    if (!m_editing)
        m_editing = true;
    else {
        paintCurve();
        m_curve->clear();
        m_editing = false;
    }
}

void KisToolExample::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KShortcut(Qt::Key_F9));
        m_action = new KRadioAction(i18n("&Example"),
                                    "tool_example",
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

#include "kis_tool_example.moc"
