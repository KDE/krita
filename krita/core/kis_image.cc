/*
 *  kis_image.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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
#include <qregexp.h>
#include <qfileinfo.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <kdebug.h>

#include "kis_factory.h"
#include "kis_layer.h"
#include "kis_util.h"
#include "kis_brush.h"
#include "kis_global.h"
#include "kis_timer.h"
#include "kis_image.h"

#define KIS_DEBUG(AREA, CMD)
//#define TEST_UNOPTIMIZED_VISUAL

KisImage::KisImage( const QString& n, int w, int h, cMode cm, uchar bd )
  : m_name (n)
  , m_width (w)
  , m_height (h)
  , m_cMode (cm)
  , m_bitDepth (bd)
{
    m_author = "The Krayon team";
    m_email = "kimageshop@mail.kde.org";

    // jwc - need to update tile extents when a new layer is
    // added larger than the first (or largest) layer - done!
    QRect tileExtents
        = KisUtil::findTileExtents( QRect(0, 0, m_width, m_height) );

    m_xTiles = tileExtents.width() / TILE_SIZE;
    m_yTiles = tileExtents.height() / TILE_SIZE;

    // setup dirty flag array
    m_dirty.resize (m_xTiles * m_yTiles);

    // no tiles are marked dirty to start
    for(int y = 0; y < m_yTiles; y++)
        for(int x = 0; x < m_xTiles; x++)
	        m_dirty[y * m_xTiles + x] = false;

    setUpVisual();
    QPixmap::setDefaultOptimization( QPixmap::NoOptim );

    m_ptiles = new QPixmap* [m_xTiles*m_yTiles];

    // allocate the pixmaps for the tiles
    for( int y = 0; y < m_yTiles; y++)
        for( int x = 0; x < m_xTiles; x++)
        {
            m_ptiles[y * m_xTiles + x] = new QPixmap(TILE_SIZE, TILE_SIZE);
            m_ptiles[y * m_xTiles + x]->fill();
        }

    m_img.create(TILE_SIZE, TILE_SIZE, 32);

    m_pCurrentLay = 0;

    m_pComposeLay = new KisLayer( "_compose", m_cMode, m_bitDepth );
    m_pComposeLay->allocateRect( QRect( 0, 0, TILE_SIZE, TILE_SIZE ) );

    m_pBGLay = new KisLayer( "_background", m_cMode, m_bitDepth );
    m_pBGLay->allocateRect( QRect( 0, 0, TILE_SIZE, TILE_SIZE ) );

    // FIXME: make it work with non-RGB color spaces
    // make it work with BigEndian! - john

    uchar* ptrRed   = m_pBGLay->channelMem(0, 0, 0, 0); // red
    uchar* ptrGreen = m_pBGLay->channelMem(1, 0, 0, 0); // green
    uchar* ptrBlue  = m_pBGLay->channelMem(2, 0, 0, 0); // blue
    uchar* ptrAlpha = 0;                                // alpha

    if (m_cMode == cm_RGBA)
	    ptrAlpha = m_pBGLay->channelMem(3, 0, 0, 0);

    /* fill in background layer - draw a checkerboard pattern of alternating
    dark and light gray tiles in 16x16 squares on the background layer -
    if the image background is white, this is not seen until its first
    layer is moved. Note:  This "background" layer is deceptive.  It is
    not the same thing as the background layer that shows up in the layerview
    which can either be transparent like this underlying one or white
    or some other solid color. */

    for( int y = 0; y < TILE_SIZE; y++)
        for(int x = 0; x < TILE_SIZE; x++)
	    {
	        uchar v = 128+63*((x/16 + y/16)%2);
	        *(ptrRed + (y * TILE_SIZE + x)) = v;
	        *(ptrGreen + (y * TILE_SIZE + x)) = v;
	        *(ptrBlue + (y * TILE_SIZE + x)) = v;
	        if (m_cMode == cm_RGBA)
		        *(ptrAlpha + (y * TILE_SIZE + x)) = 255;
	    }

    compositeImage(QRect());

    /* note - 32 bit pixmaps may not show up on a 16 bit display
    and will even crash it !!!  Without depth paramater Qt will
    use native format. imagePixmap = new QPixmap(w, h, 32); does
    not work!, so alway default to native display depth when
    creating a QPixmap.  With a QImage you can create a 32 bit
    depth with a 16 bit display with no problems, however. */

    // start update timer
    m_pUpdateTimer = new QTimer(this);
    connect( m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateTimeOut()) );
    m_pUpdateTimer->start(1);
}


