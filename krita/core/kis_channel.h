/*
 *  kis_channel.h - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999-2000 Matthias ELter  <elter@kde.org>
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

#ifndef __kis_channel_h__
#define __kis_channel_h__

#include <qrect.h>
#include <qpoint.h>
#include <koStore.h> //jwc

#include "kis_global.h"


class KisChannel
{
public:

    KisChannel(cId id, uchar bitDepth = 8);
    virtual ~KisChannel();

    cId    channelId()    const { return m_id; }
    uchar  bitDepth()     const { return m_bitDepth; }
    uint   xTiles()       const { return m_xTiles; }
    // very bad bug - jwc - yTiles() returned m_xTiles, not m_yTiles
    // this casued memory allocation problems with channelMem
    //uint   yTiles()       const { return m_xTiles; }
    uint   yTiles()       const { return m_yTiles; }
    int    width()        const { return m_imgRect.width(); }
    int    height()       const { return m_imgRect.height(); }
    QRect  tileExtents()  const { return m_tileRect; };
    QRect  imageExtents() const { return m_imgRect; };
    QPoint offset()       const { return m_imgRect.topLeft() - m_tileRect.topLeft(); };

    uchar** tiles()       { return m_tiles; }

    void allocateRect(QRect newRect);
  
    void moveBy(int dx, int dy);
    void moveTo(int x, int y);
  
    void  setPixel(uint x, uint y, uchar val);
    uchar pixel(uint x, uint y);
	
    QRect tileRect(int tileNo);

    uint lastTileOffsetX();
    uint lastTileOffsetY();

    bool  writeToStore(KoStore *store); 
    bool  loadFromStore(KoStore *store); 
    
 
 protected:
  
    cId      m_id;
    uchar    m_bitDepth;
    
    // array of pointers to tile data
    uchar**  m_tiles;
    uint     m_xTiles, m_yTiles;
  
    QRect    m_imgRect, m_tileRect;

};


#endif // __kis_channel_h__
