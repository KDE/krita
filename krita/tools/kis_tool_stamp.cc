/*
 *  kis_tool_stamp.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
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

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_tool_stamp.h"
#include "kis_view.h"

KisToolStamp::KisToolStamp() : 
	super()
{
}

KisToolStamp::~KisToolStamp() 
{
}

void KisToolStamp::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}


#if 0
void KisToolStamp::setPattern(KisPattern *pattern)
{
	m_pattern = pattern;

	/* Use this to establish pattern size and the
	   "hot spot" in center of image. This will be the
	   same for all stamps, no need to vary it.
	   when tiling patterns, use point 0,0 instead
	   these are simple variables for speed to avoid
	   copy constructors within loops. */

	patternWidth = m_pattern->width();
	patternHeight = m_pattern->height();
	mPatternSize = QSize(patternWidth, patternHeight);
	mHotSpotX = patternWidth/2;
	mHotSpotY = patternHeight/2;
	mHotSpot = QPoint(mHotSpotX, mHotSpotY);
	spacing = m_pattern->spacing();

	if (spacing < 1) 
		spacing = 3;
}
#endif

/*
   On mouse press, the image is stamped or pasted
   into the currentImg layer
 */

void KisToolStamp::mousePress(QMouseEvent *e)
{

	if (e->button() != QMouseEvent::LeftButton) 
		return;
#if 0
	// do sanity checking here, if possible, not inside loops
	// when moving mouse!
	KisImage *img = m_doc -> currentImg();

	if (!img)  
		return;

	if (!img -> colorMode() == cm_RGB && !img -> colorMode() == cm_RGBA) {
		kdDebug(0) << "colormode is not RGB or RGBA!" << endl;
		return;
	}

	KisLayer *lay = img -> getCurrentLayer();

	if (!lay || !lay -> visible())
		return;

	QImage qImage = *(m_pattern -> image());

	if (qImage.isNull()) {
		kdDebug(0) << "Stamptool::no pattern image!" << endl;
		return;
	}

	if(qImage.depth() < 32) {
		kdDebug(0) << "Stamptool::pattern less than 32 bit!" << endl;
		return;
	}

	spacing = m_pattern -> spacing();

	if (spacing < 1) 
		spacing = 3;

	m_dragging = true;

	QPoint pos = e -> pos();

	m_dragStart = pos;
	m_dragdist = 0;

	// stamp the pattern image into the layer memory
	if(stampColor(zoomed(pos) - mHotSpot))
		img -> markDirty(QRect(zoomed(pos) - mHotSpot, mPatternSize));
#endif
}

#if 0
/*
   Stamp to canvas - stamp the pattern only onto canvas -
   it will not affect the layer or image
 */

bool KisToolStamp::stampToCanvas(QPoint pos)
{
	KisView *view = getCurrentView();
	KisImage* img = m_doc->currentImg();
	KisLayer *lay = img->getCurrentLayer();
	float zF = view->zoomFactor();

	int pX = pos.x();
	int pY = pos.y();
	pX = (int)(pX / zF);
	pY = (int)(pY / zF);
	pos = QPoint(pX, pY);

	QPainter p;
	p.begin(m_canvas);
	p.scale(zF, zF);

	QRect ur(pos.x() - mHotSpotX, pos.y()- mHotSpotY,
			patternWidth, patternHeight);

	/* check image bounds.  The image extents are a rectangle
	   containing all the layers that contribute to it, or
	   maybe just a rectangle containing the currentImg layer in
	   terms of canvas coords.  This is unclear... */

	ur = ur.intersect(img->imageExtents());

	if (ur.top()    > img->height()
			|| ur.left()    > img->width()
			|| ur.bottom()  < 0
			|| ur.right()   < 0)
	{
		p.end();
		return false;
	}

	/* check the layer bounds. There may be several different
	   layers visible at once and we only want to draw on the
	   currentImg layer - which usually is also the topmost one
Note:  This is probably unnecessary because intersects
imageExtents() above is probably the same, but I'm not sure.
Better to be safe... */

	if (!ur.intersects(lay->layerExtents()))
	{
		p.end();
		return false;
	}
	ur = ur.intersect(lay->layerExtents());

	int startX = 0;
	int startY = 0;

	if(patternWidth > ur.right())
		startX = patternWidth - ur.right();
	if(patternHeight > ur.bottom())
		startY = patternHeight - ur.bottom();

	// paranioa
	if(startX < 0) startX = 0;
	if(startY < 0) startY = 0;
	if(startX > patternWidth)  startX = patternWidth;
	if(startY > patternHeight) startY = patternHeight;

	int xt = view->xPaintOffset() - view->xScrollOffset();
	int yt = view->yPaintOffset() - view->yScrollOffset();

	p.translate(xt, yt);
	p.drawPixmap( ur.left(), ur.top(),
			m_pattern->pixmap(),
			startX, startY,
			ur.width(), ur.height());

	p.end();

	return true;
}

/*
   stamp the pattern into the layer
 */

