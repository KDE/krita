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

#include "kis_tool_bezier_select.h"


#include <QPainter>
#include <QLayout>
#include <QRect>
#include <QLabel>
#include <QPushButton>
#include <QWhatsThis>
#include <QCheckBox>
#include <QPointF>

#include <kaction.h>
#include <kis_debug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kactioncollection.h>

#include "widgets/kis_cmb_composite.h"
#include "kis_config.h"
#include "kis_cursor.h"
//#include "kis_doc2.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_tool_paint.h"

#include "canvas/kis_canvas.h"
#include "kis_canvas_subject.h"

#include "kis_curve_framework.h"


KisToolBezierSelect::KisToolBezierSelect()
        : KisToolBezier(i18n("Bezier Selection Tool"))
{
    setName("tool_bezier_select");
    m_cursor = "tool_bezier_cursor.png";
    setCursor(KisCursor::load(m_cursor, 6, 6));
}

KisToolBezierSelect::~KisToolBezierSelect()
{

}

QVector<QPointF> KisToolBezierSelect::convertCurve()
{
    QVector<QPointF> points;

    for (KisCurve::iterator i = m_curve->begin(); i != m_curve->end(); i++) {
        if (((*i).hint() != BEZIERPREVCONTROLHINT) && ((*i).hint() != BEZIERNEXTCONTROLHINT))
            points.append((*i).point());
    }

    return points;
}

void KisToolBezierSelect::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_bezier_select"),
                               i18n("&Bezier Path"),
                               collection,
                               objectName());
        Q_CHECK_PTR(m_action);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("Select areas of the image with Bezier paths."));
        m_action->setActionGroup(actionGroup());

        m_ownAction = true;
    }
}

#include "kis_tool_bezier_select.moc"
