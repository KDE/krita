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

#include <Magick++.h>

#include <qpainter.h>
#include <qptrqueue.h>
#include <qtimer.h>

#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_color_utils.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_factory.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_magick.h"
#include "kis_pixel_packet.h"
#include "kis_undo.h"
#include "kis_util.h"
#include "kis_timer.h"
#include "KIsImageIface.h"

using namespace Magick;

class KisCommandLayerActive : public KNamedCommand {
	typedef KNamedCommand super;

public:
	KisCommandLayerActive(KisImage *img, KisLayerSP activeLayer, KisLayerSP oldActiveLayer);
	virtual ~KisCommandLayerActive();

	virtual void execute();
	virtual void unexecute();

private:
	KisImage *m_img;
	KisLayerSP m_activeLayer;
	KisLayerSP m_oldActiveLayer;
};

class KisCommandLayer : public KNamedCommand {
	typedef KNamedCommand super;

public:
	KisCommandLayer(KisImage *img, KisLayerSP layer, const QString& name);
	virtual ~KisCommandLayer();

protected:
	void addLayer();
	void removeLayer();

private:
	KisImage *m_img;
	KisLayerSP m_layer;
};

class KisCommandLayerAdd : public KisCommandLayer {
	typedef KisCommandLayer super;

public:
	KisCommandLayerAdd(KisImage *img, KisLayerSP layer);
	virtual ~KisCommandLayerAdd();

	virtual void execute();
	virtual void unexecute();
};

class KisCommandLayerRm : public KisCommandLayer {
	typedef KisCommandLayer super;

public:
	KisCommandLayerRm(KisImage *img, KisLayerSP layer);
	virtual ~KisCommandLayerRm();

	virtual void execute();
	virtual void unexecute();
};

KisCommandLayerActive::KisCommandLayerActive(KisImage *img, KisLayerSP activeLayer, KisLayerSP oldActiveLayer) : super(i18n("Set Active Layer"))
{
	m_img = img;
	m_activeLayer = activeLayer;
	m_oldActiveLayer = oldActiveLayer;
}

KisCommandLayerActive::~KisCommandLayerActive()
{
}

void KisCommandLayerActive::execute()
{
	Q_ASSERT(m_img);
	m_img -> setUndo(false);
	m_img -> setCurrentLayer(m_activeLayer);
	m_img -> setUndo(true);
}

void KisCommandLayerActive::unexecute()
{
	Q_ASSERT(m_img);
	m_img -> setUndo(false);
	m_img -> setCurrentLayer(m_oldActiveLayer);
	m_img -> setUndo(true);
}

KisCommandLayer::KisCommandLayer(KisImage *img, KisLayerSP layer, const QString& name) : super(name)
{
	m_img = img;
	m_layer = layer;
}

KisCommandLayer::~KisCommandLayer()
{
}

void KisCommandLayer::addLayer()
{
	Q_ASSERT(m_img);
	Q_ASSERT(m_layer);
	m_img -> setUndo(false);
	m_img -> addLayer(m_layer);
	m_img -> setUndo(true);
}

void KisCommandLayer::removeLayer()
{
	Q_ASSERT(m_img);
	Q_ASSERT(m_layer);
	m_img -> setUndo(false);
	m_img -> removeLayer(m_layer);
	m_img -> setUndo(true);
}

KisCommandLayerAdd::KisCommandLayerAdd(KisImage *img, KisLayerSP layer) : super(img, layer, i18n("Added Layer"))
{
}

KisCommandLayerAdd::~KisCommandLayerAdd()
{
}

void KisCommandLayerAdd::execute()
{
	addLayer();
}

void KisCommandLayerAdd::unexecute()
{
	removeLayer();
}

KisCommandLayerRm::KisCommandLayerRm(KisImage *img, KisLayerSP layer) : super(img, layer, i18n("Removed Layer"))
{
}

KisCommandLayerRm::~KisCommandLayerRm()
{
}

