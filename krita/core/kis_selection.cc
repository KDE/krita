/*
 *  kis_selection.cc - part of Krayon
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
#include "kis_selection.h"

/*
    KisSelection constructor - a selection is always associated
    with a document, but probably also needs a layer paramater
    as well - the default is the current layer
*/
KisSelection::KisSelection(KisDoc *doc)
{
    pDoc  = doc;
}


KisSelection::~KisSelection()
{
}

/*
    setBounds - set the containers for the selection. These include
    a rectangle, an array and a QImage.  Only the rectangle which bounds
    the selection is required.
*/
void KisSelection::setBounds(const QRect & re)
{
    // resize the selection rectangle
    rectangle = re;

    // set the array of points to same size as brush
    array.resize(rectangle.width() * rectangle.height());
    array.fill(0);

    // reallocate the image to be same size as rectangle
    // always use 32 bits for compatibility with layers
    image.create(rectangle.width(), rectangle.height(), 32);
    image.setAlphaBuffer(true);
}


/*
    setNull - resize all the containers for this selection to 0,
    but keep the containters.  No need to use pointers, new and delete.
*/
void KisSelection::setNull()
{
    // nullify selection rectangle
    rectangle.setWidth(0);
    rectangle.setHeight(0);

    // nullify selection array
    array.resize(0);
}

/*
    setImage - normally the image is constructed by the selection,
    but one can also be emplanted into a selection.  todo - size needs
    to be checked.  If not same size as selection rectangle, it needs
    to be clipped or scaled to fit.
*/
void KisSelection::setImage(QImage & img)
{
    image = img;
}


/*
    erase a selection rectange within a the current layer
    This should erase to the backgroud color or to complete
    transparency, depending on the layer
*/
bool KisSelection::erase()
{
    KisImage *img = pDoc->current();
    if (!img) return false;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    QRect clipRect = rectangle;

    if (!clipRect.intersects(lay->imageExtents()))
        return false;

    clipRect = clipRect.intersect(lay->imageExtents());

    int sx = clipRect.left();
    int sy = clipRect.top();
    int ex = clipRect.right();
    int ey = clipRect.bottom();

    uchar r, g, b;

    bool alpha = (img->colorMode() == cm_RGBA);

    for (int y = sy; y <= ey; y++)
    {
        for (int x = sx; x <= ex; x++)
        {
            // only erase pixels which are in the selection,
            // within the bounding rectangle
            if(array[(y - sy) * (ex - sx) + x - sx] != 1)
                continue;

            if (alpha)
            {
                //uchar a = lay->pixel(3, x, y);
                lay->setPixel(3, x, y, 0);
            }
            else {
                // destination binary values by channel
                r = lay->pixel(0, x,  y);
                g = lay->pixel(1, x,  y);
                b = lay->pixel(2, x,  y);

                int red = pDoc->currentView()->bgColor().R();
                int green = pDoc->currentView()->bgColor().G();
                int blue = pDoc->currentView()->bgColor().B();

                lay->setPixel(0, x, y, red);
                lay->setPixel(1, x, y, green);
                lay->setPixel(2, x, y, blue);
            }

        }
    }

    return true;
}


/*
    reverse - change all selected pixels to unselected
    and all unseleted to selected
*/
void KisSelection::reverse()
{
    for(uint i = 0; i < array.count(); i++)
    {
        if(array[i] == 0)  array[i] = 1;
        else array[i] = 0;
    }
}


