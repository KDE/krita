/*
 *  kis_tool_pen.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *                2001 John Califf
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

#include <qbitmap.h>
#include <qpainter.h>

#include <kaction.h>
#include <kdebug.h>

#include "kis_brush.h"
#include "kis_cursor.h"
#include "kis_dlg_toolopts.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_image_cmd.h"
#include "kis_framebuffer.h"
#include "kis_tool_pen.h"
#include "kis_view.h"
#include "kis_vec.h"

PenTool::PenTool(KisDoc *doc, KisCanvas *canvas, KisBrush *brush) : KisTool(doc)
{
	m_dragging = false;
	m_canvas = canvas;
	m_doc = doc;
	m_penColorThreshold = 128;
	m_opacity = 255;
	m_usePattern = false;
	m_useGradient = false;
	m_lineThickness = 1;
	setBrush(brush);
	m_cmd = 0;
}

PenTool::~PenTool()
{
}

void PenTool::setBrush(KisBrush *_brush)
{
    m_brush = _brush;

    int w = m_brush->pixmap().width();
    int h = m_brush->pixmap().height();

    if((w < 33 && h < 33) && (w > 9 && h > 9))
    {
        QBitmap mask(w, h);
        QPixmap pix(m_brush->pixmap());
        mask = pix.createHeuristicMask();
        pix.setMask(mask);
	m_doc -> setCanvasCursor(QCursor(pix));
        m_cursor = QCursor(pix);
    }
    else
    {
	m_doc -> setCanvasCursor(KisCursor::penCursor());
        m_cursor = KisCursor::penCursor();
    }
}

void PenTool::mousePress(QMouseEvent *e)
{
	/* do all status checking on mouse press only!
	   Not needed elsewhere and slows things down if
	   done in mouseMove and Paint routines.  Nothing
	   happens unless mouse is first pressed anyway */

	if (e -> button() == QMouseEvent::LeftButton) {
		KisImageSP img = m_doc -> currentImg();
		KisPaintDeviceSP device;

		if (!img) 
			return;

		device = img -> getCurrentPaintDevice();

		if (!device)
			return;

		if (!device -> visible())
			return;

		if (!m_doc -> frameBuffer())
			return;

		m_dragging = true;

		QPoint pos = zoomed(e -> pos());
		m_dragStart = pos;
		m_dragdist = 0;

		Q_ASSERT(m_cmd == 0);
		m_cmd = new KisImageCmd(i18n("Pen Stroke"), img, device);

		if (paint(pos))
			img -> markDirty(QRect(pos - m_brush -> hotSpot(), m_brush -> size()));
	}
}

bool PenTool::paint(const QPoint& pos)
{
	KisView *view = getCurrentView();
	KisImageSP img = m_doc -> currentImg();
//	KisPaintDeviceSP device = img -> getCurrentPaintDevice();
	KisLayerSP device = img -> getCurrentLayer();
	KisFrameBuffer *m_fb = m_doc -> frameBuffer();
	int startx = (pos - m_brush -> hotSpot()).x();
	int starty = (pos - m_brush -> hotSpot()).y();
	QRect clipRect(startx, starty, m_brush -> width(), m_brush -> height());

	if (!clipRect.intersects(device -> imageExtents()))
		return false;

	clipRect = clipRect.intersect(device -> imageExtents());

	int sx = clipRect.left() - startx;
	int sy = clipRect.top() - starty;
	int ex = clipRect.right() - startx;
	int ey = clipRect.bottom() - starty;
	int red = view -> fgColor().R();
	int green = view -> fgColor().G();
	int blue = view -> fgColor().B();
	bool alpha = img -> colorMode() == cm_RGBA;
	uchar *sl;
	uchar bv;
	uchar r, g, b;

	for (int y = sy; y <= ey; y++) {
		sl = m_brush -> scanline(y);

		for (int x = sx; x <= ex; x++) {
			// no color blending with pen tool (only with brush)
			// alpha blending only (maybe)
			bv = *(sl + x);

			if (bv < m_penColorThreshold) 
				continue;

			r = red;
			g = green;
			b = blue;

			if (!m_usePattern)
				// use foreround color
				device -> setPixel(startx + x, starty + y, alpha ? qRgb(r, g, b) : qRgba(r, g, b, bv), m_cmd);
			else
				// map pattern to pen pixel
				m_fb -> setPatternToPixel(device, startx + x, starty + y, 0);
		}
	}

	return true;
}

