/*
 *  linetool.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf 
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qpainter.h>

#include <kaction.h>
#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_canvas.h"
#include "kis_tool_line.h"
#include "kis_dlg_toolopts.h"

LineTool::LineTool(KisDoc *doc, KisCanvas *canvas) : super(doc)
{
	m_doc = doc;
	m_dragging = false;
	m_canvas = canvas;
	m_lineThickness = 4;
	m_opacity = 255;
	m_usePattern = false;
	m_useGradient = false;
	m_useRegions = false;
}

LineTool::~LineTool()
{
}

void LineTool::mousePress(QMouseEvent *event)
{
	if (event -> button() == LeftButton) {
		m_dragging = true;
		m_dragStart = event -> pos();
		m_dragEnd = event -> pos();
	}
}

void LineTool::mouseMove(QMouseEvent *event)
{
	if (m_dragging) {
		// erase old line
		draw(m_dragStart, m_dragEnd);
		// get current position
		m_dragEnd = event -> pos();
		// draw line to current position
		draw(m_dragStart, m_dragEnd);
	}
}

void LineTool::mouseRelease(QMouseEvent *event)
{
	if (m_dragging && event -> state() == LeftButton) {
		// erase old line
		draw(m_dragStart, m_dragEnd);
		m_dragging = false;
	}
    
	KisView *view = getCurrentView();
	KisPainter *p = view -> kisPainter();
	
	p -> drawLine(zoomed(m_dragStart.x()), zoomed(m_dragStart.y()), zoomed(m_dragEnd.x()), zoomed(m_dragEnd.y()));
}

void LineTool::draw(const QPoint& start, const QPoint& end)
{
	KisView *view = getCurrentView();
	QPainter gc(m_canvas);
	float zF = view -> zoomFactor();
	QPen pen;

	pen.setWidth(m_lineThickness);
	gc.setPen(pen);    
	gc.setRasterOp(Qt::NotROP);
	gc.drawLine(
			start.x() + view -> xPaintOffset() - static_cast<int>(zF * view -> xScrollOffset()),
			start.y() + view -> yPaintOffset() - static_cast<int>(zF * view -> yScrollOffset()),
			end.x() + view -> xPaintOffset() - static_cast<int>(zF * view -> xScrollOffset()),
			end.y() + view -> yPaintOffset() - static_cast<int>(zF * view -> yScrollOffset()));
}

void LineTool::optionsDialog()
{
	KisView *view = getCurrentView();
	ToolOptsStruct ts;    

	ts.usePattern = m_usePattern;
	ts.useGradient = m_useGradient;
	ts.lineThickness = m_lineThickness;
	ts.opacity = m_opacity;
	ts.fillShapes = m_useRegions;

	bool old_usePattern = m_usePattern;
	bool old_useGradient = m_useGradient;
	int  old_lineThickness = m_lineThickness;
	unsigned int old_opacity = m_opacity;
	bool old_useRegions = m_useRegions;

	ToolOptionsDialog OptsDialog(tt_linetool, ts);

	OptsDialog.exec();

	if(OptsDialog.result() == QDialog::Rejected)
		return;

	m_lineThickness = OptsDialog.lineToolTab() -> thickness();
	m_opacity = OptsDialog.lineToolTab() -> opacity();
	m_usePattern = OptsDialog.lineToolTab() -> usePattern();
	m_useGradient = OptsDialog.lineToolTab() -> useGradient();
	m_useRegions = OptsDialog.lineToolTab() -> solid();

	// User change value ?
	if (old_usePattern != m_usePattern || old_useGradient != m_useGradient || old_opacity != m_opacity || 
			old_lineThickness != m_lineThickness || old_useRegions != m_useRegions ) {    
		KisPainter *p = view -> kisPainter();

		p -> setLineThickness(m_lineThickness);
		p -> setLineOpacity(m_opacity);
		p -> setPatternFill(m_usePattern);
		p -> setGradientFill(m_useGradient);

		// set line tool settings
		m_doc -> setModified(true);
	}
}

void LineTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Line Tool"), "line", 0, this, SLOT(toolSelect()), collection, "tool_line");

	toggle -> setExclusiveGroup("tools");
}

void LineTool::toolSelect()
{
	KisView *view = getCurrentView();

	if (view) {
		KisPainter *gc = view -> kisPainter();

		gc -> setLineThickness(m_lineThickness);
		gc -> setLineOpacity(m_opacity);
		gc -> setPatternFill(m_usePattern);
		gc -> setGradientFill(m_useGradient);
		view -> activateTool(this);
	}
}

QString LineTool::settingsName() const
{
	return "lineTool";
}

QDomElement LineTool::saveSettings(QDomDocument& doc) const
{
	QDomElement lineTool = doc.createElement(settingsName());

	lineTool.setAttribute("thickness", m_lineThickness);
	lineTool.setAttribute("opacity", m_opacity);
	lineTool.setAttribute("fillInteriorRegions", static_cast<int>(m_useRegions));
	lineTool.setAttribute("useCurrentPattern", static_cast<int>(m_usePattern));
	lineTool.setAttribute("fillWithGradient", static_cast<int>(m_useGradient));
	return lineTool;
}

bool LineTool::loadSettings(QDomElement& elem)
{
	bool rc = elem.tagName() == settingsName();

	if (rc) {
		m_lineThickness = elem.attribute("thickness").toInt();
		m_opacity = elem.attribute("opacity").toInt();
		m_useRegions = static_cast<bool>(elem.attribute("fillInteriorRegions").toInt());
		m_usePattern = static_cast<bool>(elem.attribute("useCurrentPattern").toInt());
		m_useGradient = static_cast<bool>(elem.attribute("fillWithGradient").toInt());
	}

	return rc;
}

