/*
 *   kis_painter.cc - part of Krayon
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

#include <qpainter.h>
#include <qcolor.h>
#include <qclipboard.h>
#include <kapplication.h>
#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_vec.h"
#include "kis_cursor.h"
#include "kis_util.h"
#include "kis_painter.h"

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
  	pDoc  = doc;
    pView = view;

    lineThickness = 1;
    lineOpacity = 255;

    gradientFill    = false;
    patternFill     = false;
    filledEllipse   = false;
    filledRectangle = false;

    painterPixmap.resize(512, 512);
    clearAll();
}


KisPainter::~KisPainter()
{
}


void KisPainter::clearAll()
{
    painterPixmap.fill();
}


void KisPainter::clearRectangle(QRect & ur)
{
    QPainter p(&painterPixmap);
    p.eraseRect( ur );
}


void KisPainter::resize(int width, int height)
{
    painterPixmap.resize(width, height);
}


void KisPainter::swap(int *first, int *second)
{
    if( *first > *second )
    {
       int swap = *first; *first = *second; *second = swap;
    }
}


bool KisPainter::toLayer(QRect paintRect)
{
    painterImage = painterPixmap.convertToImage();

    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    QImage *qimg = &painterImage;
    kdDebug(0) << "painter pixmap "  << " width " << qimg->width()
    << " height " << qimg->height() << endl;

    if (!img->colorMode() == cm_RGB && !img->colorMode() == cm_RGBA)
    {
        kdDebug() << "Warning: color mode not enabled" << endl;
	    return false;
    }

    if(qimg->depth() < 16)
    {
        kdDebug() << "Warning: kisPainter image depth < 16" << endl;
    }

    bool colorBlending = true;
    bool alpha = (img->colorMode() == cm_RGBA);
    bool averageAlpha = false;

    QRect clipRect(paintRect);
    kdDebug(0) << "clipRect " << " left " << clipRect.left()
    << " top " << clipRect.top() << " width " << clipRect.width()
    << " height " << clipRect.height() << endl;

    if (!clipRect.intersects(lay->imageExtents()))
        return false;

    clipRect = clipRect.intersect(lay->imageExtents());

    int sx = clipRect.left();
    int sy = clipRect.top();
    int ex = clipRect.right();
    int ey = clipRect.bottom();

    uchar r, g, b;
    int a = 255;
    int opacity = lineOpacity;
    int invopacity = 255 - opacity;

    int fgRed     = pView->fgColor().R();
    int fgGreen   = pView->fgColor().G();
    int fgBlue    = pView->fgColor().B();

    // prepare framebuffer for painting with gradient
    if(gradientFill)
    {
        pDoc->frameBuffer()->setGradientPaint(true,
            pView->fgColor(), pView->bgColor());
    }
    // prepare framebuffer for painting with pattern
    else if(patternFill)
    {
        pDoc->frameBuffer()->setPattern(pView->currentPattern());
    }

    for (int y = sy; y <= ey; y++)
    {
        for (int x = sx; x <= ex; x++)
	    {
            // destination binary values by channel
            r = lay->pixel(0, x, y);
            g = lay->pixel(1, x, y);
            b = lay->pixel(2, x, y);

            if(alpha)
            {
                a = lay->pixel(3, x, y);
            }
            // pixel value in scanline at x offset to right
            // in terms of the image
            uint *p = (uint *)qimg->scanLine(y) + x;

            // ignore the white background filler,
            // only change black pixels
            if(QColor(*p) != Qt::black) continue;

            /* NOTE: Currently these different painting modes are
            mutually exclusive but when the krayon blend settings
            are hooked in there will be a variety of blending methods
            which can be combined to meet the user's exact specs.
            This will be handled by the KisFrameBuffer class */

            // paint with gradient
            if(gradientFill)
            {
                pDoc->frameBuffer()->setGradientToPixel(lay, x, y);
            }
            // paint with pattern
            else if(patternFill)
            {
                pDoc->frameBuffer()->setPatternToPixel(lay, x, y, 0);
            }
            // set layer pixel to foreground color
            else if(!colorBlending)
            {
	            lay->setPixel(0, x,  y, fgRed);
	            lay->setPixel(1, x,  y, fgGreen);
	            lay->setPixel(2, x,  y, fgBlue);
            }
            /* blend source and destination values
            for each color based on opacity of source */
            else
	        {
                lay->setPixel(0, x, y,
                    (fgRed*opacity + r*invopacity)/255);
                lay->setPixel(1, x, y,
                    (fgGreen*opacity + g*invopacity)/255);
                lay->setPixel(2, x, y,
                    (fgBlue*opacity + b*invopacity)/255);
            }

            /* average source and destination alpha values
            for semi-transparent effect with other layers */
            if(alpha && averageAlpha)
            {
                int v = (a + opacity)/2;
                if (v < 0 ) v = 0;
                if (v > 255 ) v = 255;
                a = (uchar) v;
	            lay->setPixel(3, x,  y, a);
            }
            /* use alpha value of the tool only -
            ignore existing layer alpha value.  This
            allows drawing on transparent areas  */
            else if(alpha)
            {
                int v = opacity;
                if (v < 0 ) v = 0;
                if (v > 255 ) v = 255;
                a = (uchar) v;
	            lay->setPixel(3, x,  y, a);
            }
	    }
    }

    img->markDirty(clipRect);
    clearRectangle(clipRect);

    return true;
}


