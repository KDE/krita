/*
 *  kis_layer.h - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
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

#ifndef __kis_layer_h__
#define __kis_layer_h__

#include <qimage.h>
#include <qobject.h>
#include <qptrlist.h>

#include "kis_global.h"
#include "kis_channel.h"
#include "kis_color.h"

class KisLayer : public QObject
{
	Q_OBJECT

 public:
	KisLayer(const QString& name, cMode cm, uchar bitDepth = 8);
	virtual ~KisLayer();

	QString name()    const { return m_name; }
	uchar   opacity() const { return m_opacity; }
	bool    visible() const { return m_visible; }
	bool    linked()  const { return m_linked; }
	uchar   numChannels() const { return m_channels; }
	cMode   colorMode()   const { return m_cMode; }
	uchar   bitDepth()    const { return m_bitDepth; }

	void    setName(const QString& name) { m_name = name; };
	void    setOpacity(uchar o) { m_opacity = o; }
	void    setVisible(bool v) { m_visible = v; }
	void    setLinked(bool l)  { m_linked = l; }

	void    moveBy(int dx, int dy);
	void    moveTo(int x, int y);

	int     xTiles() const;
	int     yTiles() const;
	int     channelLastTileOffsetX() const;
	int     channelLastTileOffsetY() const;
	QRect   tileRect(int tileNo);

    // extents of the image in canvas coords
	QRect   imageExtents() const;
    // extents of the layers tiles in canv coords  
	QRect   tileExtents() const;
    // topLeft of the image in the channel   
	QPoint  channelOffset() const; 

    // information about where the layer rectange (not the
    // entire image rectangle) is in canvas coords -jwc-
    int     width()  { return mLayerWidth; } 
    int     height() { return mLayerHeight; } 
    QPoint  offset() { return QPoint(mLayerXOffset, mLayerYOffset); }
    QRect   layerExtents() const;   
	
	void    loadRGBImage(QImage img, QImage alpha);
	void    loadGrayImage(QImage img, QImage alpha);

	void    findTileNumberAndOffset(QPoint pt, int *tileNo, int *offset) const;
	void    findTileNumberAndPos(QPoint pt, int *tileNo, int *x, int *y) const;

	uchar*  channelMem(uchar channel, uint tileNo, int ox, int oy) const;

	bool    boundryTileX(int tile) const;
	bool    boundryTileY(int tile) const;
	void    allocateRect(QRect _r);
    
	void    setPixel(uchar channel, uint x, uint y, uchar val);
	uchar   pixel(uchar channel, uint x, uint y);

	void    clear(const KisColor& c, bool transparent);

	KisChannel* firstChannel();
	KisChannel* nextChannel();

 signals:
	void layerPropertiesChanged();

 protected:
	void calcNumChannels();

 protected:
	uchar    m_opacity;
	uchar    m_channels;
	uchar    m_current;
	QString  m_name;
	bool     m_visible, m_linked;
	cMode    m_cMode;
	uchar    m_bitDepth;
    
    int      mLayerXOffset; 
    int      mLayerYOffset; 
    int      mLayerWidth;   
    int      mLayerHeight;  	 
	
    KisChannel* m_ch[MAX_CHANNELS];
        
};

#endif // __kis_layer_h__
