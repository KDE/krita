/*
 *  kis_image.h - part of KImageShop
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

#ifndef __kis_image_h__
#define __kis_image_h__

#include <qmemarray.h>
#include <qimage.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qtimer.h>
#include <qpixmap.h>

#include <stdlib.h>

#include "kis_layer.h"
#include "kis_global.h"
#include "kis_color.h"

class KisBrush;

class KisImage : public QObject {
	Q_OBJECT
	typedef QObject super;
public:
	KisImage(const QString& name, int width, int height, cMode cm = cm_RGBA, uchar bitDepth = 8);
	virtual ~KisImage();

	void upperLayer(unsigned int layer);
	void lowerLayer(unsigned int layer);
	void setFrontLayer(unsigned int layer);
	void setBackgroundLayer(unsigned int layer);

	void addLayer(const QRect& r, const KisColor& c, bool transparent, const QString& name);
	void removeLayer(unsigned int layer);

	KisLayer* layerPtr(KisLayer *layer);

	void markDirty(const QRect& rect);

	void mergeAllLayers();
	void mergeVisibleLayers();
	void mergeLinkedLayers();

	void setAutoUpdate(bool autoEnable = true);
	
	void paintContent(QPainter& painter, const QRect& rect, bool transparent = false);
	void paintPixmap(QPainter *painter, const QRect& area);

	void setCurrentLayer(int layer);

	inline QPtrList<KisLayer> layerList();

	inline int height();
	inline int width();
	inline QSize size();
	inline QRect imageExtents();

	inline QString name();
	inline QString author();
	inline QString email();

	inline cMode colorMode();
	inline uchar bitDepth();

	inline void setName(const QString& n);
	inline void setAuthor(const QString& a);
	inline void setEmail(const QString& e);

	inline KisLayer* getCurrentLayer();
	inline int getCurrentLayerIndex();
	
	inline bool bigEndian();
	inline bool bigEndianBitOrder();
	inline bool bigEndianByteOrder();

signals:
	void updated();
	void updated(const QRect& rect);
	void layersUpdated();

protected slots:
	void slotUpdateTimeOut();

protected:
	void mergeLayers(QPtrList<KisLayer> layers);
	void compositeImage(const QRect& rect = QRect());
	void compositeTile( int x, int y, KisLayer *dstLay = 0, int dstTile = -1 );
	void convertTileToPixmap( KisLayer *lay, int tileNo, QPixmap *pix );
	void renderLayerIntoTile( QRect tileBoundary, const KisLayer *srcLay, 
			KisLayer *dstLay, int dstTile );
	void renderTileQuadrant( const KisLayer *srcLay, int srcTile, KisLayer *dstLay,
			int dstTile, int srcX, int srcY, int dstX, int dstY, int w, int h );
	void convertImageToPixmap( QImage *img, QPixmap *pix );

private:
	KisImage(const KisImage&);
	KisImage& operator=(const KisImage&);

	void resizePixmap(KisLayer *lay, const QRect& rect);
	QRect findBoundingTiles(const QRect& area);

private:
	bool mBigEndianBitOrder;
	bool mBigEndianByteOrder;  
	bool mBigEndian;

	int m_xTiles;
	int m_yTiles;

	QImage m_img;
	QPtrList<KisLayer> m_layers;
	KisLayer *m_pCurrentLay;
       	KisLayer *m_pComposeLay;
       	KisLayer *m_pBGLay;

	QPixmap **m_ptiles;
	char *m_pImgData;

	QString m_name;
	QString m_author;
	QString m_email;

	int m_width;
	int m_height;

	cMode m_cMode;
	uchar m_bitDepth;
	QMemArray<bool> m_dirty;

	QPtrList<QPixmap> m_dirtyTiles;
	bool m_autoUpdate;

	QTimer* m_timer;
};

QPtrList<KisLayer> KisImage::layerList() 
{ 
	return m_layers; 
}

int KisImage::height()
{ 
	return m_height; 
}

int KisImage::width()        
{ 
	return m_width; 
}

QSize KisImage::size()         
{ 
	return QSize(m_width, m_height); 
}

QRect KisImage::imageExtents() 
{ 
	return QRect(0, 0, m_width, m_height); 
}

QString KisImage::name()         
{ 
	return m_name; 
}

QString KisImage::author()       
{ 
	return m_author; 
}

QString KisImage::email()        
{ 
	return m_email; 
}

cMode KisImage::colorMode()    
{ 
	return m_cMode; 
} 

uchar KisImage::bitDepth()     
{ 
	return m_bitDepth; 
}

void KisImage::setName(const QString& n)    
{ 
	m_name = n; 
}

void KisImage::setAuthor(const QString& a)  
{ 
	m_author = a; 
}

void KisImage::setEmail(const QString& e)  
{ 
	m_email = e; 
}

KisLayer* KisImage::getCurrentLayer() 
{ 
	return m_pCurrentLay; 
}

int KisImage::getCurrentLayerIndex() 
{ 
	return m_layers.find(m_pCurrentLay); 
}

bool KisImage::bigEndian() 
{ 
	return mBigEndian; 
}

bool KisImage::bigEndianBitOrder() 
{ 
	return mBigEndianBitOrder; 
}

bool KisImage::bigEndianByteOrder() 
{ 
	return mBigEndianByteOrder; 
}

#endif

