/*
 *  kis_layer.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999-2000 Matthias Elter <elter@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#include <kdebug.h>

#include "kis_layer.h"
#include "kis_global.h"

KisLayer::KisLayer(const QString& name, uint width, uint height, uint bpp, cMode cm, const QRgb& defaultColor) : super(name, width, height, bpp, defaultColor)
{
	m_cMode = cm;
	m_visible = true;
	m_linked = false;
	m_current = 0;
	m_opacity= 255;
	mLayerXOffset = 0;
	mLayerYOffset = 0;
}

KisLayer::~KisLayer()
{
	kdDebug() << "KisLayer::~KisLayer\n";
}

// XXX Move the firstChannel and nextChannel to KisImage
#if 0
KisChannel* KisLayer::firstChannel()
{
    m_current = 0;
    return m_ch[0];
}


KisChannel* KisLayer::nextChannel()
{
    m_current++;
    
    if (m_current < m_channels)
	    return m_ch[m_current];
    else
	    return 0;
}

void KisLayer::calcNumChannels()
{
    switch (m_cMode)
    {
	    case cm_Indexed:
	    case cm_Greyscale:
	        m_channels = 1;
	        return;

	    case cm_RGB:
	    case cm_Lab:
	        m_channels = 3;
	        return;

	    case cm_RGBA:
	    case cm_CMYK:
	    case cm_LabA:
	        m_channels = 4;
	        return;

	    case cm_CMYKA:
	        m_channels = 5;
	        return;

	    default:
	        m_channels = 0;
	        return;
    }
}

#endif

QRect KisLayer::tileRect(int tileNo)
{
	int xTile = tileNo % m_tiles.xTiles();
	int yTile = tileNo / m_tiles.xTiles(); // xTiles is used here.  Is this the intent?

	QRect tr(xTile * TILE_SIZE, yTile * TILE_SIZE, TILE_SIZE, TILE_SIZE);

	tr.moveBy(m_tileRect.x(), m_tileRect.y());
	return(tr);
}

#if 0
uint* KisLayer::scanLine()
{
	KisTile *tile = m_tiles.getTile(0, 0);

	Q_ASSERT(tile);
	return tile -> data();
}
#endif

#if 0
uchar* KisLayer::channelMem(uchar channel, uint tileNo, int ox, int oy) const
{
	assert(false);
	return 0;
	//return m_ch[channel]->tiles()[tileNo] + (oy * TILE_SIZE + ox);
}

#endif

QRect KisLayer::imageExtents() const
{
	return m_imgRect;
//    return m_ch[0]->imageExtents();
}


QRect KisLayer::layerExtents() const
{
	return QRect(mLayerXOffset, mLayerYOffset, mLayerWidth, mLayerHeight);
}


#if 0
QPoint KisLayer::channelOffset() const 
{

//    return m_ch[0]->offset();
}
#endif


void KisLayer::moveBy(int dx, int dy)
{
	mLayerXOffset += dx;
	mLayerYOffset += dy;
	m_imgRect.moveBy(dx, dy);
	m_tileRect.moveBy(dx, dy);

#if 0
    mLayerXOffset += dx;
    mLayerYOffset += dy;
    
    kdDebug() << " mLayerXOffset " << mLayerXOffset  
              << " mLayerYOffset " << mLayerYOffset 
              << " mLayerWidth "   << mLayerWidth 
              << " mLayerHeight "  << mLayerHeight 
              << endl;

    for (uchar i = 0; i < m_channels; i++)
	    m_ch[i]->moveBy(dx, dy);
#endif
}


void KisLayer::moveTo(int x, int y) 
{
	mLayerXOffset = x;
	mLayerYOffset = y;

	int dx = x - m_imgRect.x();
	int dy = y - m_imgRect.y();

	m_imgRect.moveTopLeft(QPoint(x, y));
	m_tileRect.moveBy(dx,dy);

#if 0
	Q_ASSERT(false);

    mLayerXOffset = x;
    mLayerYOffset = y;
    
    kdDebug() << " mLayerXOffset " << mLayerXOffset  
              << " mLayerYOffset " << mLayerYOffset 
              << " mLayerWidth "   << mLayerWidth 
              << " mLayerHeight "  << mLayerHeight 
              << endl;

    for (uchar i = 0; i < m_channels; i++)
	    m_ch[i]->moveTo(x, y);
#endif
}


int KisLayer::channelLastTileOffsetX() const
{
	Q_ASSERT(false);
	return 0;
//	return m_ch[0]->lastTileOffsetX();
}


int KisLayer::channelLastTileOffsetY() const
{
	Q_ASSERT(false);
	return 0;
//	return m_ch[0]->lastTileOffsetY();
}


bool KisLayer::boundryTileX(int tile) const
{
	return (((tile % xTiles()) + 1) == xTiles());
}


bool KisLayer::boundryTileY(int tile) const
{
	return (((tile/xTiles()) + 1) == yTiles());
}


void KisLayer::allocateRect(const QRect& r)
{
	mLayerWidth  = r.width();  //jwc
	mLayerHeight = r.height(); //jwc
        
	m_imgRect = m_tileRect = r;
#if 0
    for (uchar i = 0; i < m_channels; i++)
	    m_ch[i]->allocateRect(r);
#endif
}

void KisLayer::clear(const KoColor& c, bool transparent )
{
#if 0
    if (!m_cMode == cm_RGB && !m_cMode == cm_RGBA)
	    return;

    uchar r = static_cast<uchar>(c.R());
    uchar g = static_cast<uchar>(c.G());
    uchar b = static_cast<uchar>(c.B());

    bool alpha = (m_cMode == cm_RGBA);

    kdDebug(0) << "yTiles(): " << yTiles() << "xTiles(): " << xTiles() << endl;

    for(int y = 0; y < yTiles(); y++)
    {
        for(int x = 0; x < xTiles(); x++)
        {
	        // set the alpha channel
	        if (alpha)
	        {
		        if (transparent)
                {
	                memset(channelMem(3, y * xTiles() + x, 0, 0), 
                        0 , TILE_SIZE*TILE_SIZE);
                }
		        else
                {
		            memset(channelMem(3, y * xTiles() + x, 0, 0), 
                        255 , TILE_SIZE*TILE_SIZE);
                }
	        }

	        uchar* ptrRed   = channelMem(0, y * xTiles() + x, 0, 0);
	        uchar* ptrGreen = channelMem(1, y * xTiles() + x, 0, 0);
	        uchar* ptrBlue  = channelMem(2, y * xTiles() + x, 0, 0);
	
	        // set data channels to color
	        for(int y = 0; y < TILE_SIZE; y++)
            {
		        for(int x = 0; x < TILE_SIZE; x++)
		        {
		            *(ptrRed    + (y * TILE_SIZE + x)) = r;
		            *(ptrGreen  + (y * TILE_SIZE + x)) = g;
		            *(ptrBlue   + (y * TILE_SIZE + x)) = b;
		        }
            }
        }
    }    
#endif
}

#include "kis_layer.moc"
