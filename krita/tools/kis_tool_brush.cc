/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_view.h"
#include "kis_tool_brush.h"
#include "integerwidget.h"
#include "kis_cmb_composite.h"
#include "kis_brush.h"


KisToolBrush::KisToolBrush()
        : super()
{

	setCursor(KisCursor::brushCursor());
}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::paintAt(const QPoint &pos,
		     const double pressure,
		     const double xTilt,
		     const double yTilt)
{
	painter()->paintAt(pos, pressure, xTilt, yTilt);
}

void KisToolBrush::paintLine(const QPoint & pos1,
			     const QPoint & pos2,
			     const double pressure,
			     const double xtilt,
			     const double ytilt)
{
	if (!currentImage() -> activeDevice()) return;

	m_dragDist = painter() -> paintLine(PAINTOP_BRUSH, pos1, pos2, pressure, xtilt, ytilt, m_dragDist);
	currentImage() -> notify( painter() -> dirtyRect() );
	m_dragStart = pos2;
}



void KisToolBrush::setup(KActionCollection *collection)
{
	kdDebug() << "KisToolBrush::setup" << endl;
	KRadioAction *radio = new KRadioAction(i18n("&Brush"),
					       "paintbrush", 0, this,
					       SLOT(activate()), collection,
					       "tool_brush");
	radio -> setExclusiveGroup("tools");
}


#include "kis_tool_brush.moc"

