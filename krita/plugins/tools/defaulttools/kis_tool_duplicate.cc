/*
 *  kis_tool_duplicate.cc - part of Krita
 *
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include <qbitmap.h>
#include <qcheckbox.h>
#include <qpainter.h>
#include <qlabel.h>

#include <kaction.h>
#include <kdebug.h>
#include <knuminput.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_tool_duplicate.h"
#include "kis_painter.h"
#include "kis_vec.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_canvas_subject.h"

#include "kis_canvas_painter.h"
#include "kis_boundary_painter.h"

KisToolDuplicate::KisToolDuplicate() 
    : super(i18n("Duplicate Brush")), m_isOffsetNotUptodate(true), m_position(QPoint(-1,-1))
{
    setName("tool_duplicate");
    m_subject = 0;
    setCursor(KisCursor::load("tool_duplicate_cursor.png", 5, 5));
}

KisToolDuplicate::~KisToolDuplicate()
{
}

void KisToolDuplicate::activate()
{
    m_position = QPoint(-1,-1);
    super::activate();
}

void KisToolDuplicate::buttonPress(KisButtonPressEvent *e)
{
    if (e->state() == ShiftButton) {
        m_position = e->pos();
        m_isOffsetNotUptodate = true;
    } else {
        if (m_position != QPoint(-1, -1)) {
            super::buttonPress(e);
        }
    }
}


void KisToolDuplicate::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Duplicate Brush"),
                        "stamp", Qt::Key_C, this,
                        SLOT(activate()), collection,
                        name());
        m_action->setToolTip(i18n("Duplicate parts of the image. Shift-click to select the point to duplicate from to begin."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

void KisToolDuplicate::initPaint(KisEvent *e)
{
    if( m_position != QPoint(-1,-1))
    {
        if(m_isOffsetNotUptodate)
        {
            m_offset = e->pos() - m_position;
            m_isOffsetNotUptodate = false;
        }
        m_paintIncremental = false;
        super::initPaint(e);
        painter()->setDuplicateOffset( m_offset );
        KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("duplicate", 0, painter());
        if (op && m_source) {
            op->setSource(m_source);
            painter()->setPaintOp(op);
        }
    }
}

void KisToolDuplicate::move(KisMoveEvent *e)
{
    super::move(e);

    // Paint the outline where we will (or are) copying from
    if( m_position == QPoint(-1,-1) )
        return;

    QPoint srcPos;
    if (m_mode == PAINT) {
        srcPos = painter()->duplicateOffset().floorQPoint();
    } else {
        if(m_isOffsetNotUptodate)
            srcPos = e->pos().floorQPoint() - m_position.floorQPoint();
        else
            srcPos = m_offset.floorQPoint();
    }

    Q_INT32 x;
    Q_INT32 y;

    // like KisPaintOp::splitCoordinate
    x = (Q_INT32)((e->x() < 0) ? e->x() - 1 : e->x());
    y = (Q_INT32)((e->y() < 0) ? e->y() - 1 : e->y());
    srcPos = QPoint(x - srcPos.x(), y - srcPos.y());

    paintOutline(srcPos);
}

void KisToolDuplicate::paintAt(const KisPoint &pos,
                   const double pressure,
                   const double xtilt,
                   const double ytilt)
{
    if( m_position != QPoint(-1,-1))
    {
        if(m_isOffsetNotUptodate)
        {
            m_offset = pos - m_position;
            m_isOffsetNotUptodate = false;
        }
        painter()->setDuplicateHealing( m_healing->isChecked() );
        painter()->setDuplicateHealingRadius( m_healingRadius->value() );
        painter()->setDuplicatePerspectiveCorrection( m_perspectiveCorrection->isChecked() );
        painter()->paintAt( pos, pressure, xtilt, ytilt);
    }
}

QString KisToolDuplicate::quickHelp() const {
    return i18n("To start, shift-click on the place you want to duplicate from. Then you can start painting. An indication of where you are copying from will be displayed while drawing and moving the mouse.");
}

QWidget* KisToolDuplicate::createOptionWidget(QWidget* parent)
{
    QWidget* widget = KisToolPaint::createOptionWidget(parent);
    m_healing = new QCheckBox(widget);
    m_healing->setChecked( false);
    addOptionWidgetOption(m_healing, new QLabel(i18n("Healing"), widget ));
    m_healingRadius = new KIntNumInput(widget);
    
    KisBrush *brush = m_subject->currentBrush();
    int healingradius = 20;
    if( brush )
    {
        healingradius = 2 * QMAX(brush->width(),brush->height());
    }
    
    m_healingRadius->setValue( healingradius );
    addOptionWidgetOption(m_healingRadius, new QLabel(i18n("Healing radius"), widget ));
    m_perspectiveCorrection =  new QCheckBox(widget);
    addOptionWidgetOption(m_perspectiveCorrection, new QLabel(i18n("Correct the perspective"), widget ));
    return widget;
}

#include "kis_tool_duplicate.moc"
