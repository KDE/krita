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

const int BEZIERHINT = 2;

class KisCurveBezier : public KisCurve {

    typedef KisCurve super;
    
public:

    KisCurveBezier() : super() {}

    ~KisCurveBezier() {}

    virtual void calculateCurve(iterator, iterator, iterator) {calculateCurve();}
    virtual void calculateCurve();

};

void KisCurveBezier::calculateCurve()
{
    if (count() != 4)
        return;

    iterator it = find(m_curve[count()-2]);

    double t, increase;

    double ax, bx, cx;
    double ay, by, cy;

    double x0, x1, x2, x3;
    double y0, y1, y2, y3;

    double xfinal, yfinal;

    x0 = m_curve[0].point().x(); y0 = m_curve[0].point().y();
    x1 = m_curve[1].point().x(); y1 = m_curve[1].point().y();
    x2 = m_curve[count()-1].point().x(); y2 = m_curve[count()-1].point().y();
    x3 = m_curve[count()-2].point().x(); y3 = m_curve[count()-2].point().y();

    increase = 0.02;

    cx = 3 * (x1 - x0);
    bx = 3 * (x2 - x1) - cx;
    ax = x3 - x0 - cx - bx;
    cy = 3 * (y1 - y0);
    by = 3 * (y2 - y1) - cy;
    ay = y3 - y0 - cy - by;

    for (t = 0; t <= 1; t+=increase) {
        xfinal = ax * (pow(t,3)) + bx * (pow(t,2)) + cx * t + x0;
        yfinal = ay * (pow(t,3)) + by * (pow(t,2)) + cy * t + y0;

        addPoint(it,KisPoint(xfinal,yfinal),false,false,LINEHINT);
    }
}

KisToolBezier::KisToolBezier()
    : super(i18n("Tool for Bezier"))
{
    setName("tool_bezier");
    setCursor(KisCursor::load("tool_bezier_cursor.png", 6, 6));

    m_dragging = false;
    m_editing = false;
    m_curve = new KisCurveBezier;
}

KisToolBezier::~KisToolBezier()
{

}


void KisToolBezier::update (KisCanvasSubject *subject)
{
    super::update (subject);
    if (m_subject)
        m_currentImage = m_subject->currentImg ();
}


void KisToolBezier::deactivate()
{
    super::deactivate();
    m_dragging = false;
    m_editing = false;
}

void KisToolBezier::buttonPress(KisButtonPressEvent *event)
{
    if (m_currentImage && event->button() == LeftButton) {
        draw();
        m_dragging = true;
        if (!m_editing) {
            switch (m_curve->count()) {
            case 0:
                m_origin = CurvePoint(event->pos(),true,false,BEZIERHINT);
                m_curve->pushPivot(m_origin);
                break;
            case 1:
                m_control1 = CurvePoint(event->pos(),true,false,BEZIERHINT);
                m_curve->pushPivot(m_control1);
                break;
            case 2:
                m_control2 = CurvePoint(event->pos(),true,false,BEZIERHINT);
                m_curve->pushPivot(m_control2);
                break;
            case 3:
                m_destination = CurvePoint(event->pos(),true,false,BEZIERHINT);
                m_curve->addPivot(m_curve->find(m_control2),m_destination);
                m_curve->calculateCurve();
                break;
            default:
                m_editing = true;
            }
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
/*
void KisToolBezier::keyPress(QKeyEvent *event)
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
*/
void KisToolBezier::move(KisMoveEvent *event)
{
    if (m_dragging) {
        draw();
        if (!m_editing) {
            if (m_curve->pivots().count() > 1)
                m_curve->deleteLastPivot();
            switch (m_curve->count()) {
            case 0:
                m_origin = CurvePoint(event->pos(),true,false,BEZIERHINT);
                m_curve->pushPivot(m_origin);
                break;
            case 1:
                m_control1 = CurvePoint(event->pos(),true,false,BEZIERHINT);
                m_curve->pushPivot(m_control1);
                break;
            case 2:
                m_control2 = CurvePoint(event->pos(),true,false,BEZIERHINT);
                m_curve->pushPivot(m_control2);
                break;
            case 3:
                m_destination = CurvePoint(event->pos(),true,false,BEZIERHINT);
                m_curve->addPivot(m_curve->find(m_control2),m_destination);
                m_curve->calculateCurve();
                m_editing = true;
                break;
            }
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

void KisToolBezier::buttonRelease(KisButtonReleaseEvent *)
{
    if (!m_subject || !m_currentImage)
        return;

    m_dragging = false;
}

void KisToolBezier::doubleClick(KisDoubleClickEvent *)
{
    if (m_editing) {
        paintCurve();
        m_curve->clear();
        m_editing = false;
    }
}

void KisToolBezier::draw(const KisCurve& curve)
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

    QPen pen = QPen(Qt::red, 0, Qt::SolidLine);
    gc->setPen(pen);
    gc->setRasterOp(Qt::XorROP);

    int count = 0;
    QPoint pos1, pos2;
    KisCurve::iterator it = curve.begin();
    while (it != curve.end()) {
        switch ((*it).hint()) {
        case BEZIERHINT:
            pos1 = controller->windowToView((*it).point().toQPoint());
            if (count <= 1) {
                gc->fillRect(QRect(pos1-QPoint(4,4),
                                   pos1+QPoint(4,4)),Qt::red);
            }
            if (count >= 2) {
                pen = QPen(Qt::green, 0, Qt::SolidLine);
                gc->setPen(pen);
                gc->fillRect(QRect(pos1-QPoint(4,4),
                                   pos1+QPoint(4,4)),Qt::green);
            }
            if ((count == 0 && curve.count() > 1) || (count == 2 && curve.count() > 3)) {
                pos2 = controller->windowToView((*it.nextPivot()).point().toQPoint());
                gc->drawLine(pos1,pos2);
            }
            it += 1;
            count += 1;
            break;
        default:
            pen = QPen(Qt::white, 0, Qt::SolidLine);
            gc->setPen(pen);
            it = drawPoint(*gc, it, curve);
        }
    }
    
    delete gc;
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

        m_action->setToolTip(i18n("Draw a cubic bezier."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_bezier.moc"
