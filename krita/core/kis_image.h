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

#include <stdlib.h>

#include "kis_layer.h"
#include "kis_global.h"
#include "kis_color.h"

class KisBrush;
#include <X11/Xlib.h>  // for XImage


class KisImage : public QObject
{
    Q_OBJECT

 public:
    
    KisImage( const QString& name, int width, int height,
    			  cMode cm = cm_RGBA, uchar bitDepth = 8 );
    virtual ~KisImage();

    int     height()       { return m_height; }
    int     width()        { return m_width; }
    QSize   size()         { return QSize( m_width, m_height); }
    QRect   imageExtents() { return QRect(0, 0, m_width, m_height); }

    QString name()         { return m_name; }
    QString author()       { return m_author; }
    QString email()        { return m_email; }

    cMode   colorMode()    { return m_cMode; } 
    uchar   bitDepth()     { return m_bitDepth; }

    void setName(const QString& n)    { m_name = n; }
    void setAuthor(const QString& a)  { m_author = a; }
    void setEmail(const QString& e)  { m_email = e; }
    
    void paintContent( QPainter& painter, const QRect& rect, bool transparent = FALSE );
    void paintPixmap( QPainter *painter, QRect area);

    KisLayer* getCurrentLayer() { return m_pCurrentLay; }
    int getCurrentLayerIndex() { return m_layers.find( m_pCurrentLay ); }
    void setCurrentLayer( int _layer );

    void upperLayer( unsigned int _layer );
    void lowerLayer( unsigned int _layer );
    void setFrontLayer( unsigned int _layer );
    void setBackgroundLayer( unsigned int _layer );

    void addLayer(const QRect& r, const KisColor& c, bool transparent, const QString& name);
    void removeLayer( unsigned int _layer );

    KisLayer* layerPtr( KisLayer *_layer );
    QPtrList<KisLayer> layerList() { return m_layers; };
 
    void markDirty( QRect rect );
     
    void mergeAllLayers();
    void mergeVisibleLayers();
    void mergeLinkedLayers();
    void mergeLayers(QPtrList<KisLayer>);

    bool bigEndian() { return mBigEndian; }
    bool bigEndianBitOrder() { return mBigEndianBitOrder; }        
    bool bigEndianByteOrder() { return mBigEndianByteOrder; }

 signals:
    void updated();
    void updated( const QRect& rect );
    void layersUpdated();

 protected slots:
    void slotUpdateTimeOut();  
    
 protected:
    void compositeImage( QRect _rect );
    void compositeTile( int x, int y, KisLayer *dstLay = 0, int dstTile = -1 );
    void convertTileToPixmap( KisLayer *lay, int tileNo, QPixmap *pix );
    void renderLayerIntoTile( QRect tileBoundary, const KisLayer *srcLay, 
        KisLayer *dstLay, int dstTile );
    void renderTileQuadrant( const KisLayer *srcLay, int srcTile, KisLayer *dstLay,
	    int dstTile, int srcX, int srcY, int dstX, int dstY, int w, int h );
    void setUpVisual();
    void convertImageToPixmap( QImage *img, QPixmap *pix );
        
 private:
    enum dispVisual { unknown, rgb565, rgb888x } visual;

    bool mBigEndianBitOrder;
    bool mBigEndianByteOrder;  
    bool mBigEndian;
    
    int               m_xTiles;
    int               m_yTiles;

    QImage            m_img;
    QPtrList<KisLayer>   m_layers;
    KisLayer         *m_pCurrentLay, *m_pComposeLay, *m_pBGLay;

    QPixmap    **m_ptiles;
    char        *m_pImgData;
    XImage      *m_pxi;
    QString      m_name;
    QString      m_author;
    QString      m_email;
    int          m_width;
    int          m_height;
    cMode        m_cMode;
    uchar        m_bitDepth;
    QMemArray<bool> m_dirty;
    QTimer      *m_pUpdateTimer;
};

#endif
