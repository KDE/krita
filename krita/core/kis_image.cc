/*
 *  kis_image.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <assert.h>

#include <qpainter.h>

#include <klocale.h>
#include <kdebug.h>

#include "kis_factory.h"
#include "kis_layer.h"
#include "kis_util.h"
#include "kis_brush.h"
#include "kis_global.h"
#include "kis_timer.h"
#include "kis_image.h"
#include "KIsImageIface.h"

#define KIS_DEBUG(AREA, CMD)

KisImage::KisImage(const QString& name, int w, int h, cMode cm, uchar bd)
	: m_name(name),
	m_width(w),
	m_height(h),
	m_cMode(cm),
	m_bitDepth(bd)
{
	bd = m_bitDepth = 32; // XXX
	m_author = i18n("The Krita team");
	m_email = "kimageshop@mail.kde.org";
	m_autoUpdate = true;

	m_dcop = 0;
	//dcopObject(); // build it

	QRect tileExtents = KisUtil::findTileExtents(QRect(0, 0, m_width, m_height));

	m_xTiles = tileExtents.width() / TILE_SIZE;
	m_yTiles = tileExtents.height() / TILE_SIZE;

	// setup dirty flag array
	m_dirty.resize(m_xTiles * m_yTiles);

	// no tiles are marked dirty to start
	for(int y = 0; y < m_yTiles; y++)
		for(int x = 0; x < m_xTiles; x++)
			m_dirty[y * m_xTiles + x] = false;

	//QPixmap::setDefaultOptimization( QPixmap::NoOptim );
	m_pixmapTiles = new QPixmap*[m_xTiles * m_yTiles];

	// allocate the pixmaps for the tiles
	for (int y = 0; y < m_yTiles; y++)
		for (int x = 0; x < m_xTiles; x++) {
			m_pixmapTiles[y * m_xTiles + x] = new QPixmap(TILE_SIZE, TILE_SIZE);
			m_pixmapTiles[y * m_xTiles + x] -> fill();
		}

	m_imgTile.create(TILE_SIZE, TILE_SIZE, m_bitDepth);
	m_activeLayer = 0;

	m_composeLayer = new KisLayer("_compose", TILE_SIZE, TILE_SIZE, bd, cm);
	m_composeLayer -> allocateRect(QRect(0, 0, TILE_SIZE, TILE_SIZE));

	m_bgLayer = new KisLayer("_background", TILE_SIZE, TILE_SIZE, bd, cm);
	m_bgLayer -> allocateRect(QRect(0, 0, TILE_SIZE, TILE_SIZE));

	// FIXME: make it work with non-RGB color spaces
	// make it work with BigEndian! - john
	uint *ptr = m_bgLayer -> getTile(0, 0) -> data();

	for (int y = 0; y < TILE_SIZE; y++)
		for (int x = 0; x < TILE_SIZE; x++) {
			uchar v = 128 + 63 * ((x / 16 + y / 16) % 2);

			*(ptr + (y * TILE_SIZE) + x) = qRgba(v, v, v, 255);
		}

	compositeImage();

	/* note - 32 bit pixmaps may not show up on a 16 bit display
	   and will even crash it !!!  Without depth paramater Qt will
	   use native format. imagePixmap = new QPixmap(w, h, 32); does
	   not work!, so alway default to native display depth when
	   creating a QPixmap.  With a QImage you can create a 32 bit
	   depth with a 16 bit display with no problems, however. */

#if 0
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotUpdateTimeOut()));
	m_timer -> start(1);
#endif
}

/*
    KisImage destructor - note that various arrays must be
    deleted as these are not children of the QObject for
    KisImage, but are regular arrays and pointers to them.
*/

KisImage::~KisImage()
{
	for (int y = 0; y < m_yTiles; y++)
		for (int x = 0; x < m_xTiles; x++)
			delete m_pixmapTiles[y * m_xTiles + x];

	delete[] m_pixmapTiles;
	delete m_composeLayer;
	delete m_bgLayer;
        delete m_dcop;
	// XXX m_layers;
	// XXX m_channels;
}


DCOPObject* KisImage::dcopObject()
{
    if (!m_dcop)
	m_dcop = new KIsImageIface(this);

    return m_dcop;
}


void KisImage::upperLayer(unsigned int layer)
{
	Q_ASSERT(layer < m_layers.count());

	if (layer > 0) {
		KisLayer *pLayer = m_layers.take(layer);

		m_layers.insert(layer - 1, pLayer);
	}
}

