/*
 *  kis_tool_bezier.cc -- part of Krita
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
#include "kis_tool_bezier.h"

const int BEZIERENDHINT = 0x0010;
const int BEZIERPREVCONTROLHINT = 0x0020;
const int BEZIERNEXTCONTROLHINT = 0x0040;

const int SYMMETRICALCONTROLSOPTION = 0x0020;

class KisCurveBezier : public KisCurve {

    typedef KisCurve super;

    KisPoint midpoint (const KisPoint&, const KisPoint&);
    void recursiveCurve (const KisPoint&, const KisPoint&, const KisPoint&, const KisPoint&, int, iterator);

    int m_maxLevel;
    
public:

    KisCurveBezier() : super() {m_maxLevel = 6;}

    ~KisCurveBezier() {}

    virtual iterator pushPivot(const KisPoint&);
    virtual void calculateCurve(iterator, iterator, iterator);
    virtual iterator movePivot(iterator, const KisPoint&);
    virtual bool deletePivot(iterator);

};

KisPoint KisCurveBezier::midpoint (const KisPoint& P1, const KisPoint& P2)
{
    KisPoint temp;
    temp.setX((P1.x()+P2.x())/2);
    temp.setY((P1.y()+P2.y())/2);
    return temp;
}

void KisCurveBezier::recursiveCurve (const KisPoint& P1, const KisPoint& P2, const KisPoint& P3,
                                     const KisPoint& P4, int level, KisCurve::iterator it)
{
    if (level > m_maxLevel) {
        addPoint(it,midpoint(P1,P4),false,false,LINEHINT);
        return;
    }

    KisPoint L1, L2, L3, L4;
    KisPoint H, R1, R2, R3, R4;

    L1 = P1;
    L2 = midpoint(P1, P2);
    H  = midpoint(P2, P3);
    R3 = midpoint(P3, P4);
    R4 = P4;
    L3 = midpoint(L2, H);
    R2 = midpoint(R3, H);
    L4 = midpoint(L3, R2);
    R1 = L4;
    recursiveCurve(L1, L2, L3, L4, level + 1, it);
    recursiveCurve(R1, R2, R3, R4, level + 1, it);
}

KisCurve::iterator KisCurveBezier::pushPivot (const KisPoint& point) {
    KisPoint prevTrans(15.0,-15.0);
    KisPoint nextTrans(-15.0,15.0);
    iterator it,prev;

    it = pushPoint(point,true,false,BEZIERENDHINT);
    if (count() > 1)
        prev = addPoint(it,point+prevTrans,true,false,BEZIERPREVCONTROLHINT);
    
    it = pushPoint(point+nextTrans,true,false,BEZIERNEXTCONTROLHINT);
    
    return selectPivot(it);
}

void KisCurveBezier::calculateCurve(KisCurve::iterator tstart, KisCurve::iterator tend, KisCurve::iterator)
{
    if (pivots().count() < 4)
        return;

    iterator origin, dest, control1, control2;

    if ((*tstart).hint() == BEZIERENDHINT) {
        origin = tstart;
        control1 = tstart.nextPivot();
    } else if ((*tstart).hint() == BEZIERNEXTCONTROLHINT) {
        origin = tstart.previousPivot();
        control1 = tstart;
    } else if ((*tstart).hint() == BEZIERPREVCONTROLHINT) {
        origin = tstart.nextPivot();
        control1 = origin.nextPivot();
    } else
        return;
        
    if ((*tend).hint() == BEZIERENDHINT) {
        dest = tend;
        control2 = tend.previousPivot();
    } else if ((*tend).hint() == BEZIERPREVCONTROLHINT) {
        dest = tend.nextPivot();
        control2 = tend;
    } else if ((*tend).hint() == BEZIERNEXTCONTROLHINT) {
        dest = tend.previousPivot();
        control2 = dest.previousPivot();
    } else
        return;

    deleteCurve(control1,control2);
    recursiveCurve((*origin).point(),(*control1).point(),(*control2).point(),(*dest).point(),1,control2);
    
}

KisCurve::iterator KisCurveBezier::movePivot(KisCurve::iterator it, const KisPoint& newPt)
{
    if (!(*it).isPivot())
        return end();

    int hint = (*it).hint();
    iterator thisEnd, prevEnd, nextEnd;

    if (hint == BEZIERENDHINT) {
        thisEnd = it;
        nextEnd = it.nextPivot().nextPivot().nextPivot();    // Horrible...
        prevEnd = it.previousPivot().previousPivot().previousPivot();
    } else if (hint == BEZIERPREVCONTROLHINT) {
        thisEnd = it.nextPivot();
        nextEnd = end();
        prevEnd = it.previousPivot().previousPivot();
    } else if ((*it) == last()) {
        thisEnd = it.previousPivot();
        nextEnd = end();
        prevEnd = thisEnd.previousPivot().previousPivot().previousPivot();
    } else if (hint == BEZIERNEXTCONTROLHINT) {
        thisEnd = it.previousPivot();
        nextEnd = it.nextPivot().nextPivot();
        prevEnd = end();
    }

    if (hint == BEZIERENDHINT) {
        KisPoint trans = newPt - (*it).point();
        (*thisEnd).setPoint((*thisEnd).point()+trans);
        (*thisEnd.previous()).setPoint((*thisEnd.previous()).point()+trans);
        (*thisEnd.next()).setPoint((*thisEnd.next()).point()+trans);
    } else
        (*it).setPoint(newPt);
    if (hint == BEZIERPREVCONTROLHINT && (*it.next().next()) == last() || (m_actionOptions & SYMMETRICALCONTROLSOPTION)) {
        KisPoint trans = (*it).point() - (*thisEnd).point();
        trans = KisPoint(-trans.x()*2,-trans.y()*2);
        (*it.next().next()).setPoint(newPt+trans);
    }

    if (hint == BEZIERNEXTCONTROLHINT && (*it) == last() || (m_actionOptions & SYMMETRICALCONTROLSOPTION)) {
        KisPoint trans = (*it).point() - (*thisEnd).point();
        trans = KisPoint(-trans.x()*2,-trans.y()*2);
        (*it.previous().previous()).setPoint(newPt+trans);
    }

    if (nextEnd != end() && count() > 4)
        calculateCurve (thisEnd,nextEnd,iterator());
    if (prevEnd != thisEnd && count() > 4)
        calculateCurve (prevEnd,thisEnd,iterator());

    return it;
}

bool KisCurveBezier::deletePivot (KisCurve::iterator it)
{
    if (!(*it).isPivot())
        return false;

    iterator prevControl,nextControl;

    if ((*it).hint() == BEZIERENDHINT) {
        prevControl = it.previousPivot().previousPivot();
        nextControl = it.nextPivot().nextPivot();
    } else if ((*it).hint() == BEZIERPREVCONTROLHINT) {
        prevControl = it.previousPivot();
        nextControl = it.nextPivot().nextPivot().nextPivot();
    } else if ((*it).hint() == BEZIERNEXTCONTROLHINT) {
        prevControl = it.previousPivot().previousPivot().previousPivot();
        nextControl = it.nextPivot();
    } else
        return KisCurve::deletePivot(it);

    if ((*prevControl) == first()) {
        deleteFirstPivot();
        deleteFirstPivot();
        deleteFirstPivot();
    } else if (nextControl == end()) {
        deleteLastPivot();
        deleteLastPivot();
        deleteLastPivot();
    } else
        calculateCurve(prevControl,nextControl,iterator());

    return true;
}

KisToolBezier::KisToolBezier()
    : super(i18n("Tool for Bezier"))
{
    setName("tool_bezier");
    setCursor(KisCursor::load("tool_bezier_cursor.png", 6, 6));

    m_curve = new KisCurveBezier;
}

KisToolBezier::~KisToolBezier()
{

}

void KisToolBezier::doubleClick(KisDoubleClickEvent *)
{
    paintCurve();
    m_curve->clear();
}

long KisToolBezier::convertStateToOptions(long state)
{
    long options = KisToolCurve::convertStateToOptions(state);

    if (state & Qt::AltButton)
        options |= SYMMETRICALCONTROLSOPTION;

    return options;
}

KisCurve::iterator KisToolBezier::paintPoint (KisPainter& painter, KisCurve::iterator point)
{
    KisPoint origin,destination,control1,control2;
    switch ((*point).hint()) {
    case BEZIERENDHINT:
        if (m_curve->count() > 4 && (*point.next()) != m_curve->last()) {
            origin = (*point++).point();
            control1 = (*point).point();
            point = point.nextPivot();
            control2 = (*point++).point();
            destination = (*point).point();
            painter.paintBezierCurve(origin,PRESSURE_DEFAULT,0,0,control1,control2,destination,PRESSURE_DEFAULT,0,0,0);
        } else
            point += 1;
        break;
    default:
        point = KisToolCurve::paintPoint(painter,point);
    }

    return point;
}

KisCurve::iterator KisToolBezier::drawPivot(KisCanvasPainter& gc, KisCurve::iterator point, const KisCurve& curve)
{
    KisCanvasController *controller = m_subject->canvasController();
    QPoint prevControlPos,endpPos,nextControlPos;
    QPen oldPen, prevLinePen, nextLinePen, endpPen;
    QColor prevControlColor, nextControlColor;
    QRect endpRect, nextControlRect, prevControlRect;
    KisCurve::iterator endp, prevControl, nextControl;
    if ((*point).hint() == BEZIERNEXTCONTROLHINT) {
        endp = point.previousPivot();
        nextControl = point;
        if (curve.count() > 2)
            prevControl = endp.previousPivot();
        else
            prevControl = curve.end();
    } else
        return point;

    endpPos = controller->windowToView((*endp).point().toQPoint());
    nextControlPos = controller->windowToView((*nextControl).point().toQPoint());
    prevControlPos = controller->windowToView((*prevControl).point().toQPoint());
    endpRect = QRect(endpPos-QPoint(5,5),
                     endpPos+QPoint(5,5));
    nextControlRect = QRect(nextControlPos-QPoint(4,4),
                            nextControlPos+QPoint(4,4));
    prevControlRect = QRect(prevControlPos-QPoint(4,4),
                            prevControlPos+QPoint(4,4));
    oldPen = gc.pen();

    if ((*endp).isSelected())
        endpPen = QPen(Qt::yellow, 1, Qt::SolidLine);
    else
        endpPen = QPen(Qt::darkGreen, 1, Qt::SolidLine);
        
    if ((*nextControl).isSelected())
        nextControlColor = Qt::yellow;
    else
        nextControlColor = Qt::darkRed;
        
    if (prevControl != curve.end() && (*prevControl).isSelected())
        prevControlColor = Qt::yellow;
    else
        prevControlColor = Qt::darkRed;
        
    if ((*endp).isSelected() || (*prevControl).isSelected())
        prevLinePen = QPen(Qt::yellow, 1, Qt::SolidLine);
    else
        prevLinePen = QPen(Qt::red, 0, Qt::SolidLine);

    if ((*endp).isSelected() || (*nextControl).isSelected())
        nextLinePen = QPen(Qt::yellow, 1, Qt::SolidLine);
    else
        nextLinePen = QPen(Qt::red, 0, Qt::SolidLine);
        
    gc.setPen(endpPen);
    gc.drawEllipse(endpRect);
    gc.fillRect(nextControlRect,nextControlColor);
    if (prevControl != curve.end()) {
        gc.setPen(prevLinePen);
        gc.fillRect(prevControlRect,prevControlColor);
        gc.drawLine(prevControlPos,endpPos);
    }
    gc.setPen(nextLinePen);
    gc.drawLine(endpPos,nextControlPos);
    gc.setPen(oldPen);

    // Now draw the bezier

    KisCurve::iterator nextEnd, forwardControl;

    forwardControl = nextControl.nextPivot();
    nextEnd = forwardControl.nextPivot();
    if (forwardControl != curve.end()) {
        point = forwardControl;
        QPointArray vec(4);
        vec[0] = controller->windowToView((*endp).point().toQPoint());
        vec[1] = controller->windowToView((*nextControl).point().toQPoint());
        vec[2] = controller->windowToView((*forwardControl).point().toQPoint());
        vec[3] = controller->windowToView((*nextEnd).point().toQPoint());
        gc.drawCubicBezier(vec);
    }

    return point;
}

void KisToolBezier::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KShortcut(Qt::Key_F9));
        m_action = new KRadioAction(i18n("&Bezier"),
                                    "tool_bezier",
                                    shortcut,
                                    this,
                                    SLOT(activate()),
                                    collection,
                                    name());
        Q_CHECK_PTR(m_action);

        m_action->setToolTip(i18n("Draw cubic beziers: click and drag the control points. Double-click to finish."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_bezier.moc"
