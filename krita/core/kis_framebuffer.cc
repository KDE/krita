/*
 *  kis_framebuffer.cc - part of Krayon
 *
 *  Copyright (c) 2001 JohnCaliff <jcaliff@compuzone.net>
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

/*
    The krayon framebuffer is for all pixel-writing operations
    caused by the user.  It differs from the krayon engine which
    is responsible for writing to the tiles which compose a krayon
    image as they are marked dirty and updated with the timer.
    These tile are then painted to the canvas to actually show the
    changed channel data on the screen.

    The framebuffer allows writing directly to channel memory
    without being concerned about tiles and layer and channel
    internals, which should be encapsulated from these kinds of
    operations.  This makes it like a conventional framebuffer while
    also allowing the speed of direct access to the channel (binary)
    data mostly through KisLayer::setPixel() and layer::pixel().

    The krayon framebuffer also allows use of QImage methods to
    translate back and forth between QImage data and Krayon channel
    data - for example, in patterns which are accessed as QImages and
    in importing and exporting common image formats with Qt image
    methods.
*/

#include <qcolor.h>
#include <qclipboard.h>

#include <kapplication.h>
#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_vec.h"
#include "kis_util.h"
#include "kis_selection.h"
#include "kis_pattern.h"
#include "kis_gradient.h"
#include "kis_framebuffer.h"


KisFrameBuffer::KisFrameBuffer(KisDoc *doc)
{
	pDoc = doc;
	pScratchLayer = 0;
	mPatternPaint  =  false;
	mGradientPaint =  false;
}

KisFrameBuffer::~KisFrameBuffer()
{
}

void KisFrameBuffer::setRect(QRect & rect)
{
	destRect.setLeft(rect.left());
	destRect.setTop(rect.top());
	destRect.setRight(rect.right());
	destRect.setBottom(rect.bottom());
}

void KisFrameBuffer::setNull()
{
	destRect.setWidth(0);
	destRect.setHeight(0);
}

void KisFrameBuffer::addScratchLayer(int /*width*/, int /*height*/)
{
}

void KisFrameBuffer::removeScratchLayer()
{
}

void KisFrameBuffer::setImage(QImage & img)
{
	srcImage = img;
}

/*
    erase a rectange within a the current layer
    ignore destination color values and alpha value
*/