/*
    KisImage destructor - note that various arrays must be
    deleted as these are not children of the QObject for
    KisImage, but are regular arrays and pointers to them.
*/
KisImage::~KisImage()
{
    kdDebug()<<"~KisImage()\n";

    for( int y = 0; y < m_yTiles; y++)
        for( int x = 0; x < m_xTiles; x++)
	        delete m_ptiles[y * m_xTiles + x];

    delete m_ptiles;

    if ((visual != unknown) && (visual != rgb888x))
        free(m_pImgData);
}

/*
    markDirty - this is a public method for drawing and painting
    tools, etc., to use to indicate rectantular areas that have
    been written to, usually using layer->setPixel(), so that they
    can then be rendered to the display when the update timer sends
    a timeout signal. See below..
*/
void KisImage::markDirty( QRect r )
{
    for(int y = 0; y < m_yTiles; y++)
        for(int x = 0; x < m_xTiles; x++)
            if (r.intersects(QRect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE)))
		        m_dirty[y * m_xTiles + x] = true;
}


/*
    slotUpdateTimeOut - invoked by timeout signal from the
    timer for this image.  Those tiles which are "dirty"
    are then rendered into a QImage and converted to a QPixmpa,
    one at a time, for display to screen paint device, which
    is normall the kisCanvas() for the view.
*/

#define TEST_PIXMAP_IMAGE

void KisImage::slotUpdateTimeOut()
{
    // if tile is dirty,
    for(int y = 0; y < m_yTiles; y++)
	  for(int x = 0; x < m_xTiles; x++)
	    if (m_dirty[y * m_xTiles + x])
	        compositeImage(QRect(x*TILE_SIZE, y*TILE_SIZE,TILE_SIZE,TILE_SIZE));

    // tiles updated, mark all tiles clean
    for(int y = 0; y < m_yTiles; y++)
      for(int x = 0; x < m_xTiles; x++)
	    if (m_dirty[y * m_xTiles + x])
	    {
	        m_dirty[y * m_xTiles + x] = false;

   /* redundant - will be removed after further testing. "updated" signal
   has already been emitted for each dirty tile in compositImage() for each
   tile which is dirty.  No need to do it again when tiles are marked clean.
   Eliminating this redundancy results in a 2x performance improvement
   for the krayon rendering engine!!! */

#ifndef TEST_PIXMAP_IMAGE
		    emit updated(QRect(x*TILE_SIZE, y*TILE_SIZE,TILE_SIZE,TILE_SIZE));
#endif
	    }
}


