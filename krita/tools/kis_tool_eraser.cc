/*
 *  kis_tool_eraser.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#include <kaction.h>
#include <kdebug.h>

#include "kis_brush.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_canvas.h"
#include "kis_vec.h"
#include "kis_cursor.h"
#include "kis_util.h"

#include "kis_tool_eraser.h"
#include "kis_dlg_toolopts.h"

EraserTool::EraserTool(KisDoc *doc, KisBrush *_brush) : KisTool(doc)
{
	m_dragging = false;
	m_dragdist = 0;
	m_doc = doc;

	// initialize eraser tool settings
	m_usePattern = false;
	m_useGradient = true;
	m_opacity = 255;

	setBrush(_brush);
}

EraserTool::~EraserTool() 
{
}

void EraserTool::setBrush(KisBrush *_brush)
{
	m_brush = _brush;

	int w = m_brush->pixmap().width();
	int h = m_brush->pixmap().height();

	// cursor cannot be larger than 32x32
	if ((w < 33 && h < 33) && (w > 9 && h > 9)) {
		QBitmap mask(w, h);
		QPixmap pix(m_brush->pixmap());
		mask = pix.createHeuristicMask();
		pix.setMask(mask);
		m_view->kisCanvas()->setCursor(QCursor(pix));
		m_cursor = QCursor(pix);
	}
	else {
		m_view->kisCanvas()->setCursor(KisCursor::eraserCursor());
		m_cursor = KisCursor::eraserCursor();
	}
}

void EraserTool::mousePress(QMouseEvent *e)
{
	KisImage *img = m_doc->current();

	if (!img) 
		return;

	if (!img->getCurrentLayer())
		return;

	if (!img->getCurrentLayer()->visible())
		return;

	if (e->button() != QMouseEvent::LeftButton)
		return;

	m_dragging = true;

	QPoint pos = e->pos();
	pos = zoomed(pos);
	m_dragStart = pos;
	m_dragdist = 0;

	if (paint(pos))
		m_doc->current()->markDirty(QRect(pos - m_brush->hotSpot(), m_brush->size()));
}


bool EraserTool::paint(QPoint pos)
{
	KisImage * img = m_doc->current();
	KisLayer *lay = img->getCurrentLayer();

	if (!img)	return false;
	if (!lay)   return false;
	if (!m_brush) return false;

	// FIXME: Implement this for non-RGB modes.
	if (!img->colorMode() == cm_RGB  && !img->colorMode() == cm_RGBA)
		return false;

	int startx = (pos - m_brush->hotSpot()).x();
	int starty = (pos - m_brush->hotSpot()).y();

	QRect clipRect(startx, starty, m_brush->width(), m_brush->height());

	if (!clipRect.intersects(img->getCurrentLayer()->imageExtents())) {
		kdDebug() << "Does not intersect\n";
		return false;
	}

	clipRect = clipRect.intersect(img->getCurrentLayer()->imageExtents());

	int sx = clipRect.left() - startx;
	int sy = clipRect.top() - starty;
	int ex = clipRect.right() - startx;
	int ey = clipRect.bottom() - starty;

	uchar *sl;
	uchar bv, invbv;

	bool alpha = (img->colorMode() == cm_RGBA);
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
				lay -> setPixel(startx + x, starty + y, qRgba(r, g, b, a));
			}
		}
	}
	else   // no alpha channel -> erase to background color
	{
		kdDebug() << "EraserTool::paint\n";

		int red = m_view->bgColor().R();
		int green = m_view->bgColor().G();
		int blue = m_view->bgColor().B();

		for (int y = sy; y <= ey; y++) {
			sl = m_brush->scanline(y);

			for (int x = sx; x <= ex; x++) {
				rgb = lay -> pixel(startx + x, starty + y);
				r = qRed(rgb);
				g = qGreen(rgb);
				b = qBlue(rgb);

				bv = *(sl + x);

				if (bv == 0) 
					continue;

				invbv = 255 - bv;

				b = ((blue * bv) + (b * invbv))/255;
				g = ((green * bv) + (g * invbv))/255;
				r = ((red * bv) + (r * invbv))/255;
				lay -> setPixel(startx + x, starty + y, qRgb(r, g, b));
			}
		}
	}

	return true;
}


void EraserTool::mouseMove(QMouseEvent *e)
{
	KisImage * img = m_doc->current();
	if (!img) return;

	int spacing = m_brush->spacing();
	if (spacing <= 0) spacing = 1;

	if(m_dragging)
	{
		if( !img->getCurrentLayer()->visible() )
			return;

		QPoint pos = e->pos();
		int mouseX = e->x();
		int mouseY = e->y();

		pos = zoomed(pos);
		mouseX = zoomed(mouseX);
		mouseY = zoomed(mouseY);

		KisVector end(mouseX, mouseY);
		KisVector start(m_dragStart.x(), m_dragStart.y());

		KisVector dragVec = end - start;
		float saved_dist = m_dragdist;
		float new_dist = dragVec.length();
		float dist = saved_dist + new_dist;

		if ((int)dist < spacing)
		{
			// save for next movevent
			m_dragdist += new_dist;
			m_dragStart = pos;
			return;
		}
		else
			m_dragdist = 0;

		dragVec.normalize();
		KisVector step = start;

		while (dist >= spacing)
		{
			if (saved_dist > 0)
			{
				step += dragVec * (spacing-saved_dist);
				saved_dist -= spacing;
			}
			else
				step += dragVec * spacing;

			QPoint p(qRound(step.x()), qRound(step.y()));

			if (paint(p))
				img->markDirty(QRect(p - m_brush->hotSpot(), m_brush->size()));

			dist -= spacing;
		}
		//save for next movevent
		if (dist > 0) m_dragdist = dist;
		m_dragStart = pos;
	}

}

void EraserTool::mouseRelease(QMouseEvent *e)
{
	if (e->button() != LeftButton)
		return;
	m_dragging = false;
}

void EraserTool::optionsDialog()
{
	ToolOptsStruct ts;

	ts.usePattern  = m_usePattern;
	ts.useGradient = m_useGradient;
	ts.opacity     = m_opacity;

	bool old_usePattern     = m_usePattern;
	bool old_useGradient    = m_useGradient;
	unsigned int  old_opacity    = m_opacity;

	ToolOptionsDialog OptsDialog(tt_erasertool, ts);

	OptsDialog.exec();

	if(OptsDialog.result() == QDialog::Rejected)
		return;

	m_opacity   = OptsDialog.eraserToolTab()->opacity();
	m_usePattern    = OptsDialog.eraserToolTab()->usePattern();
	m_useGradient   = OptsDialog.eraserToolTab()->useGradient();

	// User change value ?
	if ( old_usePattern != m_usePattern || old_useGradient != m_useGradient 
			|| old_opacity != m_opacity ) {
		// set eraser tool settings
		m_doc->setModified( true );
	}
}

void EraserTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Eraser tool"), "eraser", 0, this, SLOT(toolSelect()), collection, "tool_eraser");

	toggle -> setExclusiveGroup("tools");
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