void KisImage::lowerLayer(unsigned int layer)
{
	Q_ASSERT(layer < m_layers.count());

	if (layer < m_layers.count() - 1) {
		KisLayer *pLayer = m_layers.take(layer);
		m_layers.insert(layer + 1, pLayer);
	}
}

void KisImage::setFrontLayer(unsigned int layer)
{
	Q_ASSERT(layer < m_layers.count());

	if (layer < m_layers.count() - 1) {
		KisLayer *pLayer = m_layers.take(layer);

		m_layers.append(pLayer);
	}
}

void KisImage::setBackgroundLayer(unsigned int layer)
{
	Q_ASSERT(layer < m_layers.count());

	if (layer > 0) {
		KisLayer *pLayer = m_layers.take(layer);

		m_layers.insert(0, pLayer);
	}
}

void KisImage::addLayer(const QRect& rect, const KisColor& c, bool tr, const QString& name)
{
	KisLayer *lay = new KisLayer(name, m_width, m_height, m_bitDepth, m_cMode);

	lay -> allocateRect(rect);
	lay -> clear(c, tr);
	m_layers.append(lay);
	m_activeLayer = lay;
	resizePixmap(lay, rect);
}

void KisImage::removeLayer(unsigned int layer)
{
	if (layer >= m_layers.count())
		return;

	KisLayer *lay = m_layers.take(layer);

	if (m_activeLayer == lay) {
		if (m_layers.count())
			m_activeLayer = m_layers.at(0);
		else
			m_activeLayer = 0;
	}

	delete lay;
}

/*
    markDirty - this is a public method for drawing and painting
    tools, etc., to use to indicate rectantular areas that have
    been written to, usually using layer->setPixel(), so that they
    can then be rendered to the display when the update timer sends
    a timeout signal. See below..
*/

void KisImage::markDirty(const QRect& r)
{
	QRect rc = KisUtil::findBoundingTiles(r);
	int maxY = rc.bottom() < m_yTiles ? rc.bottom() + 1 : m_yTiles;
	int maxX = rc.right() < m_xTiles ? rc.right() + 1 : m_xTiles;

	for (int y = rc.top(); y < maxY; y++) {
		for (int x = rc.left(); x < maxX; x++) {
//			Q_ASSERT(m_pixmapTiles[y * m_xTiles + x]);
			m_dirty[y * m_xTiles + x] = true;

			if (m_autoUpdate)
				compositeImage(QRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE));
			else
				m_dirtyTiles.append(m_pixmapTiles[y * m_xTiles + x]);
		}
	}
}

/*
    slotUpdateTimeOut - invoked by timeout signal from the
    timer for this image.  Those tiles which are "dirty"
    are then rendered into a QImage and converted to a QPixmpa,
    one at a time, for display to screen paint device, which
    is normall the kisCanvas() for the view.
*/

#if 0
void KisImage::slotUpdateTimeOut()
{
	// TODO : Go over tiles in m_dirtyTiles
	// if tile is dirty,
	for(int y = 0; y < m_yTiles; y++)
		for(int x = 0; x < m_xTiles; x++)
			if (m_dirty[y * m_xTiles + x])
				compositeImage(QRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE));
}
#endif

void KisImage::paintContent(QPainter& painter, const QRect& rect, bool /*transparent*/)
{
	paintPixmap(&painter, rect);
}

/*
    Paint the pixmap of each tile which falls within the given
    rectangle onto the QPainter passed in - normally this is the
    kisCanvas - the main widget in the client area of the view
    but it can be another painter as well, such as another pixmap
    for the entire image or the area to be updated.
*/

void KisImage::paintPixmap(QPainter *p, const QRect& area)
{
	if (!p)
		return;

	int startX, startY, pixX, pixY, clipX, clipY = 0;
	int l = area.left();
	int t = area.top();
	int r = area.right();
	int b = area.bottom();
	QRect rc = findBoundingTiles(area);

	for (int y = rc.top(); y < rc.bottom(); y++) {
		for (int x = rc.left(); x < rc.right(); x++) {
			int xt = x * TILE_SIZE;
			int yt = y * TILE_SIZE;

			if (xt < l) {
				startX = 0;
				pixX = l - xt;
			}
			else {
				startX = xt - l;
				pixX = 0;
			}

			clipX = r - xt - pixX;

			if (yt < t) {
				startY = 0;
				pixY = t - yt;
			}
			else {
				startY = yt - t;
				pixY = 0;
			}

			clipY = b - yt - pixY;

			if (clipX <= 0)
				clipX = -1;

			if (clipY <= 0)
				clipY = -1;

			p -> drawPixmap(startX, startY, *m_pixmapTiles[y * m_xTiles + x], pixX, pixY, clipX, clipY);
		}
	}
}

