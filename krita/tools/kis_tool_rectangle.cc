/*
 *  kis_tool_rectangle.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
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

#include <kaction.h>
#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_color.h"
#include "kis_canvas.h"
#include "kis_tool_rectangle.h"
#include "kis_dlg_toolopts.h"

RectangleTool::RectangleTool(KisDoc *doc, KisCanvas *canvas) : KisTool(doc)
{
	m_doc = doc;
	m_dragging = false;

	// initialize rectangle tool settings
	m_lineThickness = 4;
	m_opacity = 255;
	m_usePattern = false;
	m_useGradient = false;
	m_fillSolid = false;
	m_canvas = canvas;
}

RectangleTool::~RectangleTool()
{
}

void RectangleTool::mousePress(QMouseEvent *event)
{
	if (event -> button() == LeftButton) {
		m_dragging = true;
		m_dragStart = event -> pos();
		m_dragEnd = event -> pos();
	}
}

void RectangleTool::mouseMove(QMouseEvent *event)
{
	if (m_dragging) {
		// erase old lines on canvas
		drawRectangle(m_dragStart, m_dragEnd);
		// get current mouse position
		m_dragEnd = event -> pos();
		// draw new lines on canvas
		drawRectangle(m_dragStart, m_dragEnd);
	}
}

void RectangleTool::mouseRelease(QMouseEvent *event)
{
	if (m_dragging && event -> state() == LeftButton) {
		// erase old lines on canvas
		drawRectangle(m_dragStart, m_dragEnd);
		m_dragging = false;
	}
    
	// get topLeft and bottomRight.
	int maxX = 0, maxY = 0;
	int minX = 0, minY = 0;

	if (m_dragStart.x() > m_dragEnd.x()) {
		maxX = m_dragStart.x();
		minX = m_dragEnd.x();
	}
	else {
		maxX = m_dragEnd.x();
		minX = m_dragStart.x();
	}

	if (m_dragStart.y() > m_dragEnd.y()) {
		maxY = m_dragStart.y();
		minY = m_dragEnd.y();
	}
	else {
		maxY = m_dragEnd.y();
		minY = m_dragStart.y();
	}

	QPoint topLeft = QPoint(minX, minY);
	QPoint bottomRight = QPoint(maxX, maxY);
	m_final_lines = QRect(zoomed(topLeft), zoomed(bottomRight));

	// draw final lines onto layer
	KisPainter *p = m_view -> kisPainter();
	p -> drawRectangle(m_final_lines);
}

void RectangleTool::drawRectangle(const QPoint& start, const QPoint& end )
{
    QPainter p;
    QPen pen;
    pen.setWidth(m_lineThickness);
    
    p.begin(m_canvas);
    p.setPen(pen);
    p.setRasterOp( Qt::NotROP );
    float zF = m_view->zoomFactor();
    p.drawRect( QRect(start.x() + m_view->xPaintOffset() 
                                - (int)(zF * m_view->xScrollOffset()),
                      start.y() + m_view->yPaintOffset() 
                                - (int)(zF * m_view->yScrollOffset()), 
                      end.x() - start.x(), 
                      end.y() - start.y()) );
    p.end();
}

void RectangleTool::optionsDialog()
{
	ToolOptsStruct ts;    

	ts.usePattern = m_usePattern;
	ts.useGradient = m_useGradient;
	ts.lineThickness = m_lineThickness;
	ts.opacity = m_opacity;
	ts.fillShapes = m_fillSolid;

	bool old_usePattern = m_usePattern;
	bool old_useGradient = m_useGradient;
	int  old_lineThickness = m_lineThickness;
	unsigned int  old_opacity = m_opacity;
	bool old_fillSolid = m_fillSolid;

	ToolOptionsDialog OptsDialog(tt_linetool, ts);

	OptsDialog.exec();
    
	if (OptsDialog.result() == QDialog::Rejected)
		return;

	m_lineThickness = OptsDialog.lineToolTab()->thickness();
	m_opacity   = OptsDialog.lineToolTab()->opacity();
	m_usePattern    = OptsDialog.lineToolTab()->usePattern();
	m_useGradient   = OptsDialog.lineToolTab()->useGradient();
	m_fillSolid     = OptsDialog.lineToolTab()->solid();  

	// User change value ?
	if ( old_usePattern != m_usePattern || old_useGradient != m_useGradient 
			|| old_opacity != m_opacity || old_lineThickness != m_lineThickness
			|| old_fillSolid != m_fillSolid) {    
		KisPainter *p = m_view->kisPainter();

		p->setLineThickness(m_lineThickness);
		p->setLineOpacity(m_opacity);
		p->setFilledRectangle(m_fillSolid);
		p->setPatternFill(m_usePattern);
		p->setGradientFill(m_useGradient);

		// set rectangle tool settings
		m_doc->setModified( true );
	}
}

void RectangleTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Rectangle tool"), "rectangle", 0, this, SLOT(toolSelect()), collection, "tool_rectangle");

	toggle -> setExclusiveGroup("tools");
}

void RectangleTool::toolSelect()
{
	if (m_view) {
		KisPainter *gc = m_view -> kisPainter();

		gc -> setLineThickness(m_lineThickness);
		gc -> setLineOpacity(m_opacity);
		gc -> setFilledRectangle(m_fillSolid);
		gc -> setGradientFill(m_useGradient);
		gc -> setPatternFill(m_usePattern);

		m_view -> activateTool(this);
	}
}

QDomElement RectangleTool::saveSettings(QDomDocument& doc) const
{
	// rectangle tool element
	QDomElement rectangleTool = doc.createElement("rectangleTool");

	rectangleTool.setAttribute("thickness", m_lineThickness);
	rectangleTool.setAttribute("opacity", m_opacity);
	rectangleTool.setAttribute("fillInteriorRegions", static_cast<int>(m_fillSolid));
	rectangleTool.setAttribute("useCurrentPattern", static_cast<int>(m_usePattern));
	rectangleTool.setAttribute("fillWithGradient", static_cast<int>(m_useGradient));
	return rectangleTool;

}

bool RectangleTool::loadSettings(QDomElement& elem)
{
	bool rc = elem.tagName() == "rectangleTool";

	if (rc) {
		m_lineThickness = elem.attribute("thickness").toInt();
		m_opacity = elem.attribute("opacity").toInt();
		m_fillSolid = static_cast<bool>(elem.attribute("fillInteriorRegions").toInt());
		m_usePattern = static_cast<bool>(elem.attribute("useCurrentPattern").toInt());
		m_useGradient = static_cast<bool>(elem.attribute("fillWithGradient").toInt());
	}

	return rc;
}

