/*
 *  kis_tool_brush.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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
#include <qcolor.h>
#include <qwaitcondition.h>

#include <kaction.h>
#include <kapplication.h>

#include "kis_brush.h"
#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_dlg_toolopts.h"
#include "kis_image.h"
#include "kis_image_cmd.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_tool_brush.h"
#include "kis_undo.h"
#include "kis_util.h"
#include "kis_view.h"
#include "kis_vec.h"

BrushTool::BrushTool(KisDoc *doc, KisBrush *brush) : super(doc)
{
	m_doc = doc;
	m_dragging = false;
	m_dragdist = 0;
	m_usePattern = false;
	m_useGradient = false;
	m_opacity = CHANNEL_MAX;
	m_cmd = 0;
	setBrush(brush);
}

BrushTool::~BrushTool() 
{
}

void BrushTool::setBrush(KisBrush *brush)
{
	m_brush = brush;
	m_brushWidth = m_brush -> pixmap().width();
	m_brushHeight = m_brush -> pixmap().height();
	m_hotSpot  = m_brush -> hotSpot();
	m_hotSpotX = m_brush -> hotSpot().x();
	m_hotSpotY = m_brush -> hotSpot().y();
	m_brushSize = QSize(m_brushWidth, m_brushHeight);

	// make custom cursor from brush pixmap
	// if brush pixmap is of reasonable size
	if (m_brushWidth < 33 && m_brushHeight < 33 && m_brushWidth > 9 && m_brushHeight > 9) {
		QBitmap mask(m_brushWidth, m_brushHeight);
		QPixmap pix(m_brush -> pixmap());

		mask = pix.createHeuristicMask();
		pix.setMask(mask);
		m_doc -> setCanvasCursor(QCursor(pix));
		m_cursor = QCursor(pix);
	}
	// use default brush cursor
	else {
		m_doc -> setCanvasCursor(KisCursor::brushCursor());
		m_cursor = defaultCursor();
	}
}

void BrushTool::mousePress(QMouseEvent *e)
{
	KisView *view = getCurrentView();
	KisImageSP img;
	KisPaintDeviceSP device; 
	QPoint pos = zoomed(e -> pos());

	if (e -> button() != QMouseEvent::LeftButton)
		return;

	if (!(img = m_doc -> currentImg()))
		return;

	if (!(device = img -> getCurrentPaintDevice()))
		return;

	if (!device -> visible())
		return;

	m_red = view -> fgColor().R();
	m_green = view -> fgColor().G();
	m_blue = view -> fgColor().B();
	m_alpha = img -> colorMode() == cm_RGBA;
	m_spacing = m_brush -> spacing();

	if (m_spacing <= 0) 
		m_spacing = 3;

	m_dragging = true;
	m_dragStart = pos;
	m_dragdist = 0;
	Q_ASSERT(m_cmd == 0);
	m_cmd = new KisImageCmd(i18n("Paint"), img, device);

	if (paint(pos)) {
		QRect rc(pos - m_hotSpot, m_brushSize);

		img -> markDirty(rc);
	}
	else {
		delete m_cmd;
		m_cmd = 0;
	}
}

bool BrushTool::paintCanvas(const QPoint& /* pos */)
{
	return true;
}

bool BrushTool::paint(const QPoint& pos)
{
	KisImageSP img = m_doc -> currentImg();
	KisPaintDeviceSP device = img -> getCurrentPaintDevice();
	int startx = pos.x() - m_hotSpotX;
	int starty = pos.y() - m_hotSpotY;
	QRect clipRect(startx, starty, m_brushWidth, m_brushHeight);

	if (!clipRect.intersects(device -> imageExtents()))
		return false;

	clipRect = clipRect.intersect(device -> imageExtents());

	int sx = clipRect.left() - startx;
	int sy = clipRect.top() - starty;
	int ex = clipRect.right() - startx;
	int ey = clipRect.bottom() - starty;
	uchar opacity = m_opacity;
	uchar invopacity = (CHANNEL_MAX - m_opacity);

	uchar bv, invbv;
	int bpp = device -> bpp();
	uchar src2[MAX_CHANNELS];
	uchar dst[MAX_CHANNELS];

	src2[PIXEL_RED] = m_red;
	src2[PIXEL_GREEN] = m_green;
	src2[PIXEL_BLUE] = m_blue;
	src2[PIXEL_ALPHA] = CHANNEL_MAX;
	
	bool alpha = img -> colorMode() == cm_RGBA;

	for (int y = sy; y <= ey; y++) {
		uchar *sl = m_brush -> scanline(y);

		for (int x = sx; x <= ex; x++) {
			uchar *src = device -> pixel(startx + x, starty + y);
			bv = *(sl + x);

			if (bv == 0)
				continue;

			invbv = CHANNEL_MAX - bv;

			for (int w = 0; w < bpp; w++) {
				dst[w] = (src2[w] * bv + src[w] * invbv) / CHANNEL_MAX;

				if (alpha)
					dst[w] = (dst[w] * opacity + src[w] * invopacity) / CHANNEL_MAX;
			}

			device -> setPixel(startx + x, starty + y, dst, m_cmd);
		}
	}

	return true;
}