/*
    Constructs the composite image in the tile at x,y
    and updates the relevant pixmap
*/

void KisImage::compositeTile(int x, int y, KisLayer *dstLay, int dstTile)
{
	KisTile *dst = dstLay -> getTile(0, 0);

	memcpy(dst -> data(), m_bgLayer -> getTile(0, 0) -> data(), TILE_SIZE * TILE_SIZE * sizeof(unsigned int));

	for (KisLayer *lay = m_layers.first(); lay; lay = m_layers.next()) {
		if (lay && lay -> visible()) {
			KisTile *src = lay -> getTile(x, y);

			Q_ASSERT(src);
			renderTile(dst, src, lay);
		}
	}
}

void KisImage::compositeImage(const QRect& area)
{
	QRect rc = findBoundingTiles(area);

	for (int y = rc.top(); y < rc.bottom(); y++) {
		for (int x = rc.left(); x < rc.right(); x++) {
			if (m_dirty[y * m_xTiles + x] == false)
				continue;

			// set the alpha channel to opaque in the compose layer
			// if this is not done the colors from the different layers
			// won't show up
//		        if (m_cMode == cm_RGBA)
//				memset(m_composeLayer -> channelMem(3, 0, 0, 0), 255, TILE_SIZE * TILE_SIZE);

			// for this tile, get all the tiles from different layers
			// which correspond and add them in (if they are visible, etc.)
			// to the compose layer - only this layer is used to draw
			// the image on canvas, after converting it to pixmaps
			// There is one pixmap for each tile
			compositeTile(x, y, m_composeLayer, 0);

			// convert only this tile to a pixmap for purposes of
			// drawing the pixmap on the canvas
			convertTileToPixmap(m_composeLayer, 0, m_pixmapTiles[y * m_xTiles + x]);
			m_dirty[y * m_xTiles + x] = false;
		}
	}

	emit updated(area);
}

/*
    Renders the part of srcLay which resides in dstTile of dstLay
*/