void KisCommandLayerRm::execute()
{
	removeLayer();
}

void KisCommandLayerRm::unexecute()
{
	addLayer();
}

KisImage::KisImage(KisDoc *doc, const QString& name, int w, int h, cMode cm, uchar bd)
	: m_doc(doc),
	m_name(name),
	m_width(w),
	m_height(h),
	m_cMode(cm),
	m_bitDepth(bd)
{
	QRect tileExtents = KisUtil::findTileExtents(QRect(0, 0, m_width, m_height));
	QRgb defaultColor = qRgba(CHANNEL_MIN, CHANNEL_MIN, CHANNEL_MIN, OPACITY_OPAQUE);

	m_nLayers = 0;
	m_xTiles = tileExtents.width() / TILE_SIZE;
	m_yTiles = tileExtents.height() / TILE_SIZE;
	m_activeChannel = 0;
	m_activeLayer = 0;
	m_composeLayer = 0;
	m_bgLayer = 0;
	m_bitDepth = bd;
	m_autoUpdate = false;
	m_doUndo = true;

	m_dcop = 0;
	dcopObject();
	resizePixmap(false);
	m_depth = KisUtil::calcNumChannels(cm);

	m_imgTile.create(TILE_SIZE, TILE_SIZE, m_bitDepth * m_depth, m_depth == 1 ? QImage::LittleEndian : QImage::IgnoreEndian);
	//	m_imgTile.setAlphaBuffer(true);

	m_composeLayer = new KisLayer("_compose", TILE_SIZE, TILE_SIZE, m_depth, cm, defaultColor);
	m_bgLayer = new KisLayer("_background", TILE_SIZE, TILE_SIZE, m_depth, cm, defaultColor);
	renderBg(m_bgLayer, 0);
	compositeImage();

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotUpdateTimeOut()));
	m_timer -> start(1);
}

/*
    KisImage destructor - note that various arrays must be
    deleted as these are not children of the QObject for
    KisImage, but are regular arrays and pointers to them.
*/

KisImage::~KisImage()
{
	destroyPixmap();
}

inline void KisImage::renderTile(KisPixelPacket *dst, const KisPixelPacket *src, const KisPaintDevice *srcDevice)
{
	if (!src || !srcDevice)
		return;

	memcpy(dst, src, TILE_SIZE * TILE_SIZE * sizeof(KisPixelPacket));
}

DCOPObject* KisImage::dcopObject()
{
	if (!m_dcop)
		m_dcop = new KIsImageIface(this);

	return m_dcop;
}

void KisImage::upperLayer(unsigned int layer)
{
	if (layer > 0) {
		qSwap(m_layers[layer], m_layers[layer - 1]);
		emit layersUpdated();
	}
}

void KisImage::lowerLayer(unsigned int layer)
{
	if (layer + 1 < m_layers.size()) {
		qSwap(m_layers[layer], m_layers[layer + 1]);
		emit layersUpdated();
	}
}

void KisImage::setFrontLayer(unsigned int layer)
{
	if (layer + 1 != m_layers.size()) {
		qSwap(m_layers[layer], m_layers[m_layers.size() - 1]);
		emit layersUpdated();
	}
}

void KisImage::setBackgroundLayer(unsigned int layer)
{
	if (layer) {
		qSwap(m_layers[layer], m_layers[0]);
		emit layersUpdated();
	}
}

void KisImage::addLayer(KisLayerSP layer)
{
	if (qFind(m_layers.begin(), m_layers.end(), layer) == m_layers.end()) {
		m_layers.push_back(layer);
		m_activeLayer = layer;
		compositeImage(QRect(0, 0, m_width, m_height), true);
		emit layersUpdated();
	}
}

