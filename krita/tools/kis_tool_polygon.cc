/*
 *  polygontool.cc - part of Krayon
 *
 *  Copyright (c) 2001 Toshitaka Fujioka <fujioka@kde.org>
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

#include <math.h>

#include <qpainter.h>
#include <qpointarray.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_tool_polygon.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

KisToolPolygon::KisToolPolygon() : 
	super()
{
	setName("tool_polygon");
	m_subject = 0;

// 	m_dragging = false;
// 	lineThickness = 4;
// 	m_opacity = 255;
// 	cornersValue = 3;
// 	sharpnessValue = 0;
// 	m_usePattern = false;
// 	m_useGradient = false;
// 	m_useRegions = false;
// 	checkPolygon = true;
// 	checkConcavePolygon = false;
// 	mStart = QPoint(-1, -1);
// 	mFinish = QPoint(-1, -1);     
}

KisToolPolygon::~KisToolPolygon()
{
}

void KisToolPolygon::buttonPress(KisButtonPressEvent */*event*/)
{
// 	if (event -> button() == LeftButton) {
// 		m_dragging = true;
// 		m_dragStart = event -> pos();
// 		m_dragEnd = event -> pos();
// 	}
}

void KisToolPolygon::move(KisMoveEvent */*event*/)
{
// 	if (m_dragging) {
// 		// erase old polygon on canvas
// 		drawPolygon(m_dragStart, m_dragEnd);
// 		// get current mouse position
// 		m_dragEnd = event -> pos();
// 		// draw new polygon on canvas
// 		drawPolygon(m_dragStart, m_dragEnd);
// 	}
}

void KisToolPolygon::buttonRelease(KisButtonReleaseEvent */*event*/)
{
// 	if (m_dragging && event -> state() == LeftButton) {
// 		KisView *view = getCurrentView();

// 		// erase old polygon on canvas
// 		drawPolygon(m_dragStart, m_dragEnd);
// 		m_dragging = false;

// 		// draw final polygon onto layer 
// 		KisPainter *p = view -> kisPainter();
// 		QRect rect = getDrawRect(drawPoints);
// 		QPointArray points = zoomPointArray(drawPoints);
// 		p -> drawPolygon(points, rect);
// 	}
}

void KisToolPolygon::drawPolygon(const KisPoint& /*start*/, const KisPoint& /*end*/)
{
// 	KisView *view = getCurrentView();
// 	QPainter p;
// 	QPen pen;

// 	pen.setWidth(lineThickness);
// 	p.begin(m_canvas);
// 	p.setPen(pen);
// 	p.setRasterOp(Qt::NotROP);

// 	float zF = view -> zoomFactor();
// 	double angle = 2 * M_PI / cornersValue;
// 	float dx = (float) ::fabs(start.x() - end.x());
// 	float dy = (float) ::fabs(start.y() - end.y());
// 	float radius = (dx > dy ? dx / 2.0 : dy / 2.0);
// 	float xoff = start.x() + (start.x() < end.x() ? radius : -radius) + view -> xPaintOffset() - (int)(zF * view -> xScrollOffset());
// 	float yoff = start.y() + (start.y() < end.y() ? radius : -radius) + view -> yPaintOffset() - (int)(zF * view -> yScrollOffset());
// 	float xoff_draw = start.x() + (start.x() < end.x() ? radius : -radius);
// 	float yoff_draw = start.y() + (start.y() < end.y() ? radius : -radius);
// 	QPointArray points(checkConcavePolygon ? cornersValue * 2 : cornersValue);
	
// 	points.setPoint(0, (int) xoff, (int)(-radius + yoff));

// 	// for draw layer
// 	QPointArray m_drawPoints(checkConcavePolygon ? cornersValue * 2 : cornersValue);
	
// 	m_drawPoints.setPoint(0, (int)xoff_draw, (int)(-radius + yoff_draw));

// 	if (checkConcavePolygon) {
// 		angle /= 2.0;
		
// 		double a = angle;
// 		double r = radius - (sharpnessValue / 100.0 * radius);

// 		for (int i = 1; i < cornersValue * 2; ++i) {
// 			double xp, yp;

// 			if (i % 2) {
// 				xp =  r * sin(a);
// 				yp = -r * cos(a);
// 			}
// 			else {
// 				xp = radius * sin(a);
// 				yp = -radius * cos(a);
// 			}

// 			a += angle;
// 			points.setPoint(i, (int)(xp + xoff), (int)(yp + yoff));
// 			m_drawPoints.setPoint(i, (int)(xp + xoff_draw), (int)(yp + yoff_draw));
// 		}
// 	}
// 	else {
// 		double a = angle;

// 		for (int i = 1; i < cornersValue; ++i) {
// 			double xp = radius * sin(a);
// 			double yp = -radius * cos(a);

// 			a += angle;
// 			points.setPoint(i, (int)(xp + xoff), (int)(yp + yoff));
// 			m_drawPoints.setPoint(i, (int)(xp + xoff_draw), (int)(yp + yoff_draw));
// 		}
// 	}

// 	p.drawPolygon(points);
// 	p.end();
// 	drawPoints = m_drawPoints;
}

// /*
//     need options for connecting start and finish points 
//     automatically and for winding mode also
// */
// void KisToolPolygon::optionsDialog()
// {
// 	ToolOptsStruct ts;    
    