bool KisToolStamp::stampColor(QPoint pos)
{
	KisView *view = getCurrentView();
	KisImage *img = m_doc->currentImg();
	KisLayer *lay = img->getCurrentLayer();
	QImage  *qimg = m_pattern->image();

	int startx = pos.x();
	int starty = pos.y();

	QRect clipRect(startx, starty, patternWidth, patternHeight);

	if (!clipRect.intersects(lay->imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay->imageExtents());

	int sx = clipRect.left() - startx;
	int sy = clipRect.top() - starty;
	int ex = clipRect.right() - startx;
	int ey = clipRect.bottom() - starty;

	uchar r = 0, g = 0, b = 0, a = 255;
	QRgb rgb;
	int   v = 255;
	int   bv = 0;

	int red     = view->fgColor().R();
	int green   = view->fgColor().G();
	int blue    = view->fgColor().B();

	bool colorBlending = false;
	bool grayscale = false;
	bool layerAlpha =  (img->colorMode() == cm_RGBA);
	bool patternAlpha = (qimg->hasAlphaBuffer());

	for (int y = sy; y <= ey; y++) {
		for (int x = sx; x <= ex; x++) {
			// destination binary values by channel
			if (colorBlending) {
				rgb = lay -> pixel(startx + x, starty + y);
				r = qRed(rgb);
				g = qGreen(rgb);
				b = qBlue(rgb);
				a = qAlpha(rgb);
			}

			// pixel value in scanline at x offset to right
			uint *p = (uint *)qimg->scanLine(y) + x;

			/* If the image pixel has an alpha channel value of 0,
			   don't paint the pixel. This is normal in many images used
			   as sprites. Setting an alpha value of 0 in the layer does
			   the same but also changes the layer and we don't want that
			   for images with transparent backgrounds. */

			if (patternAlpha)
				if (qAlpha(*p) == 0)
					continue;

			if (layerAlpha) {
				if (grayscale) {
					v = a + bv;
				}
				else {
					v = qAlpha(*p);
					v += a;
				}

				if (v < 0) 
					v = 0;

				if (v > 255) 
					v = 255;

				a = (uchar) v;
			}

			/*  Do rudimentary color blending based on averaging
			    values in the pattern, the background, and the currentImg
			    fgColor.  Later, various types of color blending will
			    be implemented for patterns and brushes using krayon's
			    predefined blend types. (not finished coding yet, but
			    the types have been defined and there is a combo box
			    for selecting them in tool opts dialogs.) */

			if (colorBlending) {
				// make mud!
				r = (qRed(*p) + r + red) / 3;
				g = (qGreen(*p) + g + green) / 3;
				b = (qBlue(*p) + b + blue) / 3;
			}
			else {
				/* set layer pixel to be same as image - this is
				   the same as the overwrite blend mode */

				r = qRed(*p);
				g = qGreen(*p);
				b = qBlue(*p);
			}

			lay -> setPixel(startx + x, starty + y, qRgba(r, g, b, a));
		}
	}

	return true;
}


bool KisToolStamp::stampMonochrome(QPoint /*pos*/)
{
	return true;
}
#endif

void KisToolStamp::mouseMove(QMouseEvent *e)
{
#if 0
	KisView *view = getCurrentView();
	KisImage * img = m_doc->currentImg();
	if(!img) return;

	KisLayer *lay = img->getCurrentLayer();
	if (!lay)  return;

	float zF = view->zoomFactor();

	QPoint pos = e->pos();
	int mouseX = e->x();
	int mouseY = e->y();

	KisVector end(mouseX, mouseY);
	KisVector start(m_dragStart.x(), m_dragStart.y());

	KisVector dragVec = end - start;
	float saved_dist = m_dragdist;
	float new_dist = dragVec.length();
	float dist = saved_dist + new_dist;

	if ((int)dist < spacing) {
		m_dragdist += new_dist;
		m_dragStart = pos;
		return;
	}
	else {
		m_dragdist = 0;
	}

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

		if (m_dragging) {
			/* mouse button is down. Actually draw the
			   image into the layer so long as spacing is
			   less than distance moved */

			if (stampColor(zoomed(p) - mHotSpot))
			{
				img->markDirty(QRect(zoomed(p) - mHotSpot, mPatternSize));
			}
		}
		else
		{
			/* Button is not down. Refresh canvas from the layer
			   and then blit the image to the canvas without affecting
			   the layer at all ! No need for double buffer!!!
			   Refresh first - markDirty relies on timer,
			   so we need force by directly updating the canvas. */

			QRect ur(zoomed(oldp.x()) - mHotSpotX + view->xPaintOffset() - view->xScrollOffset(),
					zoomed(oldp.y()) - mHotSpotY + view->yPaintOffset() - view->yScrollOffset(),
					(int)(patternWidth  * (zF > 1.0 ? zF : 1.0)),
					(int)(patternHeight * (zF > 1.0 ? zF : 1.0)));

			view->updateCanvas(ur);

			// after old spot is refreshed, stamp image into canvas
			// at currentImg location. This may be slow or messy if updates
			// rely on a timer - need threads and semaphores here to let
			// us know when old marking has been replaced with image
			// if timer is used, but the timer is not used for this.

			if(!stampToCanvas(p /*- mHotSpot*/))
			{
				// kdDebug(0) << "off canvas!" << endl;
			}
		}

		oldp = p;
		dist -= spacing;
	}

	if (dist > 0) m_dragdist = dist;
	m_dragStart = pos;
#endif
}


void KisToolStamp::mouseRelease(QMouseEvent *e)
{
	if (e -> button() != LeftButton)
		return;

	m_dragging = false;
}

void KisToolStamp::tabletEvent(QTabletEvent */*event*/) 
{
	// Nothing yet -- I'm not sure how to handle this, perhaps
	// have thick-thin lines for pressure.
}


void KisToolStamp::setup(KActionCollection *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Stamp (Pattern) Tool"),
						  "stamp", 0, this, 
						  SLOT(activate()), collection,
						  "tool_stamp");

	toggle -> setExclusiveGroup("tools");

}



#include "kis_tool_stamp.moc"
