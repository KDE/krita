/*
 *  kis_tool_eraser.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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


#include <kaction.h>
#include <kdebug.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_tool_eraser.h"
#include "kis_vec.h"
#include "kis_view.h"

KisToolEraser::KisToolEraser() 
	: super(),
	  m_mode ( HOVER),
	  m_dragDist ( 0 )
{
	setCursor(KisCursor::eraserCursor());

	m_painter = 0;
	m_currentImage = 0;
}

KisToolEraser::~KisToolEraser() 
{
}


void KisToolEraser::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}


void KisToolEraser::mousePress(QMouseEvent *e)
{
        if (!m_subject) return;

        if (!m_subject->currentBrush()) return;

        if (e->button() == QMouseEvent::LeftButton) {
		m_mode = ERASE;
		initErase(e -> pos());
		m_painter -> eraseAt(e->pos(), 128, 0, 0);
		// XXX: get the rect that should be notified
		m_currentImage -> notify( m_painter -> dirtyRect() );
         }
}

void KisToolEraser::mouseMove(QMouseEvent *e) {
}

void KisToolEraser::mouseRelease(QMouseEvent *e) {
}

void KisToolEraser::tabletEvent(QTabletEvent *e) {
}

void KisToolEraser::initErase(const QPoint & pos) {
}

void KisToolEraser::endErase() {
}

void KisToolEraser::erase(const QPoint& pos)
{
// 	if (!m_brush) 
// 		return false;

// 	KisImageSP img = m_doc -> currentImg();

// 	if (!img)	
// 		return false;
	
// 	KisLayerSP lay = img -> getCurrentLayer();

// 	if (!lay)   
// 		return false;

// 	if (!img -> colorMode() == cm_RGB && !img -> colorMode() == cm_RGBA)
// 		return false;

// 	int startx = (pos - m_brush -> hotSpot()).x();
// 	int starty = (pos - m_brush -> hotSpot()).y();

// 	QRect clipRect(startx, starty, m_brush -> width(), m_brush -> height());

// 	if (!clipRect.intersects(lay -> imageExtents()))
// 		return false;

// 	clipRect = clipRect.intersect(lay -> imageExtents());

// 	int sx = clipRect.left() - startx;
// 	int sy = clipRect.top() - starty;
// 	int ex = clipRect.right() - startx;
// 	int ey = clipRect.bottom() - starty;

// 	uchar *sl;
// 	uchar bv, invbv;

// 	bool alpha = img -> colorMode() == cm_RGBA;
// 	QRgb rgb;
// 	uchar r, g, b, a;

// 	if (alpha) {
// 		uchar a;
// 		int   v;

// 		for (int y = sy; y <= ey; y++) {
// 			sl = m_brush -> scanline(y);

// 			for (int x = sx; x <= ex; x++) {
// 				bv = *(sl + x);

// 				if (bv == 0) 
// 					continue;

// 				rgb = lay -> pixel(startx + x, starty + y);
// 				r = qRed(rgb);
// 				g = qGreen(rgb);
// 				b = qBlue(rgb);
// 				a = qAlpha(rgb);
// 				v = a - bv;

// 				if (v < 0) 
// 					v = 0;

// 				if (v > 255) 
// 					v = 255;

// 				a = (uchar)v;
// 				lay -> setPixel(startx + x, starty + y, qRgba(r, g, b, a), m_cmd);
// 			}
// 		}
// 	}
// 	else {   // no alpha channel -> erase to background color
// 		KisView *view = getCurrentView();
// 		int red = view -> bgColor().R();
// 		int green = view -> bgColor().G();
// 		int blue = view -> bgColor().B();

// 		for (int y = sy; y <= ey; y++) {
// 			sl = m_brush -> scanline(y);

// 			for (int x = sx; x <= ex; x++) {
// 				rgb = lay -> pixel(startx + x, starty + y);
// 				r = qRed(rgb);
// 				g = qGreen(rgb);
// 				b = qBlue(rgb);

// 				bv = *(sl + x);

// 				if (bv == 0) 
// 					continue;

// 				invbv = 255 - bv;

// 				b = ((blue * bv) + (b * invbv)) / CHANNEL_MAX;
// 				g = ((green * bv) + (g * invbv)) / CHANNEL_MAX;
// 				r = ((red * bv) + (r * invbv)) / CHANNEL_MAX;
// 				lay -> setPixel(startx + x, starty + y, qRgb(r, g, b), m_cmd);
// 			}
// 		}
// 	}

// 	return true;
}

void KisToolEraser::setup(KActionCollection *collection)
{
	
        KToggleAction *toggle;
	toggle = new KToggleAction(i18n("&Eraser Tool"), 
				   "eraser", 0, this, 
				   SLOT(activate()), collection,
				   "tool_eraser");
	toggle -> setExclusiveGroup("tools");
}