void KisImage::setUpVisual()
{
    QPixmap p;
    Display *dpy    =   p.x11Display();
    int displayDepth=   p.x11Depth();
    Visual *vis     =   (Visual*)p.x11Visual();
    bool trueColour =   (vis->c_class == TrueColor);

    QImage::Endian sysBitOrder  =   QImage::systemBitOrder();
    QImage::Endian sysByteOrder =   QImage::systemByteOrder();

    if(sysBitOrder == QImage::LittleEndian)
    {
        kdDebug()<<"setUpVisual() sysBitOrder is LittleEndian"<<endl;
        mBigEndianBitOrder = false;
    }
    else if(sysBitOrder == QImage::BigEndian)
    {
        kdDebug()<<"setUpVisual() sysBitOrder is BigEndian"<<endl;
        mBigEndianBitOrder = true;
    }
    if(sysByteOrder == QImage::LittleEndian)
    {
        kdDebug()<<"setUpVisual() sysByteOrder is LittleEndian"<<endl;
        mBigEndianByteOrder = false;
    }
    else if(sysByteOrder == QImage::BigEndian)
    {
        kdDebug()<<"setUpVisual() sysByteOrder is BigEndian"<<endl;
        mBigEndianByteOrder = true;
    }

    //mBigEndian = (mBigEndianBitOrder || mBigEndianByteOrder);
    mBigEndian = (mBigEndianBitOrder | mBigEndianByteOrder);

    bool first = true;
    bool second = true;
    bool third = (first || second);

    kdDebug() << "setUpVisual(): third " << third << endl;

    if(trueColour)
        kdDebug()<<"setUpVisual() trueColour is true"<<endl;
    else
        kdDebug()<<"setUpVisual() trueColour is false"<<endl;

    // change the false to true to test the faster image converters
    visual = unknown;

    // do they have a worthy display - doesn't work with big endian
    if (/* true && trueColour && !mBigEndian */ false)
    {
        uint red_mask  =(uint)vis->red_mask;
        uint green_mask=(uint)vis->green_mask;
        uint blue_mask =(uint)vis->blue_mask;

        if ((red_mask==0xf800) && (green_mask==0x7e0) && (blue_mask==0x1f))
        {
            kdDebug() << "visual is rgb565" << endl;
            visual = rgb565;
        }
        if ((red_mask==0xff0000) && (green_mask==0xff00) && (blue_mask==0xff))
        {
            kdDebug() << "visual is rgb565" << endl;
            visual = rgb888x;
        }

#ifdef TEST_UNOPTIMIZED_VISUAL

        visual = unknown;
        kdDebug() << "Using unoptimized visual" << endl;

#else

        if (visual==unknown)
        {
            puts("Unoptimized visual - want to write an optimised routine?");
            printf("red=%8x green=%8x blue=%8x\n",red_mask,green_mask,blue_mask);
        }
        else
        {
            puts("Using optimized visual");
            m_pxi=XCreateImage( dpy, vis, displayDepth, ZPixmap, 0,0, TILE_SIZE,TILE_SIZE, 32, 0 );
            printf("ximage: bytes_per_line=%d\n",m_pxi->bytes_per_line);

            if (visual!=rgb888x)
            {
		        m_pImgData=new char[m_pxi->bytes_per_line*TILE_SIZE];
		        m_pxi->data=m_pImgData;
            }
        }
#endif

    }
}



void KisImage::paintContent( QPainter& painter,
    const QRect& rect, bool /*transparent*/)
{
    paintPixmap( &painter, rect);
}

/*
    Paint the pixmap of each tile which falls within the given
    rectangle onto the QPainter passed in - normally this is the
    kisCanvas - the main widget in the client area of the view
    but it can be another painter as well, such as another pixmap
    for the entire image or the area to be updated.
*/
void KisImage::paintPixmap(QPainter *p, QRect area)
{
    if (!p) return;

    // KisTimer::start();
    int startX, startY, pixX, pixY, clipX, clipY = 0;

    int l = area.left();
    int t = area.top();
    int r = area.right();
    int b = area.bottom();

    //kdDebug()<<"KisImage::paintPixmap l: "<<l <<"; t: "<<t<<"; r: "<< r<<"; b: "<< b<<endl;

    for(int y=0; y < m_yTiles; y++)
    {
        for(int x=0; x < m_xTiles; x++)
	    {
            QRect tileRect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE);

	        int xt = x*TILE_SIZE;
	        int yt = y*TILE_SIZE;

	        //kdDebug()<<"tile: "<< (y*m_xTiles+x)<<endl;

	        if (tileRect.intersects(QRect(0, 0, m_width, m_height))
            && tileRect.intersects(area))
	        {
	            if (m_ptiles[y*m_xTiles+x]->isNull())
		            continue;

	            if (xt < l)
		        {
		            startX = 0;
		            pixX = l - xt;
		        }
	            else
		        {
		            startX = xt - l;
		            pixX = 0;
		        }

	            clipX = r - xt - pixX;

	            if (yt < t)
		        {
		            startY = 0;
		            pixY = t - yt;
		        }
	            else
		        {
		            startY = yt - t;
		            pixY = 0;
		        }

	            clipY = b - yt - pixY;

	            if (clipX <= 0)
		            clipX = -1;
	            if (clipY <= 0)
		            clipY = -1;

	            //kdDebug()<<"clipX "<<clipX<<" clipY "<< clipY<<endl;
	            //kdDebug()<<"pixX :"<<pixX <<" pixY "<< pixY<<endl;

	            p->drawPixmap(startX, startY, *m_ptiles[y*m_xTiles+x],
                    pixX, pixY, clipX, clipY);
	        }
	    }
    }
    // KisTimer::stop("paintImage ");
}