// 	ts.usePattern       = m_usePattern;
// 	ts.useGradient      = m_useGradient;
// 	ts.lineThickness    = lineThickness;
// 	ts.opacity      = m_opacity;
// 	ts.fillShapes       = m_useRegions;
// 	ts.polygonCorners   = cornersValue;
// 	ts.polygonSharpness = sharpnessValue;
// 	ts.convexPolygon    = checkPolygon;
// 	ts.concavePolygon   = checkConcavePolygon;

// 	bool old_usePattern       = m_usePattern;
// 	bool old_useGradient      = m_useGradient;
// 	int  old_lineThickness    = lineThickness;
// 	unsigned int  old_opacity      = m_opacity;
// 	bool old_useRegions       = m_useRegions;
// 	int old_cornersValue      = cornersValue;
// 	int old_sharpnessValue    = sharpnessValue;
// 	bool old_checkPolygon      = checkPolygon;
// 	bool old_checkConcavePolygon = checkConcavePolygon;
    
// 	ToolOptionsDialog OptsDialog(tt_polygontool, ts);

// 	OptsDialog.exec();
    
// 	if(OptsDialog.result() == QDialog::Rejected)
// 		return;

// 	lineThickness = OptsDialog.polygonToolTab()->thickness();
// 	m_opacity   = OptsDialog.polygonToolTab()->opacity();
// 	cornersValue  = OptsDialog.polygonToolTab()->corners();
// 	sharpnessValue = OptsDialog.polygonToolTab()->sharpness();
// 	m_usePattern    = OptsDialog.polygonToolTab()->usePattern();
// 	m_useGradient   = OptsDialog.polygonToolTab()->useGradient();
// 	m_useRegions    = OptsDialog.polygonToolTab()->solid();
// 	checkPolygon  = OptsDialog.polygonToolTab()->convexPolygon();
// 	checkConcavePolygon = OptsDialog.polygonToolTab()->concavePolygon();

// 	// User change value ?
// 	if (old_usePattern != m_usePattern || old_useGradient != m_useGradient 
// 			|| old_opacity != m_opacity || old_lineThickness != lineThickness
// 			|| old_useRegions != m_useRegions || old_cornersValue != cornersValue
// 			|| old_sharpnessValue != sharpnessValue || old_checkPolygon != checkPolygon
// 			|| old_checkConcavePolygon != checkConcavePolygon) {    
// 		KisView *view = getCurrentView();
// 		KisPainter *p = view -> kisPainter();

// 		Q_ASSERT(p);
// 		p->setLineThickness(lineThickness);
// 		p->setLineOpacity(m_opacity);
// 		p->setPatternFill(m_usePattern);
// 		p->setGradientFill(m_useGradient);
// 		p->setFilledPolygon(m_useRegions);

// 		// set polygon tool settings
// 		m_doc->setModified(true);
// 	}
// }

void KisToolPolygon::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Polygon"), 
					    "polygon",
					    Qt::Key_Y, 
					    this, 
					    SLOT(activate()),
					    collection, 
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

// void KisToolPolygon::toolSelect()
// {
// 	KisView *view = getCurrentView();

// 	if (view) {
// 		KisPainter *gc = view -> kisPainter();

// 		gc -> setLineThickness(lineThickness);
// 		gc -> setLineOpacity(m_opacity);
// 		gc -> setPatternFill(m_usePattern);
// 		gc -> setGradientFill(m_useGradient);

// 		view -> activateTool(this);
// 	}
// }

// QDomElement KisToolPolygon::saveSettings(QDomDocument& doc) const
// {
// 	// polygon tool element
// 	QDomElement polygonTool = doc.createElement("polygonTool");

// 	polygonTool.setAttribute("thickness", lineThickness);
// 	polygonTool.setAttribute("opacity", m_opacity);
// 	polygonTool.setAttribute("corners", cornersValue);
// 	polygonTool.setAttribute("sharpness", sharpnessValue);
// 	polygonTool.setAttribute("fillInteriorRegions", static_cast<int>(m_useRegions));
// 	polygonTool.setAttribute("useCurrentPattern", static_cast<int>(m_usePattern));
// 	polygonTool.setAttribute("fillWithGradient", static_cast<int>(m_useGradient));
// 	polygonTool.setAttribute("polygon", static_cast<int>(checkPolygon));
// 	polygonTool.setAttribute("concavePolygon", static_cast<int>(checkConcavePolygon));
// 	return polygonTool;
// }

// bool KisToolPolygon::loadSettings(QDomElement& elem)
// {
// 	bool rc = elem.tagName() == "polygonTool";

// 	if (rc) {
// 		lineThickness = elem.attribute("thickness").toInt();
// 		m_opacity = elem.attribute("opacity").toInt();
// 		cornersValue = elem.attribute("corners").toInt();
// 		sharpnessValue = elem.attribute("sharpness").toInt();
// 		m_useRegions = static_cast<bool>(elem.attribute("fillInteriorRegions").toInt());
// 		m_usePattern = static_cast<bool>(elem.attribute("useCurrentPattern").toInt());
// 		m_useGradient = static_cast<bool>(elem.attribute("fillWithGradient").toInt());
// 		checkPolygon = static_cast<bool>(elem.attribute("polygon").toInt());
// 		checkConcavePolygon = static_cast<bool>(elem.attribute("concavePolygon").toInt());
// 	}

// 	return rc;
// }

#include "kis_tool_polygon.moc"
