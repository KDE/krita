/*
 *  kis_tool_eraser.cc - part of Krayon
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <kaction.h>

#include "kis_brush.h"
#include "kis_doc.h"
#include "kis_vec.h"
#include "kis_dlg_toolopts.h"
#include "kis_image_cmd.h"
#include "kis_tool_brush.h"
#include "kis_tool_eraser.h"

EraserTool::EraserTool(KisDoc *doc, KisBrush *brush) : super(doc, brush)
{
}

EraserTool::~EraserTool() 
{
}

void EraserTool::mousePress(QMouseEvent *e)
{
	if (e -> button() == QMouseEvent::LeftButton) {
		QPoint pos = zoomed(e -> pos());
		KisImageSP img = m_doc -> currentImg();
		KisPaintDeviceSP device;

		if (!img) 
			return;

		device = img -> getCurrentPaintDevice();

		if (!device || !device -> visible())
			return;

		if ((m_spacing = m_brush -> spacing()) <= 0) 
			m_spacing = 3;

		m_dragging = true;
		m_dragStart = pos;
		m_dragdist = 0;
		Q_ASSERT(m_cmd == 0);
		m_cmd = new KisImageCmd(i18n("Erase"), img, device);

		if (paint(pos))
			img -> markDirty(QRect(pos - m_brush->hotSpot(), m_brush->size()));
	}
}

bool EraserTool::paint(const QPoint& pos)
{
	if (!m_brush) 
		return false;

	KisImageSP img = m_doc -> currentImg();

	if (!img)	
		return false;
	
	KisLayerSP lay = img -> getCurrentLayer();

	if (!lay)   
		return false;

	if (!img -> colorMode() == cm_RGB && !img -> colorMode() == cm_RGBA)
		return false;

	int startx = (pos - m_brush -> hotSpot()).x();
	int starty = (pos - m_brush -> hotSpot()).y();

	QRect clipRect(startx, starty, m_brush -> width(), m_brush -> height());

	if (!clipRect.intersects(lay -> imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay -> imageExtents());

	int sx = clipRect.left() - startx;
	int sy = clipRect.top() - starty;
	int ex = clipRect.right() - startx;
	int ey = clipRect.bottom() - starty;

	uchar *sl;
	uchar bv, invbv;

	bool alpha = img -> colorMode() == cm_RGBA;
	QRgb rgb;
	uchar r, g, b, a;

	if (alpha) {
		uchar a;
		int   v;

		for (int y = sy; y <= ey; y++) {
			sl = m_brush -> scanline(y);

			for (int x = sx; x <= ex; x++) {
				bv = *(sl + x);

				if (bv == 0) 
					continue;

				rgb = lay -> pixel(startx + x, starty + y);
				r = qRed(rgb);
				g = qGreen(rgb);
				b = qBlue(rgb);
				a = qAlpha(rgb);
				v = a - bv;

				if (v < 0) 
					v = 0;

				if (v > 255) 
					v = 255;

				a = (uchar)v;
				lay -> setPixel(startx + x, starty + y, qRgba(r, g, b, a), m_cmd);
			}
		}
	}
	else {   // no alpha channel -> erase to background color
		KisView *view = getCurrentView();
		int red = view -> bgColor().R();
		int green = view -> bgColor().G();
		int blue = view -> bgColor().B();

		for (int y = sy; y <= ey; y++) {
			sl = m_brush -> scanline(y);

			for (int x = sx; x <= ex; x++) {
				rgb = lay -> pixel(startx + x, starty + y);
				r = qRed(rgb);
				g = qGreen(rgb);
				b = qBlue(rgb);

				bv = *(sl + x);

				if (bv == 0) 
					continue;

				invbv = 255 - bv;

				b = ((blue * bv) + (b * invbv)) / CHANNEL_MAX;
				g = ((green * bv) + (g * invbv)) / CHANNEL_MAX;
				r = ((red * bv) + (r * invbv)) / CHANNEL_MAX;
				lay -> setPixel(startx + x, starty + y, qRgb(r, g, b), m_cmd);
			}
		}
	}

	return true;
}

QDomElement EraserTool::saveSettings(QDomDocument& doc) const
{
	QDomElement eraserTool = doc.createElement("eraserTool");

	eraserTool.setAttribute("opacity", m_opacity);
	eraserTool.setAttribute("blendWithCurrentGradient", static_cast<int>(m_useGradient));
	eraserTool.setAttribute("blendWithCurrentPattern", static_cast<int>(m_usePattern));
	return eraserTool;
}

bool EraserTool::loadSettings(QDomElement& elem)
{
	bool rc = elem.tagName() == "eraserTool";

	if (rc) {
		m_opacity = elem.attribute("opacity").toInt();
		m_useGradient = static_cast<bool>(elem.attribute("blendWithCurrentGradient").toInt());
		m_usePattern = static_cast<bool>(elem.attribute("blendWithCurrentPattern" ).toInt());
	}

	return rc;
}

void EraserTool::setupAction(QObject *collection)
{
	m_toggle = new KToggleAction(i18n("&Eraser Tool"), "eraser", 0, this, SLOT(toolSelect()), collection, "tool_eraser");
	m_toggle -> setExclusiveGroup("tools");
}

QCursor EraserTool::defaultCursor() const
{
	return KisCursor::eraserCursor();
}

