/*
 *  kis_painter.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@ideasandassociates.com>
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
#include <qpoint.h>
#include <qrect.h>

#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_vec.h"
#include "kis_cursor.h"
#include "kis_util.h"
#include "kis_painter.h"

static void swap(int *first, int *second)
{
	if (*first > *second) {
		*first ^= *second;
		*second ^= *first;
		*first ^= *second;
#if 0
		int swap = *first; 
		
		*first = *second; 
		*second = swap;
#endif
	}
}

/*
    KisPainter allows use of QPainter methods to indirectly draw into
    Krayon's layers.  While there is some overhead in using QPainter
    instead of native methods, this is a useful tenative solution
    for lines, ellipses, polgons, curves and other shapes, and
    text rendering.  Most of these will eventually be replaced with
    native methods which draw directly into krayon's layers for
    performance, except perhaps text and curved line segments which
    have been well implemented by Qt and/or for which killustrator can
    be used as an embedded part within krayon.  All matrix and other
    transformations available to Qt can be used with these kis_painter
    routines without inferfering at all with native krayon methods.
*/

KisPainter::KisPainter(KisDoc *doc, KisView *view)
{
  	m_doc = doc;
	m_view = view;
	m_lineThickness = 1;
	m_lineOpacity = 255;
	m_gradientFill = false;
	m_patternFill = false;
	m_filledEllipse = false;
	m_filledRectangle = false;
	m_painterPixmap.resize(512, 512);
	clearAll();
}

KisPainter::~KisPainter()
{
}

void KisPainter::clearAll()
{
	m_painterPixmap.fill();
}

void KisPainter::clearRectangle(int x, int y, int w, int h)
{
	QPainter p(&m_painterPixmap);

	p.eraseRect(x, y, w, h);
}

void KisPainter::clearRectangle(const QRect& rc)
{
	clearRectangle(rc.x(), rc.y(), rc.width(), rc.height());
}

