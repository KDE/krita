/*
 *  tool_select_elliptical.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qpainter.h>
#include <qregion.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_doc.h"
#include "kis_canvas.h"
#include "kis_tool_select_elliptical.h"
#include "kis_view.h"

EllipticalSelectTool::EllipticalSelectTool(KisDoc *doc, KisCanvas *canvas) : super(doc, canvas)
{
}

EllipticalSelectTool::~EllipticalSelectTool()
{
}

void EllipticalSelectTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Elliptical Select"), "elliptical" , 0, this, SLOT(toolSelect()),
			collection, "tool_select_elliptical" );

	toggle -> setExclusiveGroup("tools");
}

void EllipticalSelectTool::draw(const QPoint& start, const QPoint& end, QPaintEvent *e)
{
	KisView *view = getCurrentView();
	QPainter gc(m_canvas);
	QPen pen(Qt::DotLine);
	float zF = view -> zoomFactor();

	gc.setRasterOp(Qt::NotROP);
	gc.setPen(pen);
 
	if (e)
		gc.setClipRect(e -> rect());
   
	gc.drawEllipse(
			start.x() + view -> xPaintOffset() - static_cast<int>(zF * view -> xScrollOffset()),
			start.y() + view -> yPaintOffset() - static_cast<int>(zF * view -> yScrollOffset()), 
			end.x() - start.x(), 
			end.y() - start.y());
}

QRegion::RegionType EllipticalSelectTool::regionType()
{
	return QRegion::Ellipse;
}

void EllipticalSelectTool::setSelection(const QRect& rc, KisLayer *lay)
{
	m_doc -> getSelection() -> setEllipticalSelection(rc, lay);
}

