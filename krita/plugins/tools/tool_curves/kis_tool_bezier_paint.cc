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


#include <qpainter.h>
#include <qlayout.h>
#include <qrect.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kis_cmb_composite.h"
#include "kis_colorspace.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_int_spinbox.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_point.h"
#include "kis_tool_controller.h"
#include "kis_tool_paint.h"

#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_canvas_subject.h"

#include "kis_curve_framework.h"
#include "kis_tool_bezier_paint.h"

KisToolBezierPaint::KisToolBezierPaint()
    : super(i18n("Bezier painting tool"))
{
    setName("tool_bezier_paint");
    m_cursor = "tool_bezier_cursor.png";
    setCursor(KisCursor::load(m_cursor, 6, 6));
}

KisToolBezierPaint::~KisToolBezierPaint()
{

}

KisCurve::iterator KisToolBezierPaint::paintPoint (KisPainter& painter, KisCurve::iterator point)
{
    KisCurve::iterator origin,destination,control1,control2;
    switch ((*point).hint()) {
    case BEZIERENDHINT:
        origin = point++;
        control1 = point;
        control2 = control1.nextPivot();
        destination = control2.next();
        if (m_curve->count() > 4 && (*point) != m_curve->last()) {
            point = point.nextPivot().next();
            painter.paintAt((*origin).point(),PRESSURE_DEFAULT,0,0);
            painter.paintBezierCurve((*origin).point(),PRESSURE_DEFAULT,0,0,(*control1).point(),
            (*control2).point(),(*destination).point(),PRESSURE_DEFAULT,0,0,0);
        }
        break;
    default:
        point = super::paintPoint(painter,point);
    }

    return point;
}

void KisToolBezierPaint::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KShortcut(Qt::Key_F9));
        m_action = new KRadioAction(i18n("&Bezier"),
                                    "tool_bezier_paint",
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

#include "kis_tool_bezier_paint.moc"
