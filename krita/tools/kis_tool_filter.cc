/*
 *  kis_tool_filter.cc - part of Krita
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qbitmap.h>
#include <qpainter.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_filter_registry.h"
#include "kis_brush.h"
#include "kis_view.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_tool_filter.h"
#include "kis_painter.h"
#include "kis_vec.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

KisToolFilter::KisToolFilter(KisView* view) 
	: super(i18n("Filter tool")), m_view(view)
{
	setName("tool_filter");
	m_subject = 0;
	setCursor(KisCursor::penCursor());
}

KisToolFilter::~KisToolFilter()
{
}

void KisToolFilter::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Filter"),
					    "filter", 0, this,
					    SLOT(activate()), collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

void KisToolFilter::paintLine(const KisPoint & pos1,
				 const double pressure1,
				 const double xtilt1,
				 const double ytilt1,
				 const KisPoint & pos2,
				 const double pressure2,
				 const double xtilt2,
				 const double ytilt2)
{
	painter()->setFilter( m_view->filterRegistry()->get( "Invert" ) );
	m_dragDist = painter() -> paintLine(PAINTOP_FILTER, pos1, pressure1, xtilt1, ytilt1, pos2, pressure2, xtilt2, ytilt2, m_dragDist);
}

void KisToolFilter::paintAt(const KisPoint &pos,
			       const double pressure,
			       const double xtilt,
			       const double ytilt)
{
	painter()->setFilter( m_view->filterRegistry()->get( "Invert" ) );
	painter() -> filterAt( pos, pressure, xtilt, ytilt);
}