void KisPainter::clearRectangle(const QPoint& topLeft, const QPoint& bottomRight)
{
	clearRectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

void KisPainter::resize(int width, int height)
{
	m_painterPixmap.resize(width, height);
}

void KisPainter::drawLine(int x1, int y1, int x2, int y2)
{
	/* use black for pen color - it will be set
	   to actual foreground color and mapped to gradient
	   and pattern when drawn into layer */

	QPainter p(&m_painterPixmap);
	QPen pen(Qt::black, m_lineThickness);

	p.setPen(pen);
	p.drawLine(x1, y1, x2, y2);

	/* establish rectangle with values ascending from
	   left to right and top to bottom for copying into
	   layer image - not needed with rectangle and ellipse */

	swap(&x1, &x2);
	swap(&y1, &y2);

	QRect rect = QRect(QPoint(x1, y1), QPoint(x2, y2));

	// account for line thickness in update rectangle
	QRect ur(rect);
	ur.setLeft(rect.left() - m_lineThickness);
	ur.setTop(rect.top() - m_lineThickness);
	ur.setRight(rect.right() + m_lineThickness);
	ur.setBottom(rect.bottom() + m_lineThickness);

	if (!toLayer(ur))
		kdDebug() << "error drawing line" << endl;
}

void KisPainter::drawLine(const QRect& rc)
{
	drawLine(rc.left(), rc.top(), rc.right(), rc.bottom());
}

void KisPainter::drawLine(const QPoint& topLeft, const QPoint& bottomRight)
{
	drawLine(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

void KisPainter::drawRectangle(int x, int y, int w, int h)
{
	QRect rc(x, y, w, h);

	drawRectangle(rc);
}

void KisPainter::drawRectangle(const QRect& rect)
{
	QPainter p(&m_painterPixmap);

	// constructs a pen with the given line thickness
	// color is always black - it is changed by krayon
	// to the current fgColor when drawn to layer
	QPen pen(Qt::black, m_lineThickness);
	p.setPen(pen);

	// constructs a brush with a solid pattern if
	// we want to fill the rectangle
	QBrush brush(Qt::black);

	if (m_filledRectangle)
		p.setBrush(brush);

	p.drawRect(rect);

	// account for line thickness in update rectangle
	QRect ur(rect);

	ur.setLeft(rect.left() - m_lineThickness);
	ur.setTop(rect.top() - m_lineThickness);
	ur.setRight(rect.right() + m_lineThickness);
	ur.setBottom(rect.bottom() + m_lineThickness);

	if (!toLayer(ur))
		kdDebug() << "error drawing rectangle" << endl;
}

void KisPainter::drawRectangle(const QPoint& topLeft, const QPoint& bottomRight)
{
	QRect rc(topLeft, bottomRight);

	drawRectangle(rc);
}

/*
    draw ellipse with center at (x,y) with given width and height
*/
void KisPainter::drawEllipse(int x, int y, int w, int h)
{
	QPainter p(&m_painterPixmap);

	// constructs a pen with the given line thickness
	// color is always black - it is changed by krayon
	// to the current fgColor when drawn to layer
	QPen pen(Qt::black, m_lineThickness);
	p.setPen(pen);

	// constructs a brush with a solid pattern if
	// we want to fill the ellipse
	QBrush brush(Qt::black);

	if (m_filledEllipse)
		p.setBrush(brush);

	p.drawEllipse(x, y, w, h);

	QRect rect(x - w / 2, y - h / 2, w, h);
	QRect ur(rect);

	// account for line thickness in update rectangle
	ur.setLeft(rect.left() - m_lineThickness);
	ur.setTop(rect.top() - m_lineThickness);
	ur.setRight(rect.right() + m_lineThickness);
	ur.setBottom(rect.bottom() + m_lineThickness);

	if (!toLayer(QRect(x, y, w, h)))
		kdDebug() << "error drawing ellipse" << endl;
}

/*
    draw ellipse inside given rectangle
*/
void KisPainter::drawEllipse(const QRect& rc)
{
	drawEllipse(rc.x(), rc.y(), rc.width(), rc.height());
}

void KisPainter::drawEllipse(const QPoint& topLeft, const QPoint& bottomRight)
{
	drawEllipse(QRect(topLeft, bottomRight));
}

/*
    draw polygon
*/
void KisPainter::drawPolygon(const QPointArray& points, const QRect& rect)
{
	QPainter p(&m_painterPixmap);
	QPen pen(Qt::black, m_lineThickness);
	QBrush brush(Qt::black);

	p.setPen(pen);

	// constructs a brush with a solid pattern if
	// we want to fill the ellipse
	if (m_filledPolygon)
		p.setBrush(brush);

	p.drawPolygon(points);

	// account for line thickness in update rectangle
	QRect ur(rect);

	ur.setLeft(rect.left() - m_lineThickness);
	ur.setTop(rect.top() - m_lineThickness);
	ur.setRight(rect.right() + m_lineThickness);
	ur.setBottom(rect.bottom() + m_lineThickness);

	if (!toLayer(ur))
		kdDebug() << "error drawing polygon" << endl;
}

bool KisPainter::toLayer(const QRect& paintRect)
{
	KisImage *img;
	KisLayer *lay;
	QImage *qimg;

	m_painterImage = m_painterPixmap.convertToImage();

	if (!(img = m_doc -> current()))
		return false;

	if (!(lay = img -> getCurrentLayer()))
		return false;

	qimg = &m_painterImage;
	kdDebug() << "painter pixmap "  << " width " << qimg -> width() << " height " << qimg -> height() << endl;

	if (!img -> colorMode() == cm_RGB && !img -> colorMode() == cm_RGBA) {
		kdDebug() << "Warning: color mode not enabled" << endl;
		return false;
	}

	if (qimg -> depth() < 16)
		kdDebug() << "Warning: kisPainter image depth < 16" << endl;

	bool colorBlending = true;
	bool alpha = img -> colorMode() == cm_RGBA;
	bool averageAlpha = false;

	QRect clipRect(paintRect);
	
	if (!clipRect.intersects(lay -> imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay -> imageExtents());

	int sx = clipRect.left();
	int sy = clipRect.top();
	int ex = clipRect.right();
	int ey = clipRect.bottom();
	unsigned char r;
	unsigned char g;
	unsigned char b;
	int a = 255;
	int opacity = m_lineOpacity;
	int invopacity = 255 - opacity;

	int fgRed     = m_view -> fgColor().R();
	int fgGreen   = m_view -> fgColor().G();
	int fgBlue    = m_view -> fgColor().B();

	// prepare framebuffer for painting with gradient
	if (m_gradientFill)
		m_doc -> frameBuffer() -> setGradientPaint(true, m_view -> fgColor(), m_view -> bgColor());
	// prepare framebuffer for painting with pattern
	else if (m_patternFill)
		m_doc -> frameBuffer() -> setPattern(m_view -> currentPattern());

	for (int y = sy; y <= ey; y++) {
		for (int x = sx; x <= ex; x++) {
			// destination binary values by channel
			r = lay -> pixel(0, x, y);
			g = lay -> pixel(1, x, y);
			b = lay -> pixel(2, x, y);

			if (alpha)
				a = lay -> pixel(3, x, y);
            
			// pixel value in scanline at x offset to right
			// in terms of the image
			uint *p = (uint *)qimg -> scanLine(y) + x;

			// ignore the white background filler,
			// only change black pixels
			if(QColor(*p) != Qt::black) 
				continue;

			/* NOTE: Currently these different painting modes are
			   mutually exclusive but when the krayon blend settings
			   are hooked in there will be a variety of blending methods
			   which can be combined to meet the user's exact specs.
			   This will be handled by the KisFrameBuffer class */

			// paint with gradient
			if (m_gradientFill)
				m_doc -> frameBuffer() -> setGradientToPixel(lay, x, y);
			// paint with pattern
			else if (m_patternFill)
				m_doc -> frameBuffer() -> setPatternToPixel(lay, x, y, 0);
			// set layer pixel to foreground color
			else if (!colorBlending) {
				lay -> setPixel(0, x,  y, fgRed);
				lay -> setPixel(1, x,  y, fgGreen);
				lay -> setPixel(2, x,  y, fgBlue);
			}
			/* blend source and destination values
			   for each color based on opacity of source */
			else {
				lay -> setPixel(0, x, y, (fgRed * opacity + r * invopacity) / 255);
				lay -> setPixel(1, x, y, (fgGreen * opacity + g * invopacity) / 255);
				lay -> setPixel(2, x, y, (fgBlue * opacity + b * invopacity) / 255);
			}

			/* average source and destination alpha values
			   for semi-transparent effect with other layers */
			if (alpha && averageAlpha) {
				int v = (a + opacity) / 2;
				
				if (v < 0) 
					v = 0;

				if (v > 255) 
					v = 255;

				a = (uchar)v;
				lay -> setPixel(3, x,  y, a);
			}
			/* use alpha value of the tool only -
			   ignore existing layer alpha value.  This
			   allows drawing on transparent areas  */
			else if(alpha) {
				int v = opacity;

				if (v < 0 ) v = 0;
				if (v > 255 ) v = 255;
				a = (uchar) v;
				lay->setPixel(3, x,  y, a);
			}
		}
	}

	img -> markDirty(clipRect);
	clearRectangle(clipRect);
	return true;
}

#include "kis_painter.moc"