void KisImage::renderLayerIntoTile(QRect tileBoundary, const KisLayer *srcLay,
          KisLayer *dstLay, int dstTile)
{
#if 0
    int tileNo, tileOffsetX, tileOffsetY, xTile, yTile;

    //puts("renderLayerIntoTile");

    srcLay->findTileNumberAndPos(tileBoundary.topLeft(), &tileNo,
								 &tileOffsetX, &tileOffsetY);
    xTile=tileNo%srcLay->xTiles();
    yTile=tileNo/srcLay->xTiles();
    KIS_DEBUG(render, showi(tileNo); );

    bool renderQ1=true, renderQ2=true, renderQ3=true, renderQ4=true;

    if (tileOffsetX<0)
        renderQ1=renderQ3=false;
    if (tileOffsetY<0)
        renderQ2=renderQ1=false;

    KIS_DEBUG(render, showi(tileOffsetX); );
    KIS_DEBUG(render, showi(tileOffsetY); );

    int maxLayerX=TILE_SIZE, maxLayerY=TILE_SIZE;

    if (srcLay->boundryTileX(tileNo))
    {
        maxLayerX=srcLay->channelLastTileOffsetX();
        if (tileOffsetX>=0)
            renderQ2=false;
        KIS_DEBUG(render, showi(maxLayerX); );
    }

    if (tileOffsetX==0)
        renderQ4=false;

    if (srcLay->boundryTileY(tileNo))
    {
        maxLayerY=srcLay->channelLastTileOffsetY();

	    if (tileOffsetY>=0)
	        renderQ3=false;
	    KIS_DEBUG(render, showi(maxLayerX); );
    }

    if (tileOffsetY==0)
        renderQ4=false;

    KIS_DEBUG(render, showi(renderQ1); );
    KIS_DEBUG(render, showi(renderQ2); );
    KIS_DEBUG(render, showi(renderQ3); );
    KIS_DEBUG(render, showi(renderQ4); );

    // Render quadrants of each tile (either 1, 2 or 4 quadrants get rendered)
    //
    //  ---------
    //  | 1 | 2 |
    //  ---------
    //  | 3 | 4 |
    //  ---------
    //

    KIS_DEBUG(render, {SHOW_POINT(tileBoundary.topLeft());
    printf("tileNo %d, tileOffsetX %d, tileOffsetY %d\n",
	 tileNo, tileOffsetX, tileOffsetY);  });

    int renderedToX, renderedToY;

    KIS_DEBUG(render, printf("Test 1: "); );

    if (renderQ1)
    {
        // true => render 1
        renderTileQuadrant(srcLay, tileNo, dstLay, dstTile,
            tileOffsetX, tileOffsetY, 0, 0,
	    TILE_SIZE, TILE_SIZE);

        renderedToX=maxLayerX-tileOffsetX;
        renderedToY=maxLayerY-tileOffsetY;
    }
    else
        KIS_DEBUG(render, puts("ignore"); );

    KIS_DEBUG(render, printf("Test 2:"); );

    if (renderQ2)
    {
        // true => render 2
        if (renderQ1)
            renderTileQuadrant(srcLay, tileNo+1, dstLay, dstTile,
			 0, tileOffsetY, maxLayerX-tileOffsetX, 0,
			 TILE_SIZE, TILE_SIZE);
        else
            renderTileQuadrant(srcLay, tileNo, dstLay, dstTile,
			 0, tileOffsetY, -tileOffsetX,0,
			 TILE_SIZE, TILE_SIZE);
    }
    else
        KIS_DEBUG(render, puts("ignore"));

    KIS_DEBUG(render, printf("Test 3:"); );

    if (renderQ3)
    {
        // true => render 3
        if (renderQ1)
            renderTileQuadrant(srcLay, tileNo+srcLay->xTiles(), dstLay, dstTile,
	        tileOffsetX, 0, 0, maxLayerY-tileOffsetY,
	        TILE_SIZE, TILE_SIZE);
        else
            renderTileQuadrant(srcLay, tileNo, dstLay, dstTile,
		 tileOffsetX, 0, 0, -tileOffsetY,
		 TILE_SIZE, TILE_SIZE);
    }
    else
        KIS_DEBUG(render, puts("ignore"); );

    KIS_DEBUG(render, printf("Test 4:"); );

    // true => render 4
    if (renderQ4)
    {
        int newTile=tileNo;
        KIS_DEBUG(render, showi(xTile); );
        KIS_DEBUG(render, showi(yTile); );
        if (renderQ1)
        {
            xTile++; yTile++; newTile+=srcLay->xTiles()+1;
        }
        else
        {
            if (renderQ2) { yTile++; newTile+=srcLay->xTiles(); }
	    if (renderQ3) { xTile++; newTile+=1; }
        }

        KIS_DEBUG(render, showi(xTile); );
        KIS_DEBUG(render, showi(yTile); );

        if ((xTile<srcLay->xTiles()) && (yTile<srcLay->yTiles()))
        {
            KIS_DEBUG(render, showi(newTile); );
            if (!(renderQ1 && !renderQ2 && !renderQ3))
            {
		        if (tileOffsetX>0) tileOffsetX=tileOffsetX-TILE_SIZE;
		        if (tileOffsetY>0) tileOffsetY=tileOffsetY-TILE_SIZE;
		        renderTileQuadrant(srcLay, newTile, dstLay, dstTile,											 0, 0, -tileOffsetX, -tileOffsetY,
		            TILE_SIZE, TILE_SIZE);
            }
        }
        else
            KIS_DEBUG(render, puts("ignore"); );
    }
    else
        KIS_DEBUG(render, puts("ignore"); );
#endif
}