void KisImage::addLayer(const QRect& rect, const KoColor& c, bool /*tr*/, const QString& name)
{
	KisLayerSP lay;
	QRgb defaultColor;

	if (getCurrentLayer())
		defaultColor = qRgba(0, 0, 0, 0);
	else
		defaultColor = c.color().rgb();
	
	lay = new KisLayer(name, rect.width(), rect.height(), m_depth, m_cMode, defaultColor);
	m_nLayers++;
	m_layers.push_back(lay);
	m_activeLayer = lay;
	resizeImage(lay, rect);
	emit layersUpdated();

	if (m_doUndo)
		addCommand(new KisCommandLayerAdd(this, lay));
}

void KisImage::removeLayer(KisLayerSPLstIterator it)
{
	KisLayerSP lay = *it;

	if (m_activeLayer == lay) {
		if (m_layers.empty())
			m_activeLayer = 0;
		else
			m_activeLayer = m_layers[0];
	}

	if (m_doUndo)
		addCommand(new KisCommandLayerRm(this, lay));

	m_layers.erase(it);
	emit layersUpdated();
}

void KisImage::removeLayer(unsigned int layer)
{
	if (layer >= m_layers.size()) {
		return;
	}

	removeLayer(m_layers.begin() + layer);
}

void KisImage::removeLayer(KisLayerSP layer)
{
	KisLayerSPLstIterator it;

	if ((it = qFind(m_layers.begin(), m_layers.end(), layer)) != m_layers.end()) {
		removeLayer(it);
		compositeImage(QRect(0, 0, m_width, m_height), true);
		emit layersUpdated();
	}
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
			//dirtyTilesMutex.lock();
			m_dirty[y * m_xTiles + x] = true;

			//dirtyTilesMutex.unlock();

			if (m_autoUpdate)
				compositeImage(QRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE));
		}
	}

	//wcDirty.wakeOne();
}

/*
    slotUpdateTimeOut - invoked by timeout signal from the
    timer for this image.  Those tiles which are "dirty"
    are then rendered into a QImage and converted to a QPixmpa,
    one at a time, for display to screen paint device, which
    is normall the kisCanvas() for the view.
*/

void KisImage::slotUpdateTimeOut()
{
#if 1
	// TODO : Go over tiles in m_dirtyTiles
	// if tile is dirty,
	for(int y = 0; y < m_yTiles; y++)
		for(int x = 0; x < m_xTiles; x++)
			if (m_dirty[y * m_xTiles + x])
				compositeImage(QRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE));

#else
	dirtyTilesMutex.lock();

	while (1) {
		if (dirtyTiles.isEmpty())
			break;

		kdDebug() << "KisImage::slotUpdateTimeOut\n";
		QPoint *pt = dirtyTiles.dequeue();

		compositeImage(QRect(pt -> x() * TILE_SIZE, pt -> y() * TILE_SIZE, TILE_SIZE, TILE_SIZE));
		delete pt;
	}

	dirtyTilesMutex.unlock();
#endif
}

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
		int yt = y * TILE_SIZE;

		for (int x = rc.left(); x < rc.right(); x++) {
			int xt = x * TILE_SIZE;

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

			p -> drawPixmap(xt, yt, *m_pixmapTiles[y * m_xTiles + x]);
		}
	}
}

/*
    Constructs the composite image in the tile at x,y
    and updates the relevant pixmap
*/

void KisImage::compositeTile(KisPaintDevice *dstDevice, int tileNo, int x, int y)
{
	for (KisLayerSPLstIterator it = m_layers.begin(); it != m_layers.end(); it++) {
		KisLayerSP layer = *it;

		if (layer && layer -> visible()) {
			Magick::Image *dst = dstDevice -> getImage();
			Magick::Image *src = layer -> getImage();
			Magick::Image area(*src);

			Q_ASSERT(dst && src);
			area.crop(Geometry(TILE_SIZE, TILE_SIZE, x, y));
			dst -> composite(area, 0, 0, OverCompositeOp);
		}
	}

	// TODO Paint the current channel
}