/*
    fill - fill the selection with a color, gradient and pattern
    This is normally done with selections in place in a given
    layer, the current one, but it can also be done with any
    layer.
*/
void KisSelection::fill(uint color,
    KisPattern * /*pattern*/, KisGradient * /*gradient*/)
{
    KisImage *img = pDoc->current();
    if (!img) return;

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return;

    QRect clipRect = rectangle;

    if (!clipRect.intersects(lay->imageExtents()))
        return;

    clipRect = clipRect.intersect(lay->imageExtents());

    int sx = clipRect.left();
    int sy = clipRect.top();
    int ex = clipRect.right();
    int ey = clipRect.bottom();

    uchar red = qRed(color);
    uchar green = qGreen(color);
    uchar blue = qBlue(color);

    bool alpha = (img->colorMode() == cm_RGBA);
    bool colorBlending = false;

    for (int y = sy; y <= ey; y++)
    {
        for (int x = sx; x <= ex; x++)
	    {
            // only change pixels which are in the selection,
            // within the bounding rectangle
            if(array[(y - sy) * (ex - sx) + x - sx] != 1)
                continue;

            // destination binary values by channel
            if(colorBlending)
            {

	            //uchar r = lay->pixel(0, x,  y);
	            //uchar g = lay->pixel(1, x,  y);
	            //uchar b = lay->pixel(2, x,  y);

                // .... blending method - todo
                // need blend(uint *source, uint *dest);
                // sets destination pixel depending on
                // blending algorithm
            }

            // change all pixels in selection to
            // the desired color if blending is not used
	        lay->setPixel(0, x, y, red);
	        lay->setPixel(1, x, y, green);
	        lay->setPixel(2, x, y, blue);

            // don't change alpha values, for now
            if (alpha)
	        {
	            uchar a = lay->pixel(3, x, y);
                lay->setPixel(3, x, y, a);
	        }
	    }
    }
}


void KisSelection::setRectangularSelection(QRect & re, KisLayer *lay)
{
    setBounds(re);

    QRect clipRect = rectangle;

    if (!clipRect.intersects(lay->imageExtents()))
        return;

    clipRect = clipRect.intersect(lay->imageExtents());

    int sx = clipRect.left();
    int sy = clipRect.top();
    int ex = clipRect.right();
    int ey = clipRect.bottom();

    uchar r, g, b;

    bool alpha = (pDoc->current()->colorMode() == cm_RGBA);

    for (int y = sy; y <= ey; y++)
    {
        for (int x = sx; x <= ex; x++)
	    {
            // for a rectangular selection, all pixels
            // in the rectangle bounding it are selected
            array[(y - sy) * (ex - sx) + x - sx] = 1;

            // destination binary values by channel
	        r = lay->pixel(0, x, y);
	        g = lay->pixel(1, x, y);
	        b = lay->pixel(2, x, y);

            if(alpha)
            {
	            uchar a = lay->pixel(3, x, y);
	            image.setPixel(x - sx, y - sy, qRgba(r, g, b, a));
            }
            else
            {
	            image.setPixel(x - sx, y - sy, qRgb(r, g, b));
            }
	    }
    }
}


