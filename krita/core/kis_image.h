/*
 *  kis_image.h - part of KImageShop
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

#ifndef __kis_image_h__
#define __kis_image_h__

#include <qimage.h>
#include <qmemarray.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qvaluevector.h>

#include <ksharedptr.h>

#include <koColor.h>

#include "kis_channel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"

#include "kis_tiles.h"
#include "kis_tile.h"

class QTimer;
class DCOPObject;
class KCommand;

class KisBrush;
class KisDoc;
class KisImage;

typedef KSharedPtr<KisImage> KisImageSP;
typedef QValueVector<KisImageSP> KisImageSPLst;
typedef KisImageSPLst::iterator KisImageSPLstIterator;
typedef KisImageSPLst::const_iterator KisImageSPLstConstIterator;

class KisImage : public QObject, public KShared {
	Q_OBJECT
	typedef QObject super;

public:
	KisImage(KisDoc *doc, const QString& name, int width, int height, cMode cm = cm_RGBA, uchar bitDepth = 8);
	virtual ~KisImage();

	virtual DCOPObject* dcopObject();

	KisPaintDeviceSP getCurrentPaintDevice();
	KisChannelSP getCurrentChannel();
	KisLayerSP getCurrentLayer();

	void upperLayer(unsigned int layer);
	void lowerLayer(unsigned int layer);
	void setFrontLayer(unsigned int layer);
	void setBackgroundLayer(unsigned int layer);

	void addLayer(const QRect& rc, const KoColor& c, bool transparent, const QString& name);
	void addLayer(KisLayerSP layer);
	void removeLayer(KisLayerSPLstIterator it);
	void removeLayer(unsigned int layer);
	void removeLayer(KisLayerSP layer);

	/**
	 * @name nLayers
	 * @return Returns the number of layers in this image.
	 */
	int nLayers() const;

	void mergeAllLayers();
	void mergeVisibleLayers();
	void mergeLinkedLayers();

	int getCurrentLayerIndex() const;
	void setCurrentLayer(int layer);
	void setCurrentLayer(KisLayerSP layer);

	inline KisLayerSPLst layerList();
	inline KisChannelSPLst channelList();

	void addChannel(const QRect& rc, uchar opacity, const QString& name);
	void addChannel(KisChannelSP channel);
	void removeChannel(KisChannelSPLstIterator it);
	void removeChannel(unsigned int channel);
	void removeChannel(KisChannelSP channel);

	int getCurrentChannelIndex() const;
	void setCurrentChannel(int channel);
	void setCurrentChannel(KisChannelSP channel);

	void markDirty(const QRect& rect);

	void setAutoUpdate(bool autoUpdate = true);
	inline void setUndo(bool doUndo = true);

	void paintContent(QPainter& painter, const QRect& rect, bool transparent = false);
	void paintPixmap(QPainter *painter, const QRect& area);

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

	int getHishestLayerEver() const;

signals:
	void updated();
	void updated(const QRect& rect);
	void layersUpdated();

#if 1
public slots:
	void slotUpdateTimeOut();
#endif

protected:
	void mergeLayers(KisLayerSPLst& layers);
	void compositeImage(const QRect& rect = QRect(), bool allDirty = false);
	void compositeTile(KisPaintDevice *dstDevice, int tileNo, int x, int y);
	void convertTileToPixmap(KisPaintDevice *dstDevice, int tileNo, QPixmap *pix);
	void convertImageToPixmap(QImage *img, QPixmap *pix);

private:
	KisImage(const KisImage&);
	KisImage& operator=(const KisImage&);

	void addCommand(KCommand *cmd);
	void renderTile(KisPixelPacket *dst, const KisPixelPacket *src, const KisPaintDevice *srcDevice);
	void renderBg(KisPaintDevice *srcDevice, int tileNo);
	void resizeImage(KisPaintDevice *device, const QRect& rect);
	void resizePixmap(bool dirty);
	void destroyPixmap();
	QRect findBoundingTiles(const QRect& area);

private:
	int m_depth;
	KisDoc *m_doc;

	KisLayerSPLst m_layers;
	KisChannelSPLst m_channels;

	int m_xTiles;
	int m_yTiles;
	QPixmap **m_pixmapTiles;
	QImage m_imgTile;

	KisLayerSP m_activeLayer;
       	KisLayerSP m_composeLayer;
       	KisLayerSP m_bgLayer;

	KisChannelSP m_activeChannel;

	QString m_name;
	QString m_author;
	QString m_email;

	int m_width;
	int m_height;

	cMode m_cMode;
	uchar m_bitDepth;
	QMemArray<bool> m_dirty;

	QPtrList<QPixmap> m_dirtyTiles;
        DCOPObject* m_dcop;
	bool m_autoUpdate;
	bool m_doUndo;
	int m_nLayers;
	QTimer *m_timer;
};

KisLayerSPLst KisImage::layerList()
{
	return m_layers;
}

KisChannelSPLst KisImage::channelList()
{
	return m_channels;
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

void KisImage::setUndo(bool doUndo)
{
	m_doUndo = doUndo;
}

#endif