void KisImage::renderTileQuadrant(const KisLayer *srcLay, int srcTile,
	KisLayer *dstLay, int dstTile,
	int srcX, int srcY, int dstX, int dstY, int w, int h)
{
#if 0
    for (uchar i = 0; i < srcLay->numChannels(); i++)
	if (srcLay->channelMem(i, srcTile, 0, 0) == 0) return;

    uchar opacity=srcLay->opacity();

    // Constrain the width so that the copy is clipped to the overlap
    w = kMin(kMin(w, TILE_SIZE-srcX), TILE_SIZE-dstX);
    h = kMin(kMin(h, TILE_SIZE-srcY), TILE_SIZE-dstY);

    // now constrain if on the boundry of the layer
    if (srcLay->boundryTileX(srcTile))
        w = kMin(w, srcLay->channelLastTileOffsetX()-srcX);
    if (srcLay->boundryTileY(srcTile))
        h = kMin(h, srcLay->channelLastTileOffsetY()-srcY);

    // XXX now constrain for the boundry of the Canvas

    int leadIn=(TILE_SIZE-w);

    // FIXME:: Make it work for non-RGB modes
    /* this may need to be adjusted for big-endian systems */

    uchar *dptr0 = dstLay->channelMem(0, dstTile, dstX, dstY);
    uchar *dptr1 = dstLay->channelMem(1, dstTile, dstX, dstY);
    uchar *dptr2 = dstLay->channelMem(2, dstTile, dstX, dstY);
    uchar *dptr3 = 0;
    if (m_cMode == cm_RGBA)
	    dptr3 = dstLay->channelMem(3, dstTile, dstX, dstY);

    uchar *sptr0 = srcLay->channelMem(0, srcTile, srcX, srcY);
    uchar *sptr1 = srcLay->channelMem(1, srcTile, srcX, srcY);
    uchar *sptr2 = srcLay->channelMem(2, srcTile, srcX, srcY);
    uchar *sptr3 = 0;
    if (m_cMode == cm_RGBA)
	    sptr3 = srcLay->channelMem(3, srcTile, srcX, srcY);

    uchar opac,invOpac;
    for(int y = h; y; y--)
    {
        for(int x = w; x; x--)
	    {
	        // for prepultiply => invOpac = 255 - (*alpha*opacity)/255;

	        if (m_cMode == cm_RGBA)
	        {
		        opac = (*sptr3*opacity)/255;
		        invOpac=255-opac;

		        *dptr0++ = (((*dptr0 * *dptr3)/255) * invOpac + *sptr0++ * opac)/255;
		        *dptr1++ = (((*dptr1 * *dptr3)/255) * invOpac + *sptr1++ * opac)/255;
		        *dptr2++ = (((*dptr2 * *dptr3)/255) * invOpac + *sptr2++ * opac)/255;
		        *dptr3++ = *sptr3 + *dptr3 - (*sptr3 * *dptr3)/255;
		        sptr3++;
	        }
            else
	        {
		        invOpac = 255-opacity;
		        *dptr0++ = (*dptr0  * invOpac + *sptr0++ * opacity)/255;
		        *dptr1++ = (*dptr1  * invOpac + *sptr1++ * opacity)/255;
		        *dptr2++ = (*dptr2  * invOpac + *sptr2++ * opacity)/255;
	        }
	    }

	    dptr0 += leadIn;
	    dptr1 += leadIn;
	    dptr2 += leadIn;
	    sptr0 += leadIn;
	    sptr1 += leadIn;
	    sptr2 += leadIn;

	    if (m_cMode == cm_RGBA)
	    {
	        dptr3 += leadIn;
	        sptr3 += leadIn;
	    }
    }
#endif
}

/*
    layerPtr - internal method for assuring a valid layer
    by defaulting to the current layer if the given layer
    is null.
*/

#if 0
KisLayer* KisImage::layerPtr(KisLayer *layer)
{
	return layer ? layer : m_activeLayer;
}
#endif

/*
    setCurrentLayer - sets the current layer to the given
    index.  There also needs to be a method accepting a
    layer ptr instead of an index for setting current layer.
*/

void KisImage::setCurrentLayer(int layer)
{
	KisLayer *p = m_layers.at(layer);

	if (p)
		m_activeLayer = p;
}

/*
    convertImageToPixmap - an attempt to optimize and improve
    Qt's pix->convertFromImage() using hardware-specific hacks.
    This seemed to cause problems for big endian machines so
    if big endian architecture is detetected it now uses "unknown"
    visual - the standard Qt method.  This change should eliminate
    big endian display problems. (Note:  In any case, the "optimized"
    methods do not seem any faster, or slower, than the standard
    ones used by Qt).
*/
void KisImage::convertImageToPixmap(QImage *image, QPixmap *pix)
{
	if (image && pix)
		pix -> convertFromImage(*image);
}

