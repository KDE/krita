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

#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_view.h"
#include "kis_vec.h"
#include "kis_util.h"

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

	x1 += m_view -> xPaintOffset();
	x2 += m_view -> xPaintOffset();
	y1 += m_view -> yPaintOffset();
	y2 += m_view -> yPaintOffset();

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
	// to the currentImg fgColor when drawn to layer
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
	// to the currentImg fgColor when drawn to layer
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
	kdDebug() << "KisPainter::toLayer\n";

	KisImage *img;
	KisLayer *lay;
	QImage qimg = m_painterPixmap.convertToImage();

	if (!(img = m_doc -> currentImg()))
		return false;

	if (!(lay = img -> getCurrentLayer()))
		return false;

	QRect clipRect = paintRect.intersect(lay -> imageExtents());
	
	if (clipRect.isNull())
		return false;

	bool colorBlending = true;
	bool alpha = img -> colorMode() == cm_RGBA;
	bool averageAlpha = true;
	int a = 255;
	int opacity = m_lineOpacity;
	int invopacity = 255 - opacity;
	QRgb fg = qRgb(m_view -> fgColor().R(), m_view -> fgColor().G(), m_view -> fgColor().B());

	if (m_gradientFill)
		m_doc -> frameBuffer() -> setGradientPaint(true, m_view -> fgColor(), m_view -> bgColor());
	else if (m_patternFill)
		m_doc -> frameBuffer() -> setPattern(m_view -> currentPattern());

	for (int y = clipRect.top(); y <= clipRect.bottom(); y++) {
		for (int x = clipRect.left(); x <= clipRect.right(); x++) {
			QRgb rgb = lay -> pixel(x, y);
			int r = qRed(rgb);
			int g = qGreen(rgb);
			int b = qBlue(rgb);

			if (alpha) {
				int v;

				if (averageAlpha)
					v = (qAlpha(rgb) + opacity) / 2;
				else
					v = opacity;

				if (v < 0) 
					v = 0;

				if (v > 255) 
					v = 255;

				a = v;
			}
			
			QRgb *p = (QRgb*)qimg.scanLine(y) + x;

			if (QColor(*p) != Qt::black) 
				continue;

			/* NOTE: Currently these different painting modes are
			   mutually exclusive but when the krayon blend settings
			   are hooked in there will be a variety of blending methods
			   which can be combined to meet the user's exact specs.
			   This will be handled by the KisFrameBuffer class */

			if (m_gradientFill)
				m_doc -> frameBuffer() -> setGradientToPixel(lay, x, y);
			else if (m_patternFill)
				m_doc -> frameBuffer() -> setPatternToPixel(lay, x, y, 0);
			else if (colorBlending) {
				r = (qRed(fg) * opacity + r * invopacity) / 255;
				g = (qGreen(fg) * opacity + g * invopacity) / 255;
				b = (qBlue(fg) * opacity + b * invopacity) / 255;
				rgb = qRgba(r, g, b, a);
				lay -> setPixel(x, y, rgb);
			}
			else
				lay -> setPixel(x, y, fg);
		}
	}

	img -> markDirty(clipRect);
	clearRectangle(clipRect);
	return true;
}

//#include "kis_painter.moc"