void KisImage::compositeImage(const QRect& area, bool allDirty)
{
	QRect rc = findBoundingTiles(area);
	
	for (int y = rc.top(); y < rc.bottom(); y++) {
		for (int x = rc.left(); x < rc.right(); x++) {
			if (!allDirty && m_dirty[y * m_xTiles + x] == false)
				continue;

			Magick::Image *dst = m_composeLayer -> getImage();
			Magick::Image *src = m_bgLayer -> getImage();

			dst -> composite(*src, 0, 0, CopyCompositeOp);
			compositeTile(m_composeLayer, 0, x * TILE_SIZE, y * TILE_SIZE);
			convertTileToPixmap(m_composeLayer, 0, m_pixmapTiles[y * m_xTiles + x]);
			m_dirty[y * m_xTiles + x] = false;
		}
	}

	emit updated(area);
}

void KisImage::setCurrentLayer(int layer)
{
	if (static_cast<uint>(layer) < m_layers.size()) {
		KisLayerSP p = m_layers[layer];

		setCurrentLayer(p);
	}
}

void KisImage::setCurrentLayer(KisLayerSP layer)
{
	if (m_doUndo && layer)
		addCommand(new KisCommandLayerActive(this, layer, m_activeLayer));

	if (layer)
		m_activeLayer = layer;
}

void KisImage::convertImageToPixmap(QImage *image, QPixmap *pix)
{
	if (image && pix)
		pix -> convertFromImage(*image);
}

void KisImage::convertTileToPixmap(KisPaintDevice *dstDevice, int tileNo, QPixmap *pix)
{
//	KisTileSP srcTile = dstDevice -> getTile(tileNo * TILE_SIZE, tileNo * TILE_SIZE);
	m_imgTile = convertFromMagickImage(*dstDevice -> getImage());
#if 0
	const KisPixelPacket *src = srcTile -> getConstPixels();

	if (!src)
		return;

	for (int row = 0; row < m_imgTile.height(); row++) {
		for (int column = 0; column < m_imgTile.width(); column++) {
			const KisPixelPacket *pixel = src + row * m_imgTile.width() + column;
			QRgb rgb = *pixel;

			m_imgTile.setPixel(column, row, rgb);
		}
	}
#endif

	convertImageToPixmap(&m_imgTile, pix);
}

/*
    mergeAllLayers - merge all layers into the background or first
    layer and delete them after they are merged, leaving only the
    first layer.
*/
void KisImage::mergeAllLayers()
{
	KisLayerSPLst l;
	
	for (KisLayerSPLstIterator it = m_layers.begin(); it != m_layers.end(); it++)
		l.push_back(*it);

	mergeLayers(l);
}

/*
    mergeVisibleLayers - merge only those layers which are visible.
    Merge everything into the first visible layer encountered.
*/

void KisImage::mergeVisibleLayers()
{
	KisLayerSPLst l;

	for (KisLayerSPLstIterator it = m_layers.begin(); it != m_layers.end(); it++)
		if ((*it) -> visible())
			l.push_back(*it);

	mergeLayers(l);
}

/*
    mergeLinkeLayers - merge only those layers linked.
    Merge everything into the first linked layer encountered.
*/

