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
        temp -= 2;
        temp = temp.previousPivot();
    }
    if ((*it).hint() == BEZIERENDHINT) {
        temp -= 1;
        temp = temp.previousPivot();
    }
    if ((*it).hint() == BEZIERPREVCONTROLHINT) {
        temp = temp.previousPivot();
    }
    temp = temp.previousPivot();
    return temp;
}

KisCurve::iterator KisCurveBezier::pushPivot (const KisPoint& point) {
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
    } else
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
    } else {
        deleteCurve(prevControl,nextControl);
        calculateCurve(prevControl,nextControl,iterator());
    }
}

KisCurve::iterator KisCurveBezier::selectPivot(KisCurve::iterator it, bool isSelected)
{

    if (m_actionOptions & KEEPSELECTEDOPTION) {
        KisCurve selected = selectedPivots();
        for (iterator i = selected.begin(); i != selected.end(); i++) {
            if ((*i).hint() != BEZIERENDHINT)
                (*find((*i))).setSelected(false);
        }
    }

    return super::selectPivot(it,isSelected);
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

int KisToolBezier::convertKeysToOptions(int keys)
{
    int options = KisToolCurve::convertKeysToOptions(keys);

    if (keys & Qt::Key_Alt)
        options |= SYMMETRICALCONTROLSOPTION;
    if (keys & Qt::Key_Shift)
        options |= PREFERCONTROLSOPTION;

    return options;
}

KisCurve::iterator KisToolBezier::selectByHandle(const QPoint& pos)
{
    QPoint qpos;
    KisCurve pivs = m_curve->pivots(), inHandle;
    KisCurve::iterator it;
    for (it = pivs.begin(); it != pivs.end(); it++) {
        qpos = m_subject->canvasController()->windowToView((*it).point().toQPoint());
        if (pivotRect(qpos).contains(pos)) {
            if ((m_pressedKeys & Qt::ControlButton) && (*it).hint() != BEZIERENDHINT);
            else if ((m_pressedKeys & Qt::ShiftButton) && (*it).hint() == BEZIERENDHINT);
            else
                inHandle.pushPoint((*it));
        }
    }
    if (inHandle.isEmpty())
        return m_curve->end();

    if (inHandle.count() > 1) {
        if (!(m_pressedKeys & Qt::ShiftButton) || (m_pressedKeys & Qt::ControlButton)) {
            for (KisCurve::iterator i = inHandle.begin(); i != inHandle.end();) {
                if ((*i).hint() != BEZIERENDHINT)
                    inHandle.deletePivot((*i++));
                else
                    i++;
            }
        }
    }

    return m_curve->selectPivot(inHandle.first());
}

KisCurve::iterator KisToolBezier::paintPoint (KisPainter& painter, KisCurve::iterator point)
{
    KisPoint origin,destination,control1,control2;
    switch ((*point).hint()) {
    case BEZIERENDHINT:
        if (m_curve->count() > 4 && (*point.next()) != m_curve->last()) {
            painter.paintAt((*point--).point(),PRESSURE_DEFAULT,0,0);
            painter.paintBezierCurve((*++point).point(),PRESSURE_DEFAULT,0,0,(*++point).point(),
            (*++point).point(),(*++point).point(),PRESSURE_DEFAULT,0,0,0);
        } else
            point += 1;
        break;
    default:
        point = KisToolCurve::paintPoint(painter,point);
    }

    return point;
}

KisCurve::iterator KisToolBezier::drawPivot (KisCanvasPainter& gc, KisCurve::iterator point)
{
    if ((*point).hint() != BEZIERENDHINT)
        return ++point;

    KisCanvasController *controller = m_subject->canvasController();

    QPoint endpPos = controller->windowToView((*point).point().toQPoint());
    
    if (!(*point).isSelected() && !(*point.previous()).isSelected() && !(*point.next()).isSelected()) {
        gc.setPen(m_pivotPen);
        gc.drawRoundRect(pivotRect(endpPos),m_pivotRounding,m_pivotRounding);
    } else {
        QPoint nextControlPos = controller->windowToView((*point.next()).point().toQPoint());
        QPoint prevControlPos = controller->windowToView((*point.previousPivot()).point().toQPoint());

        gc.setPen(m_selectedPivotPen);
        gc.drawRoundRect(selectedPivotRect(endpPos),m_selectedPivotRounding,m_selectedPivotRounding);
        if ((prevControlPos != endpPos || nextControlPos != endpPos) && !(m_pressedKeys & Qt::ControlButton)) {
            gc.drawRoundRect(pivotRect(nextControlPos),m_pivotRounding,m_pivotRounding);
            gc.drawLine(endpPos,nextControlPos);
            gc.drawRoundRect(pivotRect(prevControlPos),m_pivotRounding,m_pivotRounding);
            gc.drawLine(prevControlPos,endpPos);
        }
    }

    gc.setPen(m_drawingPen);

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

    return ++point;
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

        m_action->setToolTip(i18n("Draw cubic beziers. Keep Alt, Control or Shift pressed for options. Return or double-click to finish."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_bezier.moc"