void KisImage::addLayer(const QRect& rect, const KisColor& c,
    bool tr, const QString& name)
{
    KisLayer *lay = new KisLayer(name, m_cMode, m_bitDepth);
    // kdDebug(0) << "KisImage::addLayer(): new allocated" << endl;

    lay->allocateRect(rect);
    // kdDebug(0) << "KisImage::addLayer(): returned from lay->allocateRect()" << endl;

    lay->clear(c, tr);
    // kdDebug(0) << "KisImage::addLayer(): returned from lay->clear()" << endl;

    m_layers.append(lay);
    // kdDebug(0) << "KisImage::addLayer(): returned from m_layers->append()" << endl;

    m_pCurrentLay=lay;

    /*##########################################################
        We need to get updated info about tiles when adding a
        new layer that is larger than existing imageExtents()
    ############################################################*/

    int tmpXTiles = lay->xTiles();
    int tmpYTiles = lay->yTiles();

    if(tmpXTiles > m_xTiles || tmpYTiles > m_yTiles)
    {
        m_pUpdateTimer->stop();

        int old_m_xTiles = m_xTiles;
        int old_m_yTiles = m_yTiles;

        if(tmpXTiles > m_xTiles) m_xTiles = tmpXTiles;
        if(tmpYTiles > m_yTiles) m_yTiles = tmpYTiles;

        if(rect.width()  > m_width)  m_width  = rect.width();
        if(rect.height() > m_height) m_height = rect.height();

        // setup dirty flag array
        m_dirty.resize (m_xTiles * m_yTiles);

        // mark everything dirty so all layers are redrawn
        // after the timer is restarted
        for(int y = 0; y < m_yTiles; y++)
            for(int x = 0; x < m_xTiles; x++)
	            m_dirty[y * m_xTiles + x] = true;

        // delete pixmaps for each tile
        for( int y = 0; y < old_m_yTiles; y++)
        {
            for( int x = 0; x < old_m_xTiles; x++)
            {
                delete m_ptiles[y * old_m_xTiles + x];
            }
        }

        // delete pointer to array of pixmaps
        delete m_ptiles;

        // reallocate pointer to array of pixmaps
        m_ptiles = new QPixmap* [m_xTiles*m_yTiles];

        // reallocate pixmaps for each tile
        for( int y = 0; y < m_yTiles; y++)
        {
            for( int x = 0; x < m_xTiles; x++)
            {
                m_ptiles[y * m_xTiles + x] = new QPixmap(TILE_SIZE, TILE_SIZE);
                m_ptiles[y * m_xTiles + x]->fill();
            }
        }

        compositeImage(QRect());

        // restart update timer
        m_pUpdateTimer->start(1);
    }
}


void KisImage::removeLayer( unsigned int _layer )
{
    if( _layer >= m_layers.count() )
        return;

    KisLayer* lay = m_layers.take( _layer );

    if( m_pCurrentLay == lay )
    {
        if( m_layers.count() != 0 )
            m_pCurrentLay = m_layers.at( 0 );
        else
            m_pCurrentLay = NULL;
    }

    delete lay;
}


/*
    Constructs the composite image in the tile at x,y
    and updates the relevant pixmap
*/

void KisImage::compositeTile(int x, int y, KisLayer *dstLay, int dstTile)
{
    // work out which tile to render into unless directed to a specific tile
    if (dstTile==-1)
        dstTile=y*m_xTiles+x;
    if (dstLay==0)
        dstLay=m_pComposeLay;

    KIS_DEBUG(tile, printf("\n*** compositeTile %d,%d\n",x,y); );

    //printf("compositeTile: dstLay=%p dstTile=%d\n",dstLay, dstTile);

    // Set the background
    for (uchar i = 0; i < dstLay->numChannels(); i++)
  	    memcpy(dstLay->channelMem(i, dstTile, 0, 0),
            m_pBGLay->channelMem(i, dstTile, 0, 0), TILE_SIZE * TILE_SIZE);

    // Find the tiles boundary in KisImage coordinates
    QRect tileBoundary(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE);

    int l=0;
    KisLayer *lay=m_layers.first();

    while(lay)
    {
        // Go through each layer and find its contribution to this tile
        l++;
        //printf("layer: %s opacity=%d\n",lay->name().data(), lay->opacity());
        if ((lay->visible()) && (tileBoundary.intersects(lay->imageExtents())))
        {
            /* The layer is part of the tile. Find out the 1-4 tiles of
            the channel which are in it and render the appropriate
            proportions of each */
            //TIME_START;
            //printf("*** compositeTile %d,%d\n",x,y);
            renderLayerIntoTile(tileBoundary, lay, dstLay, dstTile);
            //TIME_END("renderLayerIntoTile");
        }

        lay=m_layers.next();
    }
}



