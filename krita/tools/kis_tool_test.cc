/*
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qpainter.h>
#include <qpen.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <koColor.h>

#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_tool_test.h"

KisToolTest::KisToolTest(KisView *view, KisDoc *doc) : super(view, doc)
{
    m_view = view;
    m_doc = doc;
}

KisToolTest::~KisToolTest()
{
}

void KisToolTest::mousePress(QMouseEvent *e)
{
    Q_INT32 x = e->pos().x();
    Q_INT32 y = e->pos().y();

    KisImageSP img = m_view -> currentImg();
    if (img) {
        KisPaintDeviceSP device = img -> activeDevice();
        if (device) {
            KisPainter p( device );
            p.beginTransaction( "Test" );
            p.fillRect( x,  y,  10,  10,  m_color );
            p.endTransaction();
            device->anchor();
         }
        img->invalidate( x,  y,  10,  10 );

    }
    m_view -> updateCanvas(x,  y,  10,  10);
}


void KisToolTest::tabletEvent(QTabletEvent *e)
{
    Q_INT32 x = e->pos().x();
    Q_INT32 y = e->pos().y();

    KisImageSP img = m_view -> currentImg();
    if (img) {
        KisPaintDeviceSP device = img -> activeDevice();
        if (device) {
            if ( e->type() == QEvent::TabletPress ) {
                KisPainter p( device );
                p.beginTransaction( "Test" );
                p.fillRect( x,  y,  e->pressure(),  e->pressure(),  m_color, e->pressure() );
                p.endTransaction();
                device->anchor();
            }
         }
    }
    e->accept();
    m_view -> updateCanvas();
}


void KisToolTest::setup()
{
	KToggleAction *toggle;
	toggle = new KToggleAction(i18n("&Test"), "Test", 0, this,
			SLOT(activateSelf()), m_view -> actionCollection(), "tool_test");
	toggle -> setExclusiveGroup("tools");
}

