/*
 *  kis_tool_paste.cc - part of Krayon
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

#include <qcolor.h>
#include <qclipboard.h>
#include <qpainter.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_canvas.h"
#include "kis_vec.h"
#include "kis_cursor.h"
#include "kis_util.h"
#include "kis_tool_paste.h"

PasteTool::PasteTool(KisDoc *doc, KisCanvas *canvas) : KisTool(doc)
{
	m_dragging = false;
	m_dragdist = 0;
	m_canvas = canvas;
	m_cursor = KisCursor::crossCursor();
}

PasteTool::~PasteTool() 
{
}

bool PasteTool::setClip()
{
    m_clipImage = kapp->clipboard()->image();

    if (m_clipImage.isNull())
    {
        kdDebug(0) << "PasteTool:: clipboard image is null!" << endl;
        return false;
    }
    else
    {
        kdDebug(0) << "PasteTool:: clipboard image is NOT null!" << endl;
    }

    /* if dealing with 1 or 8 bit images, convert to 16 bit */
    if(m_clipImage.depth() < 16)
    {
        QImage sI = m_clipImage.smoothScale(m_clipImage.width(), m_clipImage.height());
        m_clipImage = sI;

        if(m_clipImage.isNull())
        {
            kdDebug(0) << "PasteTool:: can't smooth scale clip image!" << endl;
            return false;
        }
    }

    clipPix.convertFromImage(m_clipImage, QPixmap::AutoColor);
    if(clipPix.isNull())
    {
        kdDebug(0) << "PasteTool:: can't convernt from image!" << endl;
        return false;
    }

    /* use this to establish clip size and the "hot spot" in center
    of image, will be the  same for all clips, no need to vary it. */

    clipWidth = clipPix.width();
    clipHeight = clipPix.height();
    mClipSize = QSize(clipWidth, clipHeight);
    mHotSpotX = 0;
    mHotSpotY = 0;
    mHotSpot = QPoint(mHotSpotX, mHotSpotY);

    if(!m_clipImage.hasAlphaBuffer())
        kdDebug() << "paste tool - m_clipImage has no alpha buffer!" << endl;

    return true;
}


void PasteTool::setOpacity(int /*opacity*/)
{

}

/*
    On mouse press, simple paste clip image at mouse coords
*/

void PasteTool::mousePress(QMouseEvent *e)
{
    if (e->button() != QMouseEvent::LeftButton)
        return;

    KisImage *img = m_doc->current();
    if (!img)  return;

    if(!img->getCurrentLayer()->visible()) return;

    m_dragging = true;

    QPoint pos = e->pos();

    m_dragStart = pos;
    m_dragdist = 0;

    if(pasteColor(zoomed(pos)))
    {
        img->markDirty(QRect(zoomed(pos) - mHotSpot, mClipSize));
    }
}


bool PasteTool::pasteColor(QPoint pos)
{
    KisImage *img = m_doc->current();
    if (!img)   return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay)   return false;

    QImage *qimg = &m_clipImage;

    int startx = pos.x();
    int starty = pos.y();

    QRect clipRect(startx, starty, qimg->width(), qimg->height());

    if (!clipRect.intersects(img->getCurrentLayer()->imageExtents()))
        return false;

    clipRect = clipRect.intersect(img->getCurrentLayer()->imageExtents());

    int sx = clipRect.left() - startx;
    int sy = clipRect.top() - starty;
    int ex = clipRect.right() - startx;
    int ey = clipRect.bottom() - starty;

    uchar r, g, b, a;
    int   v = 255;
    int   bv = 0;

    int red     = m_view->fgColor().R();
    int green   = m_view->fgColor().G();
    int blue    = m_view->fgColor().B();

    bool grayscale = false;
    bool colorBlending = false;
    bool layerAlpha = (img->colorMode() == cm_RGBA);
    bool imageAlpha = qimg->hasAlphaBuffer();

#if 0
    for (int y = sy; y <= ey; y++)
    {
        for (int x = sx; x <= ex; x++)
	    {
            // destination binary values by channel
	        r = lay->pixel(0, startx + x, starty + y);
	        g = lay->pixel(1, startx + x, starty + y);
	        b = lay->pixel(2, startx + x, starty + y);

            // pixel value in scanline at x offset to right
            uint *p = (uint *)qimg->scanLine(y) + x;

            // if the alpha value of the pixel in the selection
            // image is 0, don't paint the pixel.  It's transparent.
            if((imageAlpha) && (((*p) >> 24) == 0)) continue;

            if(colorBlending)
            {
                // make mud!
	            lay->setPixel(0, startx + x, starty + y,
                    (qRed(*p) + r + red)/3);
	            lay->setPixel(1, startx + x, starty + y,
                    (qGreen(*p) + g + green)/3);
	            lay->setPixel(2, startx + x, starty + y,
                    (qBlue(*p) + b + blue/3));
            }
            else
            {
                // set layer pixel to be same as image
	            lay->setPixel(0, startx + x, starty + y, qRed(*p));
	            lay->setPixel(1, startx + x, starty + y, qGreen(*p));
	            lay->setPixel(2, startx + x, starty + y, qBlue(*p));
            }

            if (layerAlpha)
	        {
	            a = lay->pixel(3, startx + x, starty + y);
                if(grayscale)
                {
                    v = a + bv;
		            if (v < 0 ) v = 0;
		            if (v > 255 ) v = 255;
		            a = (uchar) v;
			    }

		        lay->setPixel(3, startx + x, starty + y, a);
	        }
	    }
    }