void BrushTool::mouseMove(QMouseEvent *e)
{
	if (!m_dragging)
		return;

	KisImageSP img = m_doc -> currentImg();
	KisPaintDeviceSP device; 

	if (!img) 
		return;

	device = img -> getCurrentPaintDevice();

	if (!device || !device -> visible())
		return;

	QPoint pos = zoomed(e -> pos());
	KisVector end(pos.x(), pos.y());
	KisVector start(m_dragStart.x(), m_dragStart.y());
	KisVector dragVec = end - start;
	float saved_dist = m_dragdist;
	float new_dist = dragVec.length();
	float dist = saved_dist + new_dist;

	if (static_cast<int>(dist) < m_spacing) {
		m_dragdist += new_dist;
		m_dragStart = pos;
		return;
	}
		
	m_dragdist = 0;
	dragVec.normalize();
	KisVector step = start;

	while (dist >= m_spacing) {
		if (saved_dist > 0) {
			step += dragVec * (m_spacing - saved_dist);
			saved_dist -= m_spacing;
		}
		else
			step += dragVec * m_spacing;

		QPoint p(qRound(step.x()), qRound(step.y()));

		if (paint(p)) {
			QRect rc(p - m_hotSpot, m_brushSize);

			img -> markDirty(rc);
		}

		dist -= m_spacing;
	}

	if (dist > 0) 
		m_dragdist = dist;

	m_dragStart = pos;
}

void BrushTool::mouseRelease(QMouseEvent *e)
{
	if (e -> button() != LeftButton)
		return;

	KisImageSP img = m_doc -> currentImg();
	KisPaintDeviceSP device; 

	if (!img) 
		return;

	device = img -> getCurrentPaintDevice();

	if (device && device -> visible()) {
		m_dragging = false;

		if (m_cmd) {
			m_doc -> addCommand(m_cmd);
			m_cmd = 0;
		}
	}
}

bool BrushTool::paintColor(const QPoint& /*pos*/)
{
	return true;
}

void BrushTool::optionsDialog()
{
	ToolOptsStruct ts;

	ts.usePattern = m_usePattern;
	ts.useGradient = m_useGradient;
	ts.opacity = m_opacity;

	bool old_usePattern = m_usePattern;
	bool old_useGradient = m_useGradient;
	unsigned int old_opacity = m_opacity;

	ToolOptionsDialog OptsDialog(tt_brushtool, ts);

	OptsDialog.exec();

	if (OptsDialog.result() == QDialog::Rejected)
		return;

	m_opacity = OptsDialog.brushToolTab()->opacity();
	m_usePattern = OptsDialog.brushToolTab()->usePattern();
	m_useGradient = OptsDialog.brushToolTab()->useGradient();

	if (old_usePattern != m_usePattern || old_useGradient != m_useGradient || old_opacity != m_opacity)
		m_doc -> setModified(true);
}

void BrushTool::setupAction(QObject *collection)
{
	m_toggle = new KToggleAction(i18n("&Brush tool"), "paintbrush", 0, this, SLOT(toolSelect()), collection, "tool_brush");
	m_toggle -> setExclusiveGroup("tools");
}

QDomElement BrushTool::saveSettings(QDomDocument& doc) const
{
	QDomElement tool = doc.createElement("brushTool");

	tool.setAttribute("opacity", m_opacity);
	tool.setAttribute("blendWithCurrentGradient", static_cast<int>(m_useGradient));
	tool.setAttribute("blendWithCurrentPattern", static_cast<int>(m_usePattern));
	return tool;
}

bool BrushTool::loadSettings(QDomElement& elem)
{
	bool rc = elem.tagName() == "brushTool";

	if (rc) {
		m_opacity = elem.attribute("opacity").toInt();
		m_useGradient = static_cast<bool>(elem.attribute("blendWithCurrentGradient").toInt());
		m_usePattern = static_cast<bool>(elem.attribute("blendWithCurrentPattern").toInt());
	}

	return rc;
}

void BrushTool::toolSelect()
{
	KisView *view = getCurrentView();

	if (view)
		view -> activateTool(this);

	if (m_toggle)
		m_toggle -> setChecked(true);
}

QCursor BrushTool::defaultCursor() const
{
	return KisCursor::brushCursor();
}