void KisImage::compositeImage(QRect r)
{
    //KisTimer::start();

    for(int y = 0; y < m_yTiles; y++)
    {
        for(int x = 0; x < m_xTiles; x++)
        {
            // if rectangle is null do all tiles, covering entire image
            // this is always done at first when creating background layer
            if (r.isNull()
            || r.intersects(QRect(x*TILE_SIZE, y*TILE_SIZE,TILE_SIZE,TILE_SIZE)))
	        {
                // set the alpha channel to opaque in the compose layer
                // if this is not done the colors from the different layers
                // won't show up
		        if (m_cMode == cm_RGBA)
                {
			        memset(m_pComposeLay->channelMem(3, 0, 0, 0),
                        255, TILE_SIZE*TILE_SIZE);
                }
                // for this tile, get all the tiles from different layers
                // which correspond and add them in (if they are visible, etc.)
                // to the compose layer - only this layer is used to draw
                // the image on canvas, after converting it to pixmaps
                // There is one pixmap for each tile
		        compositeTile(x, y, m_pComposeLay, 0);

                // convert only this tile to a pixmap for purposes of
                // drawing the pixmap on the canvas
		        convertTileToPixmap(m_pComposeLay, 0,
                    m_ptiles[y * m_xTiles + x]);
	        }
        }
    }

    //KisTimer::stop("compositeImage");
    emit updated(r);
}


/*
    Renders the part of srcLay which resides in dstTile of dstLay
*/

void KisImage::renderLayerIntoTile(QRect tileBoundary, const KisLayer *srcLay,
          KisLayer *dstLay, int dstTile)
{
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
}




void KisImage::renderTileQuadrant(const KisLayer *srcLay, int srcTile,
	KisLayer *dstLay, int dstTile,
	int srcX, int srcY, int dstX, int dstY, int w, int h)
{
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
}

/*
    layerPtr - internal method for assuring a valid layer
    by defaulting to the current layer if the given layer
    is null.
*/
KisLayer* KisImage::layerPtr( KisLayer *_layer )
{
    if( _layer == 0 )
        return( m_pCurrentLay );
    return( _layer );
}


/*
    setCurrentLayer - sets the current layer to the given
    index.  There also needs to be a method accepting a
    layer ptr instead of an index for setting current layer.
*/
void KisImage::setCurrentLayer( int _layer )
{
    m_pCurrentLay = m_layers.at( _layer );
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
    if (visual==unknown)
    {
        //TIME_START;
        pix->convertFromImage(*image);
        //TIME_END("convertFromImage");
    }
    else
    {
        //TIME_START;
        switch(visual)
        {
            /* 16 bit, it seems - this will not work with
            BigEndian machines like PPC - we need a variation
            on this for BigEndian systems */

            case rgb565:
            {
                ushort s;
                ushort *ptr=(ushort *)m_pImgData;
                uchar *qimg=image->bits();

                for(int y=0;y<TILE_SIZE;y++)
		            for(int x=0;x<TILE_SIZE;x++)
                    {
		                s =(*qimg++)>>3;
			            s|=(*qimg++ & 252)<<3;
			            s|=(*qimg++ & 248)<<8;
			            qimg++;
			            *ptr++=s;
		            }
            }
                break;

            /* true color - endian issues should be irrelevant */

            case rgb888x:
                m_pxi->data=(char*)image->bits();
                break;

            default: break;
        }

        XPutImage(pix->x11Display(), pix->handle(), qt_xget_readonly_gc(qt_xscreen(), false),
	        m_pxi, 0,0, 0,0, TILE_SIZE, TILE_SIZE);

        //TIME_END("fast convertImageToPixmap");
    }
}



