/*
 *  kis_tool_airbrush.cc - part of Krayon
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>

#include "kis_brush.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_vec.h"
#include "kis_cursor.h"
#include "kis_util.h"
#include "kis_tool_airbrush.h"
#include "kis_dlg_toolopts.h"
#include "kis_canvas.h"

AirBrushTool::AirBrushTool(KisDoc *doc, KisBrush *brush) : KisTool(doc)
{
	m_dragging = false;
	m_cursor = KisCursor::airbrushCursor();
	m_dragdist = 0;
	density = 64;
	m_doc = doc;

	// initialize airbrush tool settings
	m_opacity = 255;
	m_usePattern = false;
	m_useGradient = false;

	setBrush(brush);

	pos.setX(-1);
	pos.setY(-1);

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(timeoutPaint()));
}

AirBrushTool::~AirBrushTool()
{
}

void AirBrushTool::timeoutPaint()
{
	if (paint(pos, true))
		m_doc -> currentImg() -> markDirty(QRect(pos - m_brush->hotSpot(), m_brush->size()));
}

void AirBrushTool::setBrush(KisBrush *brush)
{
	m_brush = brush;
	brushWidth =  (unsigned int) m_brush->width();
	brushHeight = (unsigned int) m_brush->height();

	// set the array of points to same size as brush
	m_brushArray.resize(brushWidth * brushHeight);
	m_brushArray.fill(0);

	kdDebug() << "setBrush(): "
		<< "brushwidth "   << brushWidth
		<< " brushHeight " << brushHeight
		<< endl;

	// set custom cursor
	m_doc -> setCanvasCursor(KisCursor::airbrushCursor() );
	m_cursor = KisCursor::airbrushCursor();
}


void AirBrushTool::mousePress(QMouseEvent *e)
{
	KisImage * img = m_doc->currentImg();
	if (!img) return;

	if(!img->getCurrentLayer())
		return;

	if(!img->getCurrentLayer()->visible())
		return;

	if (e->button() != QMouseEvent::LeftButton)
		return;

	m_dragging = true;

	pos = e->pos();
	pos = zoomed(pos);
	m_dragStart = pos;
	m_dragdist = 0;

	// clear array
	m_brushArray.fill(0);
	nPoints = 0;

	// Start the m_timer - 50 milliseconds or
	// 20 timeouts/second (multishot)
	m_timer->start(50, FALSE);
}


bool AirBrushTool::paint(QPoint pos, bool timeout)
{
	return false; // BPP
#if 0
	KisView *view = getCurrentView();
	KisImage *img = m_doc->currentImg();

	if (!img)	    return false;

	KisLayer *lay = img->getCurrentLayer();
	if (!lay)       return false;

	if (!m_brush)  return false;

	if (!img->colorMode() == cm_RGB && !img->colorMode() == cm_RGBA)
		return false;

	if(!m_dragging) return false;

	int hotX = m_brush->hotSpot().x();
	int hotY = m_brush->hotSpot().y();

	int startx = pos.x() - hotX;
	int starty = pos.y() - hotY;

	QRect clipRect(startx, starty, m_brush->width(), m_brush->height());
	if (!clipRect.intersects(lay->imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay->imageExtents());

	int sx = clipRect.left() - startx;
	int sy = clipRect.top() - starty;
	int ex = clipRect.right() - startx;
	int ey = clipRect.bottom() - starty;

	uchar *sl;
	uchar bv, invbv;
	uchar r, g, b, a;
	int   v;

	int red   = view->fgColor().R();
	int green = view->fgColor().G();
	int blue  = view->fgColor().B();

	bool alpha = (img->colorMode() == cm_RGBA);

	for (int y = sy; y <= ey; y++) {
		sl = m_brush->scanline(y);

		for (int x = sx; x <= ex; x++) {
			/* get a truly ??? random number and divide it by
			   desired density - if x is that number, paint
			   a pixel from brush there this turn */
			int nRandom = KApplication::random();

			bool paintPoint = false;

			if ((nRandom % density) == ((x - sx) % density))
				paintPoint = true;

			// don't keep painting over points already painted
			// this makes image too dark and grany, eventually -
			// that effect is good with the regular brush tool,
			// but not with an airbrush or with the pen tool
			if (timeout && (m_brushArray[brushWidth * (y-sy) + (x-sx) ] > 0))
				paintPoint = false;

			if (paintPoint) {
				QRgb rgb = lay -> pixel(startx + x, starty + y);

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

				if (alpha) {
					a = qAlpha(rgb);
					v = a + bv;

					if (v < 0) 
						v = 0;

					if (v > 255) 
						v = 255;

					a = (uchar)v;
				}

				rgb = alpha ? qRgba(r, g, b, a) : qRgb(r, g, b);
				lay -> setPixel(startx + x, starty + y, rgb);

				// add this point to points already painted
				if (timeout) {
					m_brushArray[brushWidth * (y-sy) + (x-sx)] = 1;
					nPoints++;
				}
			}
		}
	}

	return true;
