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
    if (hint == BEZIERPREVCONTROLHINT && ((*it.next().next()) == last() || (m_actionOptions & SYMMETRICALCONTROLSOPTION))) {
        KisPoint trans = (*it).point() - (*thisEnd).point();
        trans = KisPoint(-trans.x()*2,-trans.y()*2);
        (*it.next().next()).setPoint(newPt+trans);
    }

    if (hint == BEZIERNEXTCONTROLHINT && ((*it) == last() || (m_actionOptions & SYMMETRICALCONTROLSOPTION))) {
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
    } else
        calculateCurve(prevControl,nextControl,iterator());
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
    KisCanvasController *controller = m_subject->canvasController();
    QPoint prevControlPos,endpPos,nextControlPos;
    QPen oldPen, prevLinePen, nextLinePen, endpPen;
    QColor prevControlColor, nextControlColor;
    QRect endpRect, nextControlRect, prevControlRect;
    KisCurve::iterator endp, prevControl, nextControl;
    if ((*point).hint() == BEZIERNEXTCONTROLHINT) {
        endp = point.previousPivot();
        nextControl = point;
        if (m_curve->count() > 2)
            prevControl = endp.previousPivot();
        else
            prevControl = m_curve->end();
    } else
        return ++point;

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
        
    if (prevControl != m_curve->end() && (*prevControl).isSelected())
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
    if (prevControl != m_curve->end()) {
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
    if (forwardControl != m_curve->end()) {
        point = forwardControl;
        QPointArray vec(4);
        vec[0] = controller->windowToView((*endp).point().toQPoint());
        vec[1] = controller->windowToView((*nextControl).point().toQPoint());
        vec[2] = controller->windowToView((*forwardControl).point().toQPoint());
        vec[3] = controller->windowToView((*nextEnd).point().toQPoint());
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

        m_action->setToolTip(i18n("Draw cubic beziers: click and drag the control points. Double-click to finish."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_bezier.moc"