void KisSelection::setEllipticalSelection(QRect & re, KisLayer *lay)
{
    // set the bounding selection rectangle
    setBounds(re);

    // construct a solid white pixmap same size as the selection
    QPixmap pix(rectangle.width(), rectangle.height());
    pix.fill();

    // draw the filled ellipse onto that pixmap in black
    QPainter p(&pix);

    /* constructs a pen.  color is always black - it is
    changed by krayon to the actual colors of each pixel
    in the ellipse in the layer below */
    QPen pen(Qt::black);
    p.setPen(pen);

    // constructs a solid brush - we want to fill the ellipse
    QBrush brush(Qt::black);
    p.setBrush(brush);

    // draw filled ellipse bounded by rectangle
    QRect ellipseRectangle(0, 0, pix.width(), pix.height());
    p.drawEllipse(ellipseRectangle);

    // convert the pixmap to an image so we can access scanlines
    // always 32 bit with alpha channel
    QImage pixImage = pix.convertToImage();
    pixImage.convertDepth(32);
    pixImage.setAlphaBuffer(true);

    // now, get the colors of each pixel from the layer if the
    // pixels lie within the selection ellipse, and transfer those
    // to the image and the mark selected pixels in the array
    QRect clipRect = rectangle;

    if (!clipRect.intersects(lay->imageExtents()))
        return;

    clipRect = clipRect.intersect(lay->imageExtents());

    int sx = clipRect.left();
    int sy = clipRect.top();
    int ex = clipRect.right();
    int ey = clipRect.bottom();

    uchar r, g, b;
    uchar a = 255;

    bool layerAlpha = (pDoc->current()->colorMode() == cm_RGBA);

    for (int y = sy; y <= ey; y++)
    {
        for (int x = sx; x <= ex; x++)
	    {
            // pixel value in scanline at x offset to right
            // in terms of the image - accounting for offset into
            // layer, image offset is always 0,0
            uint *p = (uint *)pixImage.scanLine(y - sy) + (x - sx);

            // destination binary values by channel
	        r = lay->pixel(0, x, y);
	        g = lay->pixel(1, x, y);
	        b = lay->pixel(2, x, y);

            /* if pixel has the white background filler color,
            set the alpha channel value in the image to 0,
            indicating that this pixel is transparent */
            if(QColor(*p) != Qt::black)
            {
                /* mark pixel as unselected in the array
                The position in array is number of lines times
                width of a line plus offset into current line */
                array[(y - sy) * (ex - sx) + x - sx] = 0;

                // this pixel is transparent
	            a = 0;

                // set image colors to layer colors
	            image.setPixel(x - sx, y - sy, qRgba(r, g, b, a));
            }
            /* the pixel in the mask image is black, so it's in
            the selection. Still, if the alpha value of the layer
            pixel is 0, the image pixel is still painted transparent
            and technically is not in the selection unless the array
            is used as a mask when pasting.  This can't be used
            reliably with the global kapp->clipboard() without adding
            a custom data type which makes images from other apps
            unusable. At the worst this results in transparent pixels
            from the image not getting painted, which is the whole
            reason to use the alpha channel here, so everything is ok */
            else
            {
                // mark pixel as selected in the array
                array[(y - sy) * (ex - sx) + x - sx] = 1;

                // get alpha value from layer
                if(layerAlpha) a = lay->pixel(3, x, y);

                // set image colors to layer color values
	            image.setPixel(x - sx, y - sy, qRgba(r, g, b, a));
            }
	    }
    }
}



void KisSelection::setPolygonalSelection( QRect & re, QPointArray & pointsArray, KisLayer *lay )
{
    // set the bounding selection rectangle
    setBounds( re );

    // construct a solid white pixmap same size as the selection
    QPixmap pix( rectangle.width(), rectangle.height() );
    pix.fill();

    // draw the filled polygon onto that pixmap in black
    QPainter p( &pix );

    /* constructs a pen.  color is always black - it is
    changed by krayon to the actual colors of each pixel
    in the ellipse in the layer below */
    QPen pen( Qt::black );
    p.setPen( pen );

    // constructs a solid brush - we want to fill the polygon
    QBrush brush( Qt::black );
    p.setBrush( brush );

    // draw filled polygons bounded by rectangle
    QPoint topLeft = rectangle.topLeft();
    QPointArray points = getBundedPointArray( pointsArray, topLeft );
    p.drawPolygon( points );

    // convert the pixmap to an image so we can access scanlines
    // always 32 bit with alpha channel
    QImage pixImage = pix.convertToImage();
    pixImage.convertDepth( 32 );
    pixImage.setAlphaBuffer( true );

    // now, get the colors of each pixel from the layer if the
    // pixels lie within the selection ellipse, and transfer those
    // to the image and the mark selected pixels in the array
    QRect clipRect = rectangle;

    if ( !clipRect.intersects( lay->imageExtents() ) )
        return;

    clipRect = clipRect.intersect( lay->imageExtents() );

    int sx = clipRect.left();
    int sy = clipRect.top();
    int ex = clipRect.right();
    int ey = clipRect.bottom();

    uchar r, g, b;
    uchar a = 255;

    bool layerAlpha = ( pDoc->current()->colorMode() == cm_RGBA );

    for ( int y = sy; y <= ey; ++y ) {
        for ( int x = sx; x <= ex; ++x ) {
            // pixel value in scanline at x offset to right
            // in terms of the image - accounting for offset into
            // layer, image offset is always 0,0
            uint *p = (uint *)pixImage.scanLine( y - sy ) + ( x - sx );

            // destination binary values by channel
            r = lay->pixel( 0, x, y );
            g = lay->pixel( 1, x, y );
            b = lay->pixel( 2, x, y );

            /* if pixel has the white background filler color,
            set the alpha channel value in the image to 0,
            indicating that this pixel is transparent */
            if( QColor( *p ) != Qt::black ) {
                /* mark pixel as unselected in the array
                The position in array is number of lines times
                width of a line plus offset into current line */
                array[ (y - sy) * (ex - sx) + x - sx ] = 0;

                // this pixel is transparent
                a = 0;

                // set image colors to layer colors
                image.setPixel( x - sx, y - sy, qRgba( r, g, b, a ) );
            }
            /* the pixel in the mask image is black, so it's in
            the selection. Still, if the alpha value of the layer
            pixel is 0, the image pixel is still painted transparent
            and technically is not in the selection unless the array
            is used as a mask when pasting.  This can't be used
            reliably with the global kapp->clipboard() without adding
            a custom data type which makes images from other apps
            unusable. At the worst this results in transparent pixels
            from the image not getting painted, which is the whole
            reason to use the alpha channel here, so everything is ok */
            else {
                // mark pixel as selected in the array
                array[ (y - sy) * (ex - sx) + x - sx ] = 1;

                // get alpha value from layer
                if( layerAlpha ) a = lay->pixel( 3, x, y );

                // set image colors to layer color values
                image.setPixel(x - sx, y - sy, qRgba(r, g, b, a));
            }
        }
    }
}