void KisImage::convertTileToPixmap(KisLayer *lay, int tileNo, QPixmap *pix)
{
    /* Copy the composite image into a QImage so it can be
    converted to a QPixmap.  Note: surprisingly it is not quicker
    to render directly into a QImage probably due to the CPU cache,
    it's also useless wrt (writing?) to other colour spaces  */

// too confusing - easier to use variables with color names
#if 0
    uchar *ptr0 = lay->channelMem(2, tileNo, 0, 0); // blue
    uchar *ptr1 = lay->channelMem(1, tileNo, 0, 0); // green
    uchar *ptr2 = lay->channelMem(0, tileNo, 0, 0); // red
#endif

    uchar *ptrBlue  = lay->channelMem(2, tileNo, 0, 0); // blue
    uchar *ptrGreen = lay->channelMem(1, tileNo, 0, 0); // green
    uchar *ptrRed   = lay->channelMem(0, tileNo, 0, 0); // red
    uchar *ptrAlpha = 0;

    if (m_cMode == cm_RGBA)
         ptrAlpha = lay->channelMem(3, tileNo, 0, 0); // alpha

    for(int y = 0; y < TILE_SIZE; y++)
    {
// this directly equates what is in each channel to a QImage scanline
// offset backwards to front which hardcodes it to littlendian architecture
#if 0
        uchar *ptr = m_img.scanLine(y);
	    for(int x = TILE_SIZE; x; x--)
	    {
	        *ptr++ = *ptr0++; // blue
	        *ptr++ = *ptr1++; // green
	        *ptr++ = *ptr2++; // red
	        ptr++;  // alpha - ptr incremented, but value not used here
	    }
#endif

// this may be somewhat more inefficient for littlendian systems but
// should also work with bigendian. It lets Qt determine the color values
// for each channel which automatically takes into account endianness for
// 32 bit images like these.  The intermediate variable can be removed
// later with the pointer increment taking place inside the qRgb( )
// phrase, I think, achieving the same performance.

        uchar iblue, ired, igreen, ialpha;

        uint *ptr = (uint *)m_img.scanLine(y);
	    for(int x = TILE_SIZE; x; x--)
	    {
            iblue   = *ptrBlue++;
            igreen  = *ptrGreen++;
            ired    = *ptrRed++;

            if (m_cMode == cm_RGBA)
            {
                ialpha  = *ptrAlpha++;

                // note - to duplicate (sortof) bigendian bug on littleendian
                // system, comment out next line and uncomment out the
                // one after - for testing only - john
                *ptr = qRgba(ired, igreen, iblue, ialpha);
                //*ptr = qRgb(ired, igreen, 255 - ialpha);
            }
            else
            {
                *ptr = qRgb(ired, igreen, iblue);
            }

            ptr++;
	    }
    }

    // Construct the relevant pixmap
    convertImageToPixmap(&m_img, pix);
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

        if( m_pCurrentLay == b )
	    {
	        if(m_layers.count() != 0)
	            m_pCurrentLay = m_layers.at(0);
	        else
	            m_pCurrentLay = NULL;
	    }

        delete b;
    }

    emit layersUpdated();
    compositeImage(newRect);
}


void KisImage::upperLayer( unsigned int _layer )
{
    Q_ASSERT( _layer < m_layers.count() );

    if( _layer > 0 )
    {
        KisLayer *pLayer = m_layers.take( _layer );
        m_layers.insert( _layer - 1, pLayer );
    }
}


void KisImage::lowerLayer( unsigned int _layer )
{
    Q_ASSERT( _layer < m_layers.count() );

    if( _layer < ( m_layers.count() - 1 ) )
    {
        KisLayer *pLayer = m_layers.take( _layer );
        m_layers.insert( _layer + 1, pLayer );
    }
}


void KisImage::setFrontLayer( unsigned int _layer )
{
    Q_ASSERT( _layer < m_layers.count() );

    if( _layer < ( m_layers.count() - 1 ) )
    {
        KisLayer *pLayer = m_layers.take( _layer );
        m_layers.append( pLayer );
    }
}


void KisImage::setBackgroundLayer( unsigned int _layer )
{
    Q_ASSERT( _layer < m_layers.count() );

    if( _layer > 0 )
    {
        KisLayer *pLayer = m_layers.take( _layer );
        m_layers.insert( 0, pLayer );
    }
}

#include "kis_image.moc"