#endif

    return true;
}


bool PasteTool::pasteMonochrome(QPoint /* pos */)
{
    return true;
}


/*
    Stamp to canvas - stamp the pattern only onto canvas -
    it will not affect the layer or image
*/

bool PasteTool::pasteToCanvas(QPoint pos)
{
    KisImage* img = m_doc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    float zF = m_view->zoomFactor();

    int pX = pos.x();
    int pY = pos.y();
    pX = (int)(pX / zF);
    pY = (int)(pY / zF);
    pos = QPoint(pX, pY);

    QPainter p;
    p.begin(m_canvas);
    p.scale( zF, zF );

    QRect ur(pos.x(), pos.y(), clipPix.width(), clipPix.height());

    /* check image bounds.  The image extents are a rectangle
    containing all the layers that contribute to it, or maybe
    just the current layer in terms of canvas coords.  This
    is not clear, so also check layerExtents() below. */

    ur = ur.intersect(img->imageExtents());

    if (ur.top() - mHotSpotY > img->height()
    || ur.left() - mHotSpotX > img->width()
    || ur.bottom() - mHotSpotY < 0
    || ur.right() -  mHotSpotX < 0)
    {
        p.end();
        return false;
    }

    /* check the layer bounds. There may be several different
    layers visible at once and we only want to draw over the
    current layer - which usually is also the topmost one
    This may be unnecessary because imageExtents above may
    be the same as layerExtents, but just to be sure.. */

    if (!ur.intersects(lay->layerExtents()))
    {
        p.end();
        return false;
    }
    ur = ur.intersect(lay->layerExtents());

    int startX = 0;
    int startY = 0;

    if(clipPix.width() > ur.right())
        startX = clipPix.width() - ur.right();
    if(clipPix.height() > ur.bottom())
        startY = clipPix.height() - ur.bottom();

    // paranioa
    if(startX < 0) startX = 0;
    if(startY < 0) startY = 0;
    if(startX > clipPix.width())  startX = clipPix.width();
    if(startY > clipPix.height()) startY = clipPix.height();

    int xt = m_view->xPaintOffset() - m_view->xScrollOffset();
    int yt = m_view->yPaintOffset() - m_view->yScrollOffset();

    p.translate(xt, yt);

    p.drawPixmap( ur.left(), ur.top(),
                  clipPix,
                  startX, startY,
                  ur.width(), ur.height() );

    p.end();

    return true;
}


void PasteTool::mouseMove(QMouseEvent *e)
{
    KisImage * img = m_doc->current();
    if (!img) return;

    int spacing = 10;

    float zF = m_view->zoomFactor();

    QPoint pos = e->pos();
    int mouseX = e->x();
    int mouseY = e->y();

    KisVector end(mouseX, mouseY);
    KisVector start(m_dragStart.x(), m_dragStart.y());

    KisVector dragVec = end - start;
    float saved_dist = m_dragdist;
    float new_dist = dragVec.length();
    float dist = saved_dist + new_dist;

    if ((int)dist < spacing)
    {
        m_dragdist += new_dist;
        m_dragStart = pos;
        return;
    }
    else
    {
        m_dragdist = 0;
    }

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

        if(m_dragging)
        {
            /* mouse button is down. Actually draw the
            image into the layer so long as spacing is
            less than distance moved */

            if (pasteColor(zoomed(p) - mHotSpot))
            {
                img->markDirty(QRect(zoomed(p) - mHotSpot, clipPix.size()));
            }
        }
        else
        {
            /* Button is not down. Refresh canvas from the layer
            and then blit the image to the canvas without affecting
            the layer at all ! No need for double buffer!!!
            Refresh first - markDirty relies on timer,
            so we need force by directly updating the canvas. */

            QRect ur(zoomed(oldp.x()) - mHotSpotX - m_view->xScrollOffset(),
                     zoomed(oldp.y()) - mHotSpotY - m_view->yScrollOffset(),
                     (int)(clipPix.width() * (zF > 1.0 ? zF : 1.0)),
                     (int)(clipPix.height() * (zF > 1.0 ? zF : 1.0)));

            m_view->updateCanvas(ur);

            /* after old spot is refreshed, stamp image into canvas
            at current location. This may be slow or messy as updates
            rely on a timer - need threads and semaphores here to let
            us know when old marking has been replaced with image
            if timer is used, but it's not used for this. */

            if(!pasteToCanvas(p /*- mHotSpot*/))
            {
                //kdDebug(0) << "off canvas" << endl;
            }
        }

        oldp = p;
        dist -= spacing;
    }

    if (dist > 0) m_dragdist = dist;
    m_dragStart = pos;
}


void PasteTool::mouseRelease(QMouseEvent *e)
{
    if (e->button() != LeftButton)
        return;

    m_dragging = false;
}

void PasteTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Paste tool"), "editpaste", 0, this, SLOT(toolSelect()), collection, "tool_paste");

	toggle -> setExclusiveGroup("tools");
}

void PasteTool::toolSelect()
{
	if (!m_view)
		return;

	if(m_doc -> getClipImage()) {
		setClip();
		m_view -> activateTool(this);
		m_view -> slotUpdateImage();
	}
	else
		KMessageBox::sorry(NULL, i18n("Nothing to paste!"), "", false);
}

bool PasteTool::shouldRepaint()
{
	return true;
}