void KisImage::convertTileToPixmap(KisLayer *lay, int tileNo, QPixmap *pix)
{
	KisTile *tile = lay -> getTile(0, 0);
	uint *p = tile -> data();

	for (int y = 0; y < TILE_SIZE; y++) {
		uint *ptr = (uint *)m_imgTile.scanLine(y);

		for (int x = TILE_SIZE; x; x--)
			*ptr++ = *p++;
	}

	// Construct the relevant pixmap
	convertImageToPixmap(&m_imgTile, pix);
}

/*
    mergeAllLayers - merge all layers into the background or first
    layer and delete them after they are merged, leaving only the
    first layer.
*/
void KisImage::mergeAllLayers()
{
    QPtrList<KisLayer> l;
    KisLayer *lay = m_layers.first();

    while(lay)
    {
        l.append(lay);
        lay = m_layers.next();
    }

    mergeLayers(l);
}

/*
    mergeVisibleLayers - merge only those layers which are visible.
    Merge everything into the first visible layer encountered.
*/
void KisImage::mergeVisibleLayers()
{
    QPtrList<KisLayer> l;

    KisLayer *lay = m_layers.first();

    while(lay)
    {
        if(lay->visible())
	    l.append(lay);
        lay = m_layers.next();
    }

    mergeLayers(l);
}

/*
    mergeLinkeLayers - merge only those layers linked.
    Merge everything into the first linked layer encountered.
*/
void KisImage::mergeLinkedLayers()
{
    QPtrList<KisLayer> l;

    KisLayer *lay = m_layers.first();

    while(lay)
    {
        if (lay->linked())
	        l.append(lay);
        lay = m_layers.next();
    }

    mergeLayers(l);
}

/*
    mergeLayers - merge all layers in the given list into the
    first layer in the list, and delete the others.  Normally
    this renders everything into the background layer.  There
    needs to be a paramater and option for not deleting layers
    merged into others but keeping them instead.
*/
void KisImage::mergeLayers(QPtrList<KisLayer> list)
{
    list.setAutoDelete(false);

    KisLayer *a, *b;
    QRect newRect;

    a = list.first();
    while(a)
    {
        newRect.unite(a->imageExtents());
        //a->renderOpacityToAlpha();
        a = list.next();
    }

    while((a = list.first()) && (b = list.next()))
    {
        if (!a || !b) break;
        QRect ar = a->imageExtents();
        QRect br = b->imageExtents();

        QRect urect = ar.unite(br);

        // allocate out tiles if required
        a->allocateRect(urect);
        b->allocateRect(urect);

        // rect in layer coords (offset from tileExtents.topLeft())
        QRect rect = urect;
        rect.moveTopLeft(urect.topLeft() - a->tileExtents().topLeft());

        // workout which tiles in the layer need to be updated
        int minYTile=rect.top() / TILE_SIZE;
        int maxYTile=rect.bottom() / TILE_SIZE;
        int minXTile=rect.left() / TILE_SIZE;
        int maxXTile=rect.right() / TILE_SIZE;

        QRect tileBoundary;

        for(int y=minYTile; y<=maxYTile; y++)
	    {
	        for(int x=minXTile; x<=maxXTile; x++)
	        {
	            int dstTile = y * a->xTiles() + x;
	            tileBoundary = a->tileRect(dstTile);
	            renderLayerIntoTile(tileBoundary, b, a, dstTile);
	        }
	    }

        list.remove(b);
        m_layers.remove(b);

        if( m_activeLayer == b )
	    {
	        if(m_layers.count() != 0)
	            m_activeLayer = m_layers.at(0);
	        else
	            m_activeLayer = NULL;
	    }

        delete b;
    }

    emit layersUpdated();
    compositeImage(newRect);
}

