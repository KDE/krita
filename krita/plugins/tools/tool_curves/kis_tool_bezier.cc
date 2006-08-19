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
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_cursor.h"
#include "kis_vec.h"

#include "kis_curve_framework.h"
#include "kis_tool_bezier.h"

KisCurve::iterator KisCurveBezier::groupEndpoint (KisCurve::iterator it) const
{
    iterator temp = it;
    if ((*it).hint() == BEZIERNEXTCONTROLHINT)
        temp -= 1;
    if ((*it).hint() == BEZIERPREVCONTROLHINT)
        temp += 1;
    return temp;
}

KisCurve::iterator KisCurveBezier::groupPrevControl (KisCurve::iterator it) const
{
    iterator temp = it;
    if ((*it).hint() == BEZIERENDHINT)
        temp -= 1;
    if ((*it).hint() == BEZIERNEXTCONTROLHINT)
        temp -= 2;
    return temp;
}

KisCurve::iterator KisCurveBezier::groupNextControl (KisCurve::iterator it) const
{
    iterator temp = it;
    if ((*it).hint() == BEZIERENDHINT)
        temp += 1;
    if ((*it).hint() == BEZIERPREVCONTROLHINT)
        temp += 2;
    return temp;
}

bool KisCurveBezier::groupSelected (KisCurve::iterator it) const
{
    if ((*groupPrevControl(it)).isSelected() || (*groupEndpoint(it)).isSelected() || (*groupNextControl(it)).isSelected())
        return true;
    return false;
}

KisCurve::iterator KisCurveBezier::nextGroupEndpoint (KisCurve::iterator it) const
{
    iterator temp = it;
    if ((*it).hint() == BEZIERPREVCONTROLHINT) {
        temp += 2;
        temp = temp.nextPivot();
    }
    if ((*it).hint() == BEZIERENDHINT) {
        temp += 1;
        temp = temp.nextPivot();
    }
    if ((*it).hint() == BEZIERNEXTCONTROLHINT) {
        temp = temp.nextPivot();
    }
    temp = temp.nextPivot();
    return temp;
}

KisCurve::iterator KisCurveBezier::prevGroupEndpoint (KisCurve::iterator it) const
{
    iterator temp = it;
    if ((*it).hint() == BEZIERNEXTCONTROLHINT) {
        temp -= 1;
        temp = temp.previousPivot().previousPivot();
    }
    if ((*it).hint() == BEZIERENDHINT) {
        temp = temp.previousPivot().previousPivot();
    }
    if ((*it).hint() == BEZIERPREVCONTROLHINT) {
        temp = temp.previousPivot();
    }
    temp = temp.previousPivot();
    return temp;
}

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

KisCurve::iterator KisCurveBezier::pushPivot (const KisPoint& point)
{
    iterator it;

    it = pushPoint(point,true,false,BEZIERENDHINT);
    if (count() > 1)
        addPoint(it,point,true,false,BEZIERPREVCONTROLHINT);
    
    it = pushPoint(point,true,false,BEZIERNEXTCONTROLHINT);
    
    return selectPivot(it);
}

KisCurve::iterator KisCurveBezier::movePivot(KisCurve::iterator it, const KisPoint& newPt)
{
    if (!(*it).isPivot())
        return end();

    int hint = (*it).hint();
    iterator thisEnd, prevEnd, nextEnd;

    thisEnd = groupEndpoint(it);
    prevEnd = prevGroupEndpoint(it);
    nextEnd = nextGroupEndpoint(it);

    if (hint == BEZIERENDHINT) {
        KisPoint trans = newPt - (*it).point();
        (*thisEnd).setPoint((*thisEnd).point()+trans);
        (*thisEnd.previous()).setPoint((*thisEnd.previous()).point()+trans);
        (*thisEnd.next()).setPoint((*thisEnd.next()).point()+trans);
    } else if (!(m_actionOptions & KEEPSELECTEDOPTION))
        (*it).setPoint(newPt);
    if (!(m_actionOptions & KEEPSELECTEDOPTION) && hint != BEZIERENDHINT) {
        if (nextEnd == end() || (m_actionOptions & SYMMETRICALCONTROLSOPTION)) {
            KisPoint trans = (*it).point() - (*thisEnd).point();
            trans = KisPoint(-trans.x()*2,-trans.y()*2);
            if (hint == BEZIERNEXTCONTROLHINT)
                (*groupPrevControl(it)).setPoint(newPt+trans);
            else
                (*groupNextControl(it)).setPoint(newPt+trans);
        }
    }
    
    if (nextEnd != end() && count() > 4)
        calculateCurve (thisEnd,nextEnd,iterator());
    if (prevEnd != thisEnd && count() > 4)
        calculateCurve (prevEnd,thisEnd,iterator());

    return it;
}

