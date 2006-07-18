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
const int BEZIERCONTROLHINT = 0x0020;

const int MOVEWITHCONTROLOPTION = 0x0010;

class KisCurveBezier : public KisCurve {

    typedef KisCurve super;
    
public:

    KisCurveBezier() : super() {}

    ~KisCurveBezier() {}

    virtual iterator pushPivot(const KisPoint&);
    virtual void calculateCurve(iterator, iterator, iterator);
    virtual iterator movePivot(iterator, const KisPoint&);
    virtual bool deletePivot(iterator);

};

KisCurve::iterator KisCurveBezier::pushPivot (const KisPoint& point) {
    pushPoint(point,true,false,BEZIERENDHINT);
    iterator it = pushPoint(point,true,false,BEZIERCONTROLHINT);
    return selectPivot(it);
}

void KisCurveBezier::calculateCurve(KisCurve::iterator tstart, KisCurve::iterator tend, KisCurve::iterator)
{
    if (pivots().count() < 4)
        return;

    iterator origin, dest, control1, control2;

    double t, increase;

    double ax, bx, cx;
    double ay, by, cy;

    double x0, x1, x2, x3;
    double y0, y1, y2, y3;

    double xfinal, yfinal;

    if ((*tstart).hint() == BEZIERENDHINT) {
        origin = tstart;
        control1 = tstart.nextPivot();
    } else if ((*tstart).hint() == BEZIERCONTROLHINT) {
        origin = tstart.previousPivot();
        control1 = tstart;
    } else
        return;
        
    if ((*tend).hint() == BEZIERENDHINT) {
        dest = tend;
        control2 = tend.nextPivot();
    } else if ((*tend).hint() == BEZIERCONTROLHINT) {
        dest = tend.previousPivot();
        control2 = tend;
    } else
        return;

    x0 = (*origin).point().x(); y0 = (*origin).point().y();
    x1 = (*control1).point().x(); y1 = (*control1).point().y();
    x2 = (*control2).point().x(); y2 = (*control2).point().y();
    x3 = (*dest).point().x(); y3 = (*dest).point().y();

    cx = 3 * (x1 - x0);
    bx = 3 * (x2 - x1) - cx;
    ax = x3 - x0 - cx - bx;
    cy = 3 * (y1 - y0);
    by = 3 * (y2 - y1) - cy;
    ay = y3 - y0 - cy - by;

    deleteCurve(control1,dest);

    increase = 0.02;
    for (t = 0; t <= 1; t+=increase) {
        xfinal = ax * (pow(t,3)) + bx * (pow(t,2)) + cx * t + x0;
        yfinal = ay * (pow(t,3)) + by * (pow(t,2)) + cy * t + y0;

        addPoint(dest,KisPoint(xfinal,yfinal),false,false,LINEHINT);
    }
}

KisCurve::iterator KisCurveBezier::movePivot(KisCurve::iterator it, const KisPoint& newPt)
{
    if (!(*it).isPivot()) {
        kdDebug(0) << "Ma che ci divertiamo?" << endl;
        return end();
    }

    iterator endp, control;

    if ((*it).hint() == BEZIERENDHINT) {
        endp = it;
        control = it.nextPivot();
    } else if ((*it).hint() == BEZIERCONTROLHINT) {
        endp = it.previousPivot();
        control = it;
    } else
        return KisCurve::movePivot(it, newPt);

    if (m_actionOptions & MOVEWITHCONTROLOPTION) {
        KisPoint trans = newPt-(*it).point();
        (*endp).setPoint((*endp).point() + trans);
        (*control).setPoint((*control).point() + trans);
    } else
        (*it).setPoint(newPt);

    if ((*endp) != first())
        calculateCurve(endp.previousPivot(), endp, KisCurve::iterator());
    if ((*control) != last())
        calculateCurve(control, control.nextPivot(), KisCurve::iterator());

    return it;
}

bool KisCurveBezier::deletePivot (KisCurve::iterator it)
{
    if (!(*it).isPivot())
        return false;

    iterator endp, control, prevControl, nextEnd;

    if ((*it).hint() == BEZIERENDHINT) {
        endp = it;
        control = it.nextPivot();
    } else if ((*it).hint() == BEZIERCONTROLHINT) {
        endp = it.previousPivot();
        control = it;
    } else
        return KisCurve::deletePivot(it);

    if ((*endp) == first()) {
        deleteFirstPivot();
        deleteFirstPivot();
    } else if ((*control) == last()) {
        deleteLastPivot();
        deleteLastPivot();
    } else {
        prevControl = endp.previousPivot();
        nextEnd = control.nextPivot();
        calculateCurve(prevControl,nextEnd,KisCurve::iterator());
    }

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
    draw();
    m_curve->deleteLastPivot();
    if (m_curve->pivots().count()%2)
        m_curve->deleteLastPivot();
    draw();
    paintCurve();
    m_curve->clear();
}

long KisToolBezier::convertStateToOptions(long state)
{
    long options = NOOPTIONS;

    if (state & Qt::AltButton)
        options |= MOVEWITHCONTROLOPTION;

    options |= KisToolCurve::convertStateToOptions(state);

    return options;
}

KisCurve::iterator KisToolBezier::drawPivot(KisCanvasPainter& gc, KisCurve::iterator point, const KisCurve& curve)
{
    KisCanvasController *controller = m_subject->canvasController();
    QPoint pos1, pos2;
    QPen oldPen, linePen, endpPen;
    QColor controlColor;
    QRect endpRect, controlRect;
    KisCurve::iterator endp, control;
    if ((*point).hint() == BEZIERCONTROLHINT) {
        endp = point.previousPivot();
        control = point;
    } else
        return point;

    pos1 = controller->windowToView((*endp).point().toQPoint());
    pos2 = controller->windowToView((*control).point().toQPoint());
    endpRect = QRect(pos1-QPoint(5,5),
                     pos1+QPoint(5,5));
    controlRect = QRect(pos2-QPoint(4,4),
                        pos2+QPoint(4,4));
    oldPen = gc.pen();

    if ((*endp).isSelected())
        endpPen = QPen(Qt::yellow, 1, Qt::SolidLine);
    else
        endpPen = QPen(Qt::darkGreen, 1, Qt::SolidLine);
    if ((*control).isSelected())
        controlColor = Qt::yellow;
    else
        controlColor = Qt::darkRed;
        
    if ((*endp).isSelected() || (*control).isSelected())
        linePen = QPen(Qt::yellow, 1, Qt::SolidLine);
    else
        linePen = QPen(Qt::red, 0, Qt::SolidLine);
        
    gc.setPen(endpPen);
    gc.drawEllipse(endpRect);
    gc.fillRect(controlRect,controlColor);
    gc.setPen(linePen);
    gc.drawLine(pos2,pos1);
    gc.setPen(oldPen);

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