bool KisFrameBuffer::eraseCurrentLayer()
{
	KisImage *img = pDoc -> current();

	if (!img) 
		return false;

	KisLayer *lay = img -> getCurrentLayer();

	if (!lay) 
		return false;

	if (!img->colorMode() == cm_RGB && !img->colorMode() == cm_RGBA) {
		kdDebug(0) << "colormode is not RGB or RGBA!" << endl;
		return false;
	}

	QRect clipRect(destRect);

	if (!clipRect.intersects(lay -> imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay -> imageExtents());

	int sx = clipRect.left();
	int sy = clipRect.top();
	int ex = clipRect.right();
	int ey = clipRect.bottom();

	for (int y = sy; y <= ey; y++)
		for (int x = sx; x <= ex; x++)
			lay -> setPixel(x, y, qRgb(255, 255, 255));

	return true;
}

/*
    scale - from a rectangle in current layer smoothing colors.
    first, add new layer the size of rectange.
    This will become the new current layer.
    Scale from one layer to other by copying pixels and
    averaging 4 adjacent pixels -
*/

bool KisFrameBuffer::scaleSmooth(QRect & srcR, int newWidth, int newHeight)
{
	KisImage *img = pDoc->current();
	if (!img) return false;

	KisLayer *lay = img->getCurrentLayer();
	if (!lay) return false;

	QRect nr(0, 0, newWidth, newHeight);

	QString layerName;
	layerName.sprintf("layer %d", img->layerList().size());
	// paramaters: rectangle,  color, clear to transparent, name
	img->addLayer(nr, white, true, layerName);

	// adding a layer makes it the new current layer
	KisLayer *nlay = img->getCurrentLayer();

	if (!nlay) {
		kdDebug() << "scaleSmooth(): new layer not allocated!" << endl;
		return false;
	}

	bool alpha = (img->colorMode() == cm_RGBA);
	int srcXoffset = srcR.left();
	int srcYoffset = srcR.top();
	int srcWidth   = srcR.width();
	int srcHeight  = srcR.height();

	int x = 0, y = 0;
	int xpos = x, ypos = y;

	float r, g, b;
	int r1, g1, b1, a1;
	int r2, g2, b2;
	int r3, g3, b3;
	int r4, g4, b4;
	QRgb rgb1;
	QRgb rgb2;
	QRgb rgb3;
	QRgb rgb4;

	float x1, y1;

	float xerr, yerr;
	float xfloat, yfloat;

	float ratio_x = (float)srcR.width()  / (float)newWidth;
	float ratio_y = (float)srcR.height() / (float)newHeight;

	for (ypos = y; ypos < y + newHeight; ypos++) {
		for (xpos = x; xpos < x + newWidth; xpos++) {
			xfloat = (float)(xpos - x) * ratio_x;
			yfloat = (float)(ypos - y) * ratio_y;

			x1 = srcXoffset + (int)xfloat;
			y1 = srcYoffset + (int)yfloat;

			xerr = 1.0 - (xfloat - (float)(x1 - srcXoffset));
			yerr = 1.0 - (yfloat - (float)(y1 - srcYoffset));

			rgb1 = lay -> pixel(static_cast<unsigned int>(x1), static_cast<unsigned int>(y1));
			r1 = qRed(rgb1);
			g1 = qGreen(rgb1);
			b1 = qBlue(rgb1);

			if (alpha) 
				a1 = qAlpha(rgb1);

			// do not exceed layer width with check
			// on right edge in source
			if (xpos < x + newWidth && x1 + 1 < srcWidth + srcXoffset) {
				rgb2 = lay -> pixel(static_cast<unsigned int>(x1 + 1), static_cast<unsigned int>(y1));
				r2 = qRed(rgb2);
				g2 = qGreen(rgb2);
				b2 = qBlue(rgb2);
			}
			else {
				r2 = r1; 
				g2 = g1; 
				b2 = b1;
			}
			// do not exceed layer width & height with check
			// at bottom right corner pixel in source (unique condition!)
			if (xpos < x + newWidth && ypos < y + newHeight && x1 + 1 < srcWidth + srcXoffset && y1 + 1 < srcHeight + srcYoffset) {
				rgb3 = lay -> pixel(static_cast<unsigned int>(x1 + 1), static_cast<unsigned int>(y1 + 1));
				r3 = qRed(rgb3);
				g3 = qGreen(rgb3);
				b3 = qBlue(rgb3);
			}
			else {
				r3 = r1; 
				g3 = g1; 
				b3 = b1;
			}

			// do not exceed layer height in check
			// along bottom row in source
			if (ypos < y + newHeight && y1 + 1 < srcHeight + srcYoffset) {
				rgb4 = lay -> pixel(static_cast<unsigned int>(x1), static_cast<unsigned int>(y1 + 1));
				r4 = qRed(rgb4);
				g4 = qGreen(rgb4);
				b4 = qBlue(rgb4);
			}
			else {
				r4 = r1; 
				g4 = g1; 
				b4 = b1;
			}

			r = (float)r1 * xerr + (float)r2 * (1.0 - xerr) + (float)r3 * (1.0 - xerr) + (float)r4 * xerr;
			r += (float)r1 * yerr + (float)r2 * yerr + (float)r3 * (1.0 - yerr) + (float)r4 * (1.0 - yerr);
			r *= 0.25;

			g = (float)g1 * xerr + (float)g2 * (1.0 - xerr) + (float)g3 * (1.0 - xerr) + (float)g4 * xerr;
			g += (float)g1 * yerr + (float)g2 * yerr + (float)g3 * (1.0 - yerr) + (float)g4 * (1.0 - yerr);
			g *= 0.25;

			b = (float)b1 * xerr + (float)b2 * (1.0 - xerr) + (float)b3 * (1.0 - xerr) + (float)b4 * xerr;
			b += (float)b1 * yerr + (float)b2 * yerr + (float)b3 * (1.0 - yerr) + (float)b4 * (1.0 - yerr);
			b *= 0.25;

			if (alpha)
				nlay -> setPixel(static_cast<int>(xpos - x), static_cast<int>(ypos - y), qRgba(r, g, b, a1));
			else
				nlay -> setPixel(static_cast<int>(xpos - x), static_cast<int>(ypos - y), qRgb(r, g, b));
		}
	}

	return true;
}

/*
    Scale area without smoothing colors - a must for indexed mode.
    This can produce jaggies when scaling up and inaccuracies in
    scaling down but when scaling down this is not so noticeable
*/
bool KisFrameBuffer::scaleRough(QRect & srcR, int newWidth, int newHeight)
{
	KisImage *img = pDoc->current();

	if (!img) 
		return false;

	KisLayer *lay = img->getCurrentLayer();

	if (!lay) 
		return false;

	QRect nr(0, 0, newWidth, newHeight);
	QString layerName;
	layerName.sprintf("layer %d", img->layerList().size());
	img->addLayer(nr, white, true, layerName);

	// adding a layer makes it the new current layer
	KisLayer *nlay = img->getCurrentLayer();
	
	if (!nlay) {
		kdDebug() << "scaleRough(): new layer not allocated!" << endl;
		return false;
	}

	bool alpha = (img->colorMode() == cm_RGBA);
	int srcXoffset = srcR.left();
	int srcYoffset = srcR.top();
	int x = 0, y = 0;
	int xpos = x, ypos = y;

	kdDebug() << "srcR.left() " << srcR.left() << "srcR.top() " << srcR.top()  << endl;

	float r, g, b, a;
	QRgb rgb;
	float x1, y1;
	float xfloat, yfloat;
	float ratio_x = (float)srcR.width()  / (float)newWidth;
	float ratio_y = (float)srcR.height() / (float)newHeight;

	for (ypos = y; ypos < y + newHeight; ypos++) {
		for (xpos = x; xpos < x + newWidth; xpos++) {
			xfloat = (xpos - x) * ratio_x;
			yfloat = (ypos - y) * ratio_y;

			x1 = srcXoffset + (int)xfloat;
			y1 = srcYoffset + (int)yfloat;

			rgb = lay -> pixel(static_cast<unsigned int>(x1), static_cast<unsigned int>(y1));
			nlay -> setPixel(xpos - x , ypos - y, rgb);
		}
	}

	return true;
}


bool KisFrameBuffer::mirror(QRect & )
{
    // add new layer
    // copy from end of src row data to
    // beginning of dest row for each row.

    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    return true;
}


bool KisFrameBuffer::flip(QRect & )
{
    // add new layer - same x and y sizes as src
    // copy last row in src to first row in dest for each row
    // data in each row should be same

    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    return true;
}


bool KisFrameBuffer::rotate90(QRect & )
{
    // add new layer - reverse x and y sizes
    // for each row in src, start at end of
    // row in dest and place data in reverse order

    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    return true;
}


bool KisFrameBuffer::rotate180(QRect & )
{
    // add new layer - same x and y sizes
    // start at top left of src  and copy data in
    // each row in reverse order from bottom of dest
    // to top

    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    return true;
}


bool KisFrameBuffer::rotate270(QRect & )
{
    // add new layer - reverse x and y sizes
    // start at top left of src and copy data in
    // each row in src to each column in dest

    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    return true;
}


bool KisFrameBuffer::QImageToLayer(QImage *, QRect & , QRect & )
{
    // use current layer only
    // copy from rectangle in QImage to rectangle in current layer
    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    return true;
}


bool KisFrameBuffer::layerToQImage(QImage *, QRect &, QRect &)
{
    // use current layer only
    // normally src and destination rectangles are same size
    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    return true;
}


bool KisFrameBuffer::changeColors(uint oldColor, uint newColor, QRect & r, KisSelection * /*selection*/)
{
	KisImage *img = pDoc->current();
	if (!img) return false;

	KisLayer *lay = img->getCurrentLayer();
	if (!lay) return false;

	QRect clipRect(r);

	if (!clipRect.intersects(lay->imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay->imageExtents());

	if (!clipRect.intersects(lay->layerExtents()))
		return false;

	clipRect = clipRect.intersect(lay->layerExtents());
	bool imageAlpha = (img->colorMode() == cm_RGBA);
	QRgb rgb;

	int oldRed   = qRed(oldColor);
	int oldGreen = qGreen(oldColor);
	int oldBlue  = qBlue(oldColor);

	int newRed   = qRed(newColor);
	int newGreen = qGreen(newColor);
	int newBlue  = qBlue(newColor);
	int newAlpha = qAlpha(newColor);

	int sx = clipRect.left();
	int sy = clipRect.top();
	int ex = clipRect.right();
	int ey = clipRect.bottom();

	for (int y = sy; y <= ey; y++) {
		for (int x = sx; x <= ex; x++) {
			if (oldRed == lay->pixel(x, y) && oldGreen == lay->pixel(x, y) && oldBlue == lay->pixel(x, y)) {
				if (mGradientPaint)
					setGradientToPixel(lay, x, y);
				else if (mPatternPaint)
					setPatternToPixel(lay, x, y, 0);
				else {
					lay -> setPixel(x, y, qRgb(newRed, newGreen, newBlue));
				}

				if (imageAlpha) {
					rgb = lay -> pixel(x, y);
					lay -> setPixel(x, y, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), newAlpha));
				}
			}
		}
	}

	return true;
}

