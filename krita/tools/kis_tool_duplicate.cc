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
#include <qpainter.h>

#include <kaction.h>
#include <kdebug.h>
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

KisToolDuplicate::KisToolDuplicate() 
    : super(i18n("Duplicate")), m_isOffsetNotUptodate(true), m_position(QPoint(-1,-1))
{
    setName("tool_duplicate");
    m_subject = 0;
    setCursor(KisCursor::penCursor());
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
    if (e -> button() == RightButton) {
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
    m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Duplicate"),
                        "stamp", Qt::Key_C, this,
                        SLOT(activate()), collection,
                        name());
        m_action -> setToolTip(i18n("Duplicate parts of an image"));
        m_action -> setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

void KisToolDuplicate::initPaint(KisEvent *e)
{
    if( m_position != QPoint(-1,-1))
    {
        if(m_isOffsetNotUptodate)
        {
            m_offset = e -> pos() - m_position;
            m_isOffsetNotUptodate = false;
        }
        setUseTempLayer( true );
        super::initPaint(e);
        painter() -> setDuplicateOffset( m_offset );
        KisPaintOp * op = KisPaintOpRegistry::instance() -> paintOp("duplicate", painter());
        op -> setSource(m_source);
        painter() -> setPaintOp(op);
    }
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
        painter() -> paintAt( pos, pressure, xtilt, ytilt);
    }
}

#include "kis_tool_duplicate.moc"
