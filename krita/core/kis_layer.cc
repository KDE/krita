/*
 *  kis_layer.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999-2000 Matthias Elter <elter@kde.org>
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

#include "kis_layer.h"
#include "kis_global.h"
#include <kdebug.h>


KisLayer::KisLayer(const QString& name, cMode cm, uchar bd)
  : QObject()
  , m_name(name)
  , m_cMode(cm)
  , m_bitDepth(bd)
{
    m_visible = true;
    m_linked = false;
    m_current = 0;
    m_opacity= 255;
    mLayerXOffset = 0;
    mLayerYOffset = 0;
    
    calcNumChannels();

    // FIXME: Implement non-RGB modes.
    if (cm == cm_RGB || cm == cm_RGBA)
    {
        m_ch[0] = new KisChannel(ci_Red, m_bitDepth);
	    m_ch[1] = new KisChannel(ci_Green, m_bitDepth);
	    m_ch[2] = new KisChannel(ci_Blue, m_bitDepth);

	    if (cm == cm_RGBA)
	        m_ch[3] = new KisChannel(ci_Alpha, m_bitDepth);
    }
}


KisLayer::~KisLayer()
{
    for (uchar i = 0; i < m_channels; i++)
	delete m_ch[i];
}



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


QRect KisLayer::tileRect(int tileNo)
{
    return(m_ch[0]->tileRect(tileNo));
}


uchar* KisLayer::channelMem(uchar channel, uint tileNo, int ox, int oy) const
{
    return m_ch[channel]->tiles()[tileNo] + (oy * TILE_SIZE + ox);
}


QRect KisLayer::imageExtents() const
{
    return m_ch[0]->imageExtents();
}


QRect KisLayer::layerExtents() const
{
    return QRect(mLayerXOffset, mLayerYOffset, mLayerWidth, mLayerHeight);
}


QRect KisLayer::tileExtents() const
{
    return m_ch[0]->tileExtents();
}


QPoint KisLayer::channelOffset() const 
{
    return m_ch[0]->offset();
}


int KisLayer::xTiles() const
{
    return m_ch[0]->xTiles();
}


int KisLayer::yTiles() const
{
    return m_ch[0]->yTiles();
}


void KisLayer::moveBy(int dx, int dy)
{
    mLayerXOffset += dx;
    mLayerYOffset += dy;
    
    kdDebug() << " mLayerXOffset " << mLayerXOffset  
              << " mLayerYOffset " << mLayerYOffset 
              << " mLayerWidth "   << mLayerWidth 
              << " mLayerHeight "  << mLayerHeight 
              << endl;

    for (uchar i = 0; i < m_channels; i++)
	    m_ch[i]->moveBy(dx, dy);
}


void KisLayer::moveTo(int x, int y) 
{
    mLayerXOffset = x;
    mLayerYOffset = y;
    
    kdDebug() << " mLayerXOffset " << mLayerXOffset  
              << " mLayerYOffset " << mLayerYOffset 
              << " mLayerWidth "   << mLayerWidth 
              << " mLayerHeight "  << mLayerHeight 
              << endl;

    for (uchar i = 0; i < m_channels; i++)
	    m_ch[i]->moveTo(x, y);
        
}


int KisLayer::channelLastTileOffsetX() const
{
    return m_ch[0]->lastTileOffsetX();
}


int KisLayer::channelLastTileOffsetY() const
{
    return m_ch[0]->lastTileOffsetY();
}


bool KisLayer::boundryTileX(int tile) const
{
    return (((tile % xTiles()) + 1) == xTiles());
}


bool KisLayer::boundryTileY(int tile) const
{
    return (((tile/xTiles()) + 1) == yTiles());
}


void KisLayer::allocateRect(QRect r)
{
    mLayerWidth  = r.width();  //jwc
    mLayerHeight = r.height(); //jwc
        
    for (uchar i = 0; i < m_channels; i++)
	    m_ch[i]->allocateRect(r);

}


void KisLayer::findTileNumberAndOffset(QPoint pt, int *tileNo, int *offset) const
{
    pt = pt - m_ch[0]->tileExtents().topLeft();
    *tileNo = (pt.y() / TILE_SIZE) * xTiles() + pt.x() / TILE_SIZE;
    *offset = (pt.y() % TILE_SIZE) * TILE_SIZE + pt.x() % TILE_SIZE;
}


void KisLayer::findTileNumberAndPos(QPoint pt, int *tileNo, int *x, int *y) const
{
    pt = pt - m_ch[0]->tileExtents().topLeft();
    *tileNo = (pt.y() / TILE_SIZE) * xTiles() + pt.x() / TILE_SIZE;
    *y = pt.y() % TILE_SIZE;
    *x = pt.x() % TILE_SIZE;
}


void KisLayer::setPixel(uchar channel, uint x, uint y, uchar pixel)
{
    m_ch[channel]->setPixel(x,y, pixel);
}


uchar KisLayer::pixel(uchar channel, uint x, uint y)
{
    return m_ch[channel]->pixel(x,y);
}


void KisLayer::clear(const KisColor& c, bool transparent )
{
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
}

#include "kis_layer.moc"