/*
    setPenPattern - public method of setting the pen pattern
    externally.  This will almost always be the same as the
    view's current pattern, but it can be set to any pattern.
    Normally this is called when the view sets a new current
    pattern - kis_view.cc.

*/

void KisFrameBuffer::setPattern(KisPattern *pattern)
{
    pPenPattern = pattern;
    mPatternPaint = true;
}


/*
    setPatternToPixel - map a pixel in an imaginary tile of
    patterns to a pixel in the given layer and set the layer
    pixel value to the tile pixel value.

    This is used for drawing, or filling a selection or region,
    with a pattern.  Note:  value is the color of the pixel in the
    layer for blending with a pattern - later.
*/

void KisFrameBuffer::setPatternToPixel(KisLayer *lay, int _x, int _y, uint /*value*/)
{
	if (!pPenPattern)
		return;

	if (pPenPattern->width() == 0 || pPenPattern->height() == 0)
		return;

	int xTiles = lay->imageExtents().width() / pPenPattern->width();
	int yTiles = lay->imageExtents().height() / pPenPattern->height();
	int xOffset = lay->imageExtents().x();
	int yOffset = lay->imageExtents().y();
	int x = _x - xOffset;
	int y = _y - yOffset;

	// pixel value in pattern image scanline at x offset to right
	// not that we must take into account offset of layer to start
	// at topleft of pattern image

	uint *p = (uint *) pPenPattern -> image() -> scanLine(y / (yTiles * pPenPattern->height())
				+  y % pPenPattern->height())
		+ (x / (xTiles * pPenPattern->width()))
		+ (x % pPenPattern->width());

	lay -> setPixel(x, y, *p);
}

void KisFrameBuffer::setGradientPaint(bool _gradientPaint,
    KoColor _startColor, KoColor _endColor)
{
    KisImage *img = pDoc->current();
    if (!img) return;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return;

    mGradient.mapKdeGradient(QRect(0, 0,
        lay->imageExtents().width(), lay->imageExtents().height()),
        _startColor, _endColor);

    // set boolean value
    mGradientPaint = _gradientPaint;
}


void KisFrameBuffer::setGradientToPixel(KisLayer *lay,
    int x, int y)
{
    if(mGradient.width() == 0 || mGradient.height() == 0)
        return;

    uint u32Color = 0;

    int xOffset = lay->imageExtents().x();
    int yOffset = lay->imageExtents().y();

    // pixel value in gradient array
    u32Color = mGradient.imagePixelValue(x - xOffset, y - yOffset);

    lay -> setPixel(x, y, u32Color);
}

#include "kis_framebuffer.moc"