void KisImage::mergeLinkedLayers()
{
	KisLayerSPLst l;

	for (KisLayerSPLstIterator it = m_layers.begin(); it != m_layers.end(); it++) {
		if ((*it) -> linked())
			l.push_back(*it);
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

void KisImage::mergeLayers(KisLayerSPLst& layers)
{
#if 0
	KisLayerSP a;
	KisLayerSPLstIterator it;
	KMacroCommand *macro = new KMacroCommand(i18n("Merge Layers"));
	QRect rc;

	for (it = layers.begin(); it != layers.end(); it++)
		rc = rc.unite((*it) -> imageExtents());	

	it = layers.begin();
	a = new KisLayer((*it) -> name(), m_width, m_height, m_bpp, m_cMode, qRgba(CHANNEL_MAX, CHANNEL_MAX, CHANNEL_MAX, OPACITY_OPAQUE));
	a -> allocateRect(rc, m_bpp);
	macro -> addCommand(new KisCommandLayerAdd(this, a));

	for (; it != layers.end(); it++) {
		QRect urect = (a) -> imageExtents() & (*it) -> imageExtents();
		QRect rect = urect;
		rect.moveTopLeft(urect.topLeft() - a -> tileExtents().topLeft());

		int minYTile = rect.top() / TILE_SIZE;
		int maxYTile = rect.bottom() / TILE_SIZE;
		int minXTile = rect.left() / TILE_SIZE;
		int maxXTile = rect.right() / TILE_SIZE;

		for (int y = minYTile; y <= maxYTile; y++)
			for(int x = minXTile; x <= maxXTile; x++) {
				KisTileSP dst = a -> getTile(x, y);
				KisTileSP src = (*it) -> getTile(x, y);

				renderTile(dst, src, *it);
			}

		if (m_activeLayer == *it)
			m_activeLayer = m_layers.empty() ? KisLayerSP(0) : m_layers[0];

		removeLayer(*it);
		macro -> addCommand(new KisCommandLayerRm(this, *it));
	}

	addLayer(a);
	addCommand(macro);
	emit layersUpdated();
	compositeImage(rc);
#endif
}

void KisImage::resizeImage(KisPaintDevice *device, const QRect& rect)
{
	int tmpXTiles = device -> xTiles();
	int tmpYTiles = device -> yTiles();

	m_timer -> stop();

	if (tmpXTiles > m_xTiles || tmpYTiles > m_yTiles) {
		destroyPixmap();

		if (tmpXTiles > m_xTiles)
			m_xTiles = tmpXTiles;

		if (tmpYTiles > m_yTiles)
			m_yTiles = tmpYTiles;

		if (rect.width() > m_width)
			m_width = rect.width();

		if (rect.height() > m_height)
			m_height = rect.height();

		resizePixmap(true);
		compositeImage();
	}

	m_timer -> start(1);
}

void KisImage::resizePixmap(bool dirty)
{
	/* note - 32 bit pixmaps may not show up on a 16 bit display
	   and will even crash it !!!  Without depth paramater Qt will
	   use native format. imagePixmap = new QPixmap(w, h, 32); does
	   not work!, so alway default to native display depth when
	   creating a QPixmap.  With a QImage you can create a 32 bit
	   depth with a 16 bit display with no problems, however. */

	m_dirty.resize(m_xTiles * m_yTiles);
	m_pixmapTiles = new QPixmap*[m_xTiles * m_yTiles];

	for (int y = 0; y < m_yTiles; y++)
		for (int x = 0; x < m_xTiles; x++) {
			m_dirty[y * m_xTiles + x] = dirty;
			m_pixmapTiles[y * m_xTiles + x] = new QPixmap(TILE_SIZE, TILE_SIZE);
			m_pixmapTiles[y * m_xTiles + x] -> fill();
		}

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

	if (area.isNull()) {
		maxY = m_yTiles;
		maxX = m_xTiles;
		minY = 0;
		minX = 0;
	}
	else {
		QRect rc = KisUtil::findBoundingTiles(area);

		maxY = rc.bottom() < m_yTiles ? rc.bottom() + 1 : m_yTiles;
		maxX = rc.right() < m_xTiles ? rc.right() + 1 : m_xTiles;
		minY = rc.top();
		minX = rc.left();
	}

	rc.setRight(maxX);
	rc.setBottom(maxY);
	rc.setLeft(minX);
	rc.setTop(minY);
	return rc;
}

KisPaintDeviceSP KisImage::getCurrentPaintDevice()
{
	KisPaintDevice *device = 0;

	if ((device = getCurrentChannel()))
		return device;

	device = getCurrentLayer();
	return device;
}

KisChannelSP KisImage::getCurrentChannel()
{
	return m_activeChannel;
}

KisLayerSP KisImage::getCurrentLayer() 
{ 
	return m_activeLayer; 
}

void KisImage::destroyPixmap()
{
	for (int y = 0; y < m_yTiles; y++)
		for (int x = 0; x < m_xTiles; x++)
			delete m_pixmapTiles[y * m_xTiles + x];

	delete[] m_pixmapTiles;
}

void KisImage::renderBg(KisPaintDevice *srcDevice, int tileNo)
{
	Q_ASSERT(srcDevice);
	KisPixelPacket* region = srcDevice -> getPixels(tileNo, tileNo);

	if (!region)
		return;

	for (int y = 0; y < TILE_SIZE; y++)
		for (int x = 0; x < TILE_SIZE; x++) {
			KisPixelPacket *p = region + y * TILE_SIZE + x;
			uchar v = 128 + 63 * ((x / 16 + y / 16) % 2);

			Q_ASSERT(p);
			p -> red = v;
			p -> green = v;
			p -> blue = v;
			p -> opacity = OpaqueOpacity;
		}

	srcDevice -> syncPixels(region);
	compositeImage();
}

void KisImage::addCommand(KCommand *cmd)
{
	m_doc -> addCommand(cmd);
}

int KisImage::getCurrentLayerIndex() const
{
	int n = 0;

	if (m_layers.empty())
		return -1;

	for (KisLayerSPLstConstIterator it = m_layers.begin(); it != m_layers.end(); it++) {
		if (*it == m_activeLayer)
			return n;

		n++;
	}

	return -1;
}

void KisImage::addChannel(const QRect& rc, uchar opacity, const QString& name)
{
	QRgb defaultColor = qRgba(CHANNEL_MAX, CHANNEL_MAX, CHANNEL_MAX, opacity);
	KisChannelSP chan = new KisChannel(ci_Black, name, m_width, m_height, qGray(defaultColor));

	m_channels.push_back(chan);
	m_activeChannel = chan;
	resizeImage(chan, rc);

#if 0
	if (m_doUndo)
		addCommand(new KisCommandLayerAdd(this, lay));
#endif
}

void KisImage::addChannel(KisChannelSP channel)
{
	if (qFind(m_channels.begin(), m_channels.end(), channel) == m_channels.end()) {
		m_channels.push_back(channel);
		m_activeChannel = channel;
		compositeImage(QRect(0, 0, m_width, m_height), true);
		emit layersUpdated();
	}
}

void KisImage::removeChannel(KisChannelSPLstIterator it)
{
	KisChannelSP channel = *it;

	if (m_activeChannel == channel) {
		if (m_channels.empty())
			m_activeChannel = 0;
		else
			m_activeChannel = m_channels[0];
	}

	m_channels.erase(it);
}

void KisImage::removeChannel(unsigned int channel)
{
	if (channel >= m_channels.size())
		return;

	removeChannel(m_channels.begin() + channel);
}

void KisImage::removeChannel(KisChannelSP channel)
{
	KisChannelSPLstIterator it;

	if ((it = qFind(m_channels.begin(), m_channels.end(), channel)) != m_channels.end()) {
		removeChannel(it);
		compositeImage(QRect(0, 0, m_width, m_height), true);
		emit layersUpdated();
	}
}

int KisImage::getCurrentChannelIndex() const
{
	int n = -1;

	if (m_channels.empty())
		return -1;

	for (KisChannelSPLstConstIterator it = m_channels.begin(); it != m_channels.end(); it++) {
		if (*it == m_activeChannel)
			return n;

		n++;
	}

	return -1;
}

void KisImage::setCurrentChannel(int channel)
{
	if (static_cast<uint>(channel) < m_channels.size()) {
		KisChannelSP p = m_channels[channel];

		setCurrentChannel(p);
	}
}

void KisImage::setCurrentChannel(KisChannelSP channel)
{
	if (channel) {
		m_activeChannel = channel;
		emit layersUpdated();
	}
}

int KisImage::getHishestLayerEver() const
{
	return m_nLayers;
}

#include "kis_image.moc"