void KisPainter::drawLine(int x1, int y1, int x2, int y2)
{
    /* use black for pen color - it will be set
    to actual foreground color and mapped to gradient
    and pattern when drawn into layer */

    QPainter p(&painterPixmap);
    QPen pen(Qt::black, lineThickness);
    p.setPen(pen);
    p.drawLine(x1, y1, x2, y2);

    /* establish rectangle with values ascending from
    left to right and top to bottom for copying into
    layer image - not needed with rectangle and ellipse */

    swap(&x1, &x2);
    swap(&y1, &y2);

    QRect rect = QRect( QPoint(x1, y1), QPoint(x2, y2) );

    // account for line thickness in update rectangle
    QRect ur(rect);
    ur.setLeft(rect.left() - lineThickness);
    ur.setTop(rect.top() - lineThickness);
    ur.setRight(rect.right() + lineThickness);
    ur.setBottom(rect.bottom() + lineThickness);

    if(!toLayer(ur))
        kdDebug() << "error drawing line" << endl;
}


void KisPainter::drawRectangle(QRect & rect)
{
    QPainter p(&painterPixmap);

    // constructs a pen with the given line thickness
    // color is always black - it is changed by krayon
    // to the current fgColor when drawn to layer
    QPen pen(Qt::black, lineThickness);
    p.setPen(pen);

    // constructs a brush with a solid pattern if
    // we want to fill the rectangle
    QBrush brush(Qt::black);
    if(filledRectangle)
        p.setBrush(brush);

    p.drawRect(rect);

    // account for line thickness in update rectangle
    QRect ur(rect);
    ur.setLeft(rect.left() - lineThickness);
    ur.setTop(rect.top() - lineThickness);
    ur.setRight(rect.right() + lineThickness);
    ur.setBottom(rect.bottom() + lineThickness);

    if(!toLayer(ur))
        kdDebug() << "error drawing rectangle" << endl;
}


void KisPainter::drawRectangle(int x, int y, int w, int h)
{
    QPainter p(&painterPixmap);

    // constructs a pen with the given line thickness
    // color is always black - it is changed by krayon
    // to the current fgColor when drawn to layer
    QPen pen(Qt::black, lineThickness);
    p.setPen(pen);
    p.drawRect(x, y, w, h);

    // constructs a brush with a solid pattern if
    // we want to fill the rectangle
    QBrush brush(Qt::black);
    if(filledRectangle)
        p.setBrush(brush);

    QRect rect(x, y, w, h);

    // account for line thickness in update rectangle
    QRect ur(rect);
    ur.setLeft(rect.left() - lineThickness);
    ur.setTop(rect.top() - lineThickness);
    ur.setRight(rect.right() + lineThickness);
    ur.setBottom(rect.bottom() + lineThickness);

    if(!toLayer(ur))
        kdDebug() << "error drawing rectangle" << endl;
}

/*
    draw ellipse inside given rectangle
*/
void KisPainter::drawEllipse(QRect & rect)
{
    QPainter p(&painterPixmap);

    // constructs a pen with the given line thickness
    // color is always black - it is changed by krayon
    // to the current fgColor when drawn to layer
    QPen pen(Qt::black, lineThickness);
    p.setPen(pen);

    // constructs a brush with a solid pattern if
    // we want to fill the ellipse
    QBrush brush(Qt::black);
    if(filledEllipse)
        p.setBrush(brush);

    p.drawEllipse(rect);

    // account for line thickness in update rectangle
    QRect ur(rect);
    ur.setLeft(rect.left() - lineThickness);
    ur.setTop(rect.top() - lineThickness);
    ur.setRight(rect.right() + lineThickness);
    ur.setBottom(rect.bottom() + lineThickness);

    if(!toLayer(ur))
        kdDebug() << "error drawing ellipse" << endl;
}

/*
    draw ellipse with center at (x,y) with given width and height
*/
void KisPainter::drawEllipse(int x, int y, int w, int h)
{
    QPainter p(&painterPixmap);

    // constructs a pen with the given line thickness
    // color is always black - it is changed by krayon
    // to the current fgColor when drawn to layer
    QPen pen(Qt::black, lineThickness);
    p.setPen(pen);

    // constructs a brush with a solid pattern if
    // we want to fill the ellipse
    QBrush brush(Qt::black);
    if(filledEllipse)
        p.setBrush(brush);

    p.drawEllipse(x, y, w, h);

    QRect rect(x - w/2, y - h/2, w, h);

    // account for line thickness in update rectangle
    QRect ur(rect);
    ur.setLeft(rect.left() - lineThickness);
    ur.setTop(rect.top() - lineThickness);
    ur.setRight(rect.right() + lineThickness);
    ur.setBottom(rect.bottom() + lineThickness);

    if(!toLayer(QRect(x, y, w, h)))
         kdDebug() << "error drawing ellipse" << endl;
}

/*
    draw polygon
*/
void KisPainter::drawPolygon(QPointArray & points, QRect & rect)
{
    QPainter p( &painterPixmap );

    // constructs a pen with the given line thickness
    // color is always black - it is changed by krayon
    // to the current fgColor when drawn to layer
    QPen pen( Qt::black, lineThickness );
    p.setPen( pen );

    // constructs a brush with a solid pattern if
    // we want to fill the ellipse
    QBrush brush( Qt::black );
    if ( filledPolygon )
        p.setBrush( brush );

    p.drawPolygon( points );

    // account for line thickness in update rectangle
    QRect ur( rect );
    ur.setLeft( rect.left() - lineThickness );
    ur.setTop( rect.top() - lineThickness );
    ur.setRight( rect.right() + lineThickness );
    ur.setBottom( rect.bottom() + lineThickness );

    if ( !toLayer( ur ) )
        kdDebug() << "error drawing polygon" << endl;
}

#include "kis_painter.moc"

