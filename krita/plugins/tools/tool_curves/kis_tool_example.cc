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

    virtual iterator pushPivot (const KisPoint&);

};

KisCurve::iterator KisCurveExample::pushPivot (const KisPoint& point)
{
    return selectPivot(iterator(*this,m_curve.append(CurvePoint(point,true,false,LINEHINT))), true);
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

        m_action->setToolTip(i18n("This is a test tool for the Curve Framework."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_example.moc"
