/*
 *  kis_tool_ellipse.cc - part of Krayon
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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_tool_ellipse.h"
#include "kis_dlg_toolopts.h"

KisToolEllipse::KisToolEllipse() :
     super()
{
	m_subject = 0;
	
	m_dragging = false;

	// initialize ellipse tool settings
	m_lineThickness = 4;
// 	m_opacity = 255;
// 	m_usePattern = false;
// 	m_useGradient = false;
// 	m_fillSolid = false;
}

KisToolEllipse::~KisToolEllipse()
{
}

void KisToolEllipse::draw(const QPoint& start, const QPoint& end)
{
// 	KisView *view = getCurrentView();
// 	QPainter p;
// 	QPen pen;

// 	pen.setWidth(m_lineThickness);
// 	p.begin(m_canvas);
// 	p.setPen(pen);
// 	p.setRasterOp(Qt::NotROP);
// 	float zF = view -> zoomFactor();

// 	p.drawEllipse( 
// 			QRect(start.x() + view->xPaintOffset() - (int)(zF * view->xScrollOffset()),
// 				start.y() + view->yPaintOffset() - (int)(zF * view->yScrollOffset()), 
// 				end.x() - start.x(), 
// 				end.y() - start.y()));
// 	p.end();
}

void KisToolEllipse::draw(KisPainter *gc, const QRect& rc)
{
// 	gc -> drawEllipse(rc);
}

void KisToolEllipse::setup(KActionCollection *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Ellipse Tool"), 
						  "ellipse", 
						  0, 
						  this, 
						  SLOT(activate()), 
						  collection, 
						  "tool_ellipse");

	toggle -> setExclusiveGroup("tools");
}

QString KisToolEllipse::settingsName() const
{
	return "ellipseTool";
}

#include "kis_tool_ellipse.moc"
