/*
 *  tool_select_elliptical.cc - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qregion.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_tool_select_elliptical.h"

KisToolSelectElliptical::KisToolSelectElliptical() : super()
{
	m_subject = 0;
}


KisToolSelectElliptical::~KisToolSelectElliptical()
{
}

void KisToolSelectElliptical::setup(KActionCollection *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Elliptical Select"), 
						  "elliptical" , 
						  0, 
						  this, 
						  SLOT(activate()),
						  collection, 
						  "tool_select_elliptical" );

	toggle -> setExclusiveGroup("tools");
}

void KisToolSelectElliptical::draw(const QPoint& start, const QPoint& end, QPaintEvent *e)
{
// 	KisView *view = getCurrentView();
// 	QPainter gc(m_canvas);
// 	QPen pen(Qt::DotLine);
// 	float zF = view -> zoomFactor();

// 	gc.setRasterOp(Qt::NotROP);
// 	gc.setPen(pen);
 
// 	if (e)
// 		gc.setClipRect(e -> rect());
   
// 	gc.drawEllipse(
// 			start.x() + view -> xPaintOffset() - static_cast<int>(zF * view -> xScrollOffset()),
// 			start.y() + view -> yPaintOffset() - static_cast<int>(zF * view -> yScrollOffset()), 
// 			end.x() - start.x(), 
// 			end.y() - start.y());
}

QRegion::RegionType KisToolSelectElliptical::regionType()
{
	return QRegion::Ellipse;
}

void KisToolSelectElliptical::setSelection(const QRect& rc, KisLayer *lay)
{
// 	m_doc -> getSelection() -> setEllipticalSelection(rc, lay);
}

#include "kis_tool_select_elliptical.moc"