void KisImage::resizePixmap(KisLayer *lay, const QRect& rect)
{
	int tmpXTiles = lay -> xTiles();
	int tmpYTiles = lay -> yTiles();

	//m_timer -> stop();

	if (tmpXTiles > m_xTiles || tmpYTiles > m_yTiles) {
		int old_m_xTiles = m_xTiles;
		int old_m_yTiles = m_yTiles;

		if (tmpXTiles > m_xTiles)
			m_xTiles = tmpXTiles;

		if (tmpYTiles > m_yTiles)
			m_yTiles = tmpYTiles;

		if (rect.width() > m_width)
			m_width = rect.width();

		if (rect.height() > m_height)
			m_height = rect.height();

		// setup dirty flag array
		m_dirty.resize(m_xTiles * m_yTiles);

		// mark everything dirty so all layers are redrawn
		// after the timer is restarted
		for(int y = 0; y < m_yTiles; y++)
			for(int x = 0; x < m_xTiles; x++)
				m_dirty[y * m_xTiles + x] = true;

		// delete pixmaps for each tile
		for (int y = 0; y < old_m_yTiles; y++) {
			for (int x = 0; x < old_m_xTiles; x++)
				delete m_pixmapTiles[y * old_m_xTiles + x];

			// delete pointer to array of pixmaps
			delete[] m_pixmapTiles;

			// reallocate pointer to array of pixmaps
			m_pixmapTiles = new QPixmap*[m_xTiles * m_yTiles];

			// reallocate pixmaps for each tile
			for (int y = 0; y < m_yTiles; y++) {
				for (int x = 0; x < m_xTiles; x++) {
					m_pixmapTiles[y * m_xTiles + x] = new QPixmap(TILE_SIZE, TILE_SIZE);
					m_pixmapTiles[y * m_xTiles + x] -> fill();
				}
			}

			compositeImage();
		}
	}

	//m_timer -> start(1);
}

void KisImage::setAutoUpdate(bool autoUpdate)
{
	m_autoUpdate = autoUpdate;

	if (autoUpdate) {
		// TODO Go over the dirty tiles list, and update them
		// TODO QPtrList<QPixmap> m_dirtyTiles;
		compositeImage(QRect(0, 0, m_width, m_height));
	}
}

QRect KisImage::findBoundingTiles(const QRect& area)
{
	QRect rc;
	int maxY;
	int maxX;
	int minX;
	int minY;

	if (!area.isNull()) {
		QRect rc = KisUtil::findBoundingTiles(area);

		maxY = rc.bottom() < m_yTiles ? rc.bottom() + 1 : m_yTiles;
		maxX = rc.right() < m_xTiles ? rc.right() + 1 : m_xTiles;
		minY = rc.top();
		minX = rc.left();
	}
	else {
		maxY = m_yTiles;
		maxX = m_xTiles;
		minY = 0;
		minX = 0;
	}

	rc.setRight(maxX);
	rc.setBottom(maxY);
	rc.setLeft(minX);
	rc.setTop(minY);
	return rc;
}

KisPaintDevice* KisImage::getCurrentPaintDevice()
{
	KisPaintDevice *device = 0;

	if ((device = getCurrentChannel()))
		return device;

	return getCurrentLayer();
}

KisChannel* KisImage::getCurrentChannel()
{
	return m_activeChannel;
}

KisLayer* KisImage::getCurrentLayer() 
{ 
	return m_activeLayer; 
}

void KisImage::renderTile(KisTile *dst, KisTile *src, const KisPaintDevice *srcDevice)
{
	uchar src_opacity = srcDevice -> opacity();
	uchar opacity;
	uchar inverseOpacity;
	QRgb *srgb;
	QRgb *drgb;
	uchar sr, sg, sb, sa;
	uchar dr, dg, db, da;
	
	for (int y = 0; y < TILE_SIZE; y++) {
		drgb = dst -> data() + (y * TILE_SIZE);
		srgb = src -> data() + (y * TILE_SIZE);

		for (int x = 0; x < TILE_SIZE; x++) {
			sr = qRed(*srgb);
			sg = qGreen(*srgb);
			sb = qBlue(*srgb);
			sa = qAlpha(*srgb);
			dr = qRed(*drgb);
			dg = qGreen(*drgb);
			db = qBlue(*drgb);
			da = qAlpha(*drgb);
			
			if (m_cMode == cm_RGBA) {
				opacity = sa && da ? (da * src_opacity) / 255 : 0;
				inverseOpacity = 255 - opacity;
				dr = ((dr * da / 255) * inverseOpacity + sr * opacity) / 255;
				dg = ((dg * da / 255) * inverseOpacity + sg * opacity) / 255;
				db = ((db * da / 255) * inverseOpacity + sb * opacity) / 255;
				da = sa + da - (sa * da) / 255;
				*drgb = qRgba(dr, dg, db, da);
			}
			else {
				inverseOpacity = 255 - src_opacity;
				dr = (dr * inverseOpacity + sr * src_opacity) / 255;
				dg = (dg * inverseOpacity + sg * src_opacity) / 255;
				db = (db * inverseOpacity + sb * src_opacity) / 255;
				*drgb = qRgb(dr, dg, db);
			}

			drgb++;
			srgb++;
		}
	}
}

#include "kis_image.moc"

