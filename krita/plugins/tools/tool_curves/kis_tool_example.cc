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
#include <QPointF>

#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kaction.h>
#include <kactioncollection.h>

#include "kis_global.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "KoPoint.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "KoPointerEvent.h"
#include "KoPointerEvent.h"
#include "KoPointerEvent.h"
#include "kis_paintop_registry.h"
#include "kis_cursor.h"
#include "kis_vec.h"
#include "kis_canvas.h"

#include "kis_curve_framework.h"

#include "kis_tool_example.h"


class KisCurveExample : public KisCurve {

    typedef KisCurve super;
    
public:

    KisCurveExample() : super() {}

    ~KisCurveExample() {}

    virtual iterator pushPivot (const QPointF&);

};

KisCurve::iterator KisCurveExample::pushPivot (const QPointF& point)
{
    return selectPivot(iterator(*this,m_curve.insert(m_curve.end(),CurvePoint(point,true,false,LINEHINT))), true);
}

KisToolExample::KisToolExample()
    : super(i18n("Tool for Curves - Example"))
{
    setName("tool_example");
    m_cursor = "tool_example_cursor.png";
    setCursor(KisCursor::load(m_cursor, 6, 6));

    m_curve = new KisCurveExample;
}

KisToolExample::~KisToolExample()
{

}

void KisToolExample::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_example"),
                               i18n("&Example Tool"),
                               collection,
                               objectName());
        Q_CHECK_PTR(m_action);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("This is a test tool for the Curve Framework."));
        m_action->setActionGroup(actionGroup());

        m_ownAction = true;
    }
}

#include "kis_tool_example.moc"