void PenTool::mouseMove(QMouseEvent *e)
{
	if(m_dragging) {
		KisImageSP img = m_doc -> currentImg();
		int spacing = m_brush->spacing();

		if (spacing <= 0) 
			spacing = 1;

		QPoint pos = e -> pos();
		int mouseX = e -> x();
		int mouseY = e -> y();

		pos = zoomed(pos);
		mouseX = zoomed(mouseX);
		mouseY = zoomed(mouseY);

		KisVector end(mouseX, mouseY);
		KisVector start(m_dragStart.x(), m_dragStart.y());
		KisVector dragVec = end - start;
		float saved_dist = m_dragdist;
		float new_dist = dragVec.length();
		float dist = saved_dist + new_dist;

		if ((int)dist < spacing) {
			// save for next movevent
			m_dragdist += new_dist;
			m_dragStart = pos;
			return;
		}
		
		m_dragdist = 0;
		dragVec.normalize();
		KisVector step = start;

		while (dist >= spacing) {
			if (saved_dist > 0) {
				step += dragVec * (spacing-saved_dist);
				saved_dist -= spacing;
			}
			else
				step += dragVec * spacing;

			QPoint p(qRound(step.x()), qRound(step.y()));

			if (paint(p))
				img -> markDirty(QRect(p - m_brush -> hotSpot(), m_brush -> size()));

			dist -= spacing;
		}

		//save for next movevent
		if (dist > 0) 
			m_dragdist = dist;

		m_dragStart = pos;
	}
}

void PenTool::mouseRelease(QMouseEvent *e)
{
	if (e -> button() != LeftButton)
		return;

	m_dragging = false;
	Q_ASSERT(m_doc && m_cmd);
	m_doc -> addCommand(m_cmd);
	m_cmd = 0;
}

void PenTool::optionsDialog()
{
	ToolOptsStruct ts;

	ts.usePattern       = m_usePattern;
	ts.useGradient      = m_useGradient;
	ts.penThreshold     = m_penColorThreshold;
	ts.opacity          = m_opacity;

	bool old_usePattern         = m_usePattern;
	bool old_useGradient        = m_useGradient;
	int old_penColorThreshold   = m_penColorThreshold;
	unsigned int old_opacity          = m_opacity;

	ToolOptionsDialog opt_dlg(tt_pentool, ts);

	opt_dlg.exec();

	if (opt_dlg.result() == QDialog::Rejected)
		return;

	m_usePattern          = opt_dlg.penToolTab()->usePattern();
	m_useGradient         = opt_dlg.penToolTab()->useGradient();
	m_penColorThreshold   = opt_dlg.penToolTab()->penThreshold();
	m_opacity          = opt_dlg.penToolTab()->opacity();

	// User change value ?
	if ( old_usePattern != m_usePattern || old_useGradient != m_useGradient
			|| old_penColorThreshold != m_penColorThreshold || old_opacity != m_opacity ) {
		m_doc->setModified( true );
	}
}

void PenTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Pen Tool"), "pencil", 0, this, SLOT(toolSelect()), collection, "tool_pen");

	toggle -> setExclusiveGroup("tools");
}

QDomElement PenTool::saveSettings(QDomDocument& doc) const
{ 
	QDomElement tool = doc.createElement("penTool");

	tool.setAttribute("opacity", m_opacity);
	tool.setAttribute("paintThreshold", m_penColorThreshold);
	tool.setAttribute("paintWithPattern", static_cast<int>(m_usePattern));
	tool.setAttribute("paintWithGradient", static_cast<int>(m_useGradient));
	return tool;
}

bool PenTool::loadSettings(QDomElement& tool)
{
	bool rc = tool.tagName() == "penTool";

	if (rc) {
		kdDebug() << "PenTool::loadSettings\n";
		m_opacity = tool.attribute("opacity").toInt();
		m_penColorThreshold = tool.attribute("paintThreshold").toInt();
		m_usePattern = static_cast<bool>(tool.attribute("paintWithPattern").toInt());
		m_useGradient = static_cast<bool>(tool.attribute("paintWithGradient").toInt());
	}

	return rc;
}