#endif
}

void AirBrushTool::mouseMove(QMouseEvent *e)
{
	KisImage * img = m_doc->currentImg();
	if (!img) return;

	int spacing = m_brush->spacing();
	if (spacing <= 0) spacing = 1;

	if(m_dragging)
	{
		if( !img->getCurrentLayer()->visible() )
			return;

		pos = e->pos();
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

			if (paint(p, false))
			{
				img->markDirty(QRect(p - m_brush->hotSpot(),
							m_brush->size()));
			}

			dist -= spacing;
		}

		//save for next movevent
		if (dist > 0) m_dragdist = dist;
		m_dragStart = pos;
	}
}


void AirBrushTool::mouseRelease(QMouseEvent *e)
{
	// perhaps erase on right button
	if (e->button() != LeftButton)  return;

	// stop the m_timer - restart when mouse pressed again
	m_timer->stop();

	//reset array of points
	m_brushArray.fill(0);
	nPoints = 0;

	m_dragging = false;
}

void AirBrushTool::optionsDialog()
{
	ToolOptsStruct ts;

	ts.usePattern       = m_usePattern;
	ts.useGradient      = m_useGradient;
	ts.opacity          = m_opacity;

	bool old_usePattern   = m_usePattern;
	bool old_useGradient  = m_useGradient;
	unsigned int  old_opacity      = m_opacity;

	ToolOptionsDialog OptsDialog(tt_airbrushtool, ts);

	OptsDialog.exec();

	if(OptsDialog.result() == QDialog::Rejected)
		return;

	m_opacity       = OptsDialog.airBrushToolTab()->opacity();
	m_usePattern    = OptsDialog.airBrushToolTab()->usePattern();
	m_useGradient   = OptsDialog.airBrushToolTab()->useGradient();

	// User change value ?
	if ( old_usePattern != m_usePattern || old_useGradient != m_useGradient || old_opacity != m_opacity ) {
		// set airbrush tool settings
		m_doc->setModified( true );
	}
}

void AirBrushTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Airbrush Tool"), "airbrush", 0, this, 
			SLOT(toolSelect()), collection, "tool_airbrush");

	toggle -> setExclusiveGroup("tools");
}

QDomElement AirBrushTool::saveSettings(QDomDocument& doc) const
{
	QDomElement tool = doc.createElement("tool");

	tool.setAttribute("opacity", m_opacity);
	tool.setAttribute("useCurrentGradient", static_cast<int>(m_useGradient));
	tool.setAttribute("useCurrentPattern", static_cast<int>(m_usePattern));
	return tool;
}

bool AirBrushTool::loadSettings(QDomElement& tool)
{
	bool rc = tool.tagName() == "airbrushTool";

	if (rc) {
		m_opacity = tool.attribute("opacity").toInt();
		m_useGradient = static_cast<bool>(tool.attribute("useCurrentGradient").toInt());
		m_usePattern = static_cast<bool>(tool.attribute("useCurrentPattern").toInt());
	}

	return rc;
}

#include "kis_tool_airbrush.moc"