void KisCurveBezier::deletePivot (KisCurve::iterator it)
{
    if (!(*it).isPivot())
        return;

    iterator prevControl,thisEnd,nextControl;

    prevControl = prevGroupEndpoint(it).nextPivot();
    thisEnd = groupEndpoint(it);
    nextControl = nextGroupEndpoint(it).previousPivot();

    if ((*thisEnd) == first()) {
        deleteFirstPivot();
        deleteFirstPivot();
        deleteFirstPivot();
    } else if ((*thisEnd.next()) == last()) {
        deleteLastPivot();
        deleteLastPivot();
        deleteLastPivot();
    } else {
        deleteCurve(prevControl,nextControl);
        calculateCurve(prevControl,nextControl,iterator());
    }
}

KisToolBezier::KisToolBezier(const QString& UIName)
    : super(UIName)
{
    m_derivated = new KisCurveBezier;
    m_curve = m_derivated;

    m_supportMinimalDraw = false;

    m_transactionMessage = QString("Bezier Curve");
}

KisToolBezier::~KisToolBezier()
{

}

KisCurve::iterator KisToolBezier::handleUnderMouse(const QPoint& pos)
{
    QPoint qpos;
    KisCurve pivs = m_curve->pivots(), inHandle;
    KisCurve::iterator it;
    int hint;
    for (it = pivs.begin(); it != pivs.end(); it++) {
        qpos = m_subject->canvasController()->windowToView((*it).point().toQPoint());
        hint = (*it).hint();
        if (hint != BEZIERENDHINT && !m_derivated->groupSelected(it))
            continue;
        if (hint == BEZIERENDHINT && (m_actionOptions & SHIFTOPTION))
            continue;
        if (pivotRect(qpos).contains(pos)) {
            inHandle.pushPoint((*it));
            if (hint == BEZIERENDHINT && !(m_actionOptions & SHIFTOPTION))
                break;
            if (hint != BEZIERENDHINT && (m_actionOptions & SHIFTOPTION))
                break;
        }
    }
    if (inHandle.isEmpty())
        return m_curve->end();

    return m_curve->find(inHandle.last());
}

KisCurve::iterator KisToolBezier::drawPoint (KisCanvasPainter& gc, KisCurve::iterator point)
{
    if ((*point).hint() != BEZIERENDHINT)
        return ++point;

    KisCanvasController *controller = m_subject->canvasController();

    // Now draw the bezier
    
    KisCurve::iterator origin,control1,control2,destination;

    origin = point;
    control1 = origin.next();
    control2 = control1.nextPivot();
    destination = control2.next();

    if (control2 != m_curve->end()) {
        point = control2;
        QPointArray vec(4);
        vec[0] = controller->windowToView((*origin).point().toQPoint());
        vec[1] = controller->windowToView((*control1).point().toQPoint());
        vec[2] = controller->windowToView((*control2).point().toQPoint());
        vec[3] = controller->windowToView((*destination).point().toQPoint());
        gc.drawCubicBezier(vec);
    }

    point += 1;

    return point;
}

void KisToolBezier::drawPivotHandle (KisCanvasPainter& gc, KisCurve::iterator point)
{
    if ((*point).hint() != BEZIERENDHINT)
        return;

    KisCanvasController *controller = m_subject->canvasController();

    QPoint endpPos = controller->windowToView((*point).point().toQPoint());

    if (!m_derivated->groupSelected(point)) {
        gc.setPen(m_pivotPen);
        gc.drawRoundRect(pivotRect(endpPos),m_pivotRounding,m_pivotRounding);
    } else {
        QPoint nextControlPos = controller->windowToView((*point.next()).point().toQPoint());
        QPoint prevControlPos = controller->windowToView((*point.previousPivot()).point().toQPoint());

        gc.setPen(m_selectedPivotPen);
        gc.drawRoundRect(selectedPivotRect(endpPos),m_selectedPivotRounding,m_selectedPivotRounding);
        if ((prevControlPos != endpPos || nextControlPos != endpPos) && !(m_actionOptions & CONTROLOPTION)) {
            gc.drawRoundRect(pivotRect(nextControlPos),m_pivotRounding,m_pivotRounding);
            gc.drawLine(endpPos,nextControlPos);
            gc.drawRoundRect(pivotRect(prevControlPos),m_pivotRounding,m_pivotRounding);
            gc.drawLine(prevControlPos,endpPos);
        }
    }

    gc.setPen(m_drawingPen);
}

#include "kis_tool_bezier.moc"