// Pixmap size of top left point is (0, 0). But original size of
// top left is rectangle.topLeft(). Therefore we do point.x()-topLeft.x(),
// point.y()-topLeft.y().
QPointArray KisSelection::getBundedPointArray( QPointArray & points, QPoint & topLeft )
{
    QPointArray m_points( points.size() );
    int count = 0;

    QPointArray::Iterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        QPoint point = *it;
        int x = point.x() - topLeft.x();
        int y = point.y() - topLeft.y();

        m_points.setPoint( count, QPoint( x, y ) );

        ++count;
    }

    return m_points;
}

void KisSelection::setContiguousSelection(QRect & re, KisLayer *lay)
{
    // set the bounding selection rectangle
    setBounds(re);

    // construct a solid white pixmap same size as the selection
    QPixmap pix(rectangle.width(), rectangle.height());
    pix.fill();

    // draw the filled ellipse onto that pixmap in black
    QPainter p(&pix);

    /* constructs a pen.  color is always black - it is
    changed by krayon to the actual colors of each pixel
    in the ellipse in the layer below */
    QPen pen(Qt::black);
    p.setPen(pen);

    // constructs a solid brush - we want to fill the ellipse
    QBrush brush(Qt::black);
    p.setBrush(brush);

    // draw ellipse bounded by rectangle
    QRect ellipseR(0, 0, pix.width(), pix.height());
    p.drawEllipse(ellipseR);

    // convert the pixmap to an image so we can access scanlines
    QImage pixImage = pix.convertToImage();

    // now, get the colors of each pixel from the layer if the
    // pixels lie within the selection ellipse, and transfer those
    // to the image and the mark selected pixels in the array
    QRect clipRect = rectangle;

    if (!clipRect.intersects(lay->imageExtents()))
        return;

    clipRect = clipRect.intersect(lay->imageExtents());

    int sx = clipRect.left();
    int sy = clipRect.top();
    int ex = clipRect.right();
    int ey = clipRect.bottom();

    uchar r, g, b;

    bool alpha = (pDoc->current()->colorMode() == cm_RGBA);

    for (int y = sy; y <= ey; y++)
    {
        for (int x = sx; x <= ex; x++)
	    {
            // for a rectangular selection, all pixels
            // in the rectangle bounding it are selected
            array[(y - sy) * (ex - sx) + x - sx] = 1;

            // destination binary values by channel
	        r = lay->pixel(0, x,  y);
	        g = lay->pixel(1, x,  y);
	        b = lay->pixel(2, x,  y);

            if(alpha)
            {
	            uchar a = lay->pixel(3, x, y);
	            image.setPixel(x - sx, y - sy, qRgba(r, g, b, a));
            }
            else
            {
	            image.setPixel(x - sx, y - sy, qRgb(r, g, b));
            }
	    }
    }
}


#include "kis_selection.moc"

