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

#include <qpainter.h>

#include <qthread.h>
#include <qptrqueue.h>
#include <qmutex.h>
#include <qwaitcondition.h>

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

/*
 * Very experimental thread test, only for experimenting
 */

class KisRenderThread : public QThread {
public:
	KisRenderThread(KisImage *img, QWaitCondition *wcDirty)
	{
		m_img = img;
		m_wcDirty = wcDirty;
	}

protected:
	virtual void run()
	{
		kdDebug() << "Thread running.\n";

		while (1) {
			m_wcDirty -> wait();
			m_img -> slotUpdateTimeOut();
		}
	}

	KisImage *m_img;
	QWaitCondition *m_wcDirty;
};

QWaitCondition wcDirty;
QPtrQueue<QPoint> dirtyTiles;
QMutex dirtyTilesMutex;

KisImage::KisImage(const QString& name, int w, int h, cMode cm, uchar bd)
	: m_name(name),
	m_width(w),
	m_height(h),
	m_cMode(cm),
	m_bitDepth(bd)
{
	QRect tileExtents = KisUtil::findTileExtents(QRect(0, 0, m_width, m_height));
	QRgb defaultColor = qRgba(0, 0, 0, 255);

	m_xTiles = tileExtents.width() / TILE_SIZE;
	m_yTiles = tileExtents.height() / TILE_SIZE;
	m_activeLayer = 0;
	m_composeLayer = 0;
	m_bgLayer = 0;
	bd = m_bitDepth = 32; // XXX
	m_autoUpdate = false;

	m_dcop = 0;
	//dcopObject(); // build it
	resizePixmap(false);
	m_imgTile.create(TILE_SIZE, TILE_SIZE, m_bitDepth);

	m_composeLayer = new KisLayer("_compose", TILE_SIZE, TILE_SIZE, bd, cm, defaultColor);
	m_composeLayer -> allocateRect(QRect(0, 0, TILE_SIZE, TILE_SIZE));

	m_bgLayer = new KisLayer("_background", TILE_SIZE, TILE_SIZE, bd, cm, defaultColor);
	m_bgLayer -> allocateRect(QRect(0, 0, TILE_SIZE, TILE_SIZE));;
	renderBg(m_bgLayer, 0);
	compositeImage();

#if 1
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotUpdateTimeOut()));
	m_timer -> start(1);
#else
	QThread *thr = new KisRenderThread(this, &wcDirty);
	thr -> start();
#endif
}

/*
    KisImage destructor - note that various arrays must be
    deleted as these are not children of the QObject for
    KisImage, but are regular arrays and pointers to them.
*/

KisImage::~KisImage()
{
	destroyPixmap();
	delete m_composeLayer;
	delete m_bgLayer;
	delete m_dcop;

	while (m_layers.count()) {
		KisLayer *p = m_layers.first();

		m_layers.remove(p);
		delete p;
	}

	while (m_layers.count()) {
		KisChannel *p = m_channels.first();

		m_channels.remove(p);
		delete p;
	}
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
	KisLayer *lay;
	QRgb defaultColor;

	if (getCurrentLayer())
		defaultColor = qRgba(255, 255, 255, 0);
	else
		defaultColor = c.color().rgb();
	
	lay = new KisLayer(name, m_width, m_height, m_bitDepth, m_cMode, defaultColor);
	lay -> allocateRect(rect);
	lay -> clear(c, tr);
	m_layers.append(lay);
	m_activeLayer = lay;
	resizeImage(lay, rect);
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
			//dirtyTilesMutex.lock();
			m_dirty[y * m_xTiles + x] = true;
			//dirtyTilesMutex.unlock();

			if (m_autoUpdate)
				compositeImage(QRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE));
#if 0
			else {
				dirtyTilesMutex.lock();
				dirtyTiles.enqueue(new QPoint(x, y));
				dirtyTilesMutex.unlock();
			}
#endif
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
	// TODO : Go over tiles in m_dirtyTiles
	// if tile is dirty,
	for(int y = 0; y < m_yTiles; y++)
		for(int x = 0; x < m_xTiles; x++)
			if (m_dirty[y * m_xTiles + x])
				compositeImage(QRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE));
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

void KisImage::compositeTile(KisPaintDevice *dstDevice, int tileNo, int x, int y)
{
	KisTile *dst = dstDevice -> getTile(tileNo, tileNo);

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

			compositeTile(m_composeLayer, 0, x, y);
			convertTileToPixmap(m_composeLayer, 0, m_pixmapTiles[y * m_xTiles + x]);
			m_dirty[y * m_xTiles + x] = false;
		}
	}

	emit updated(area);
}

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

void KisImage::convertTileToPixmap(KisPaintDevice *dstDevice, int tileNo, QPixmap *pix)
{
	KisTile *tile = dstDevice -> getTile(tileNo, tileNo);
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
	QPtrList<KisLayer> l(m_layers);
	
	mergeLayers(l);
}

/*
    mergeVisibleLayers - merge only those layers which are visible.
    Merge everything into the first visible layer encountered.
*/

void KisImage::mergeVisibleLayers()
{
	QPtrList<KisLayer> l;

	for (KisLayer *lay = m_layers.first(); lay; lay = m_layers.next()) {
		if (lay -> visible())
			l.append(lay);
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

	for (KisLayer *lay = m_layers.first(); lay; lay = m_layers.next()) {
		if (lay -> linked())
			l.append(lay);
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
	KisLayer *a = list.first();
	KisLayer *b;
	QRect newRect;

	while (a) {
		newRect.unite(a -> imageExtents());
		a = list.next();
	}

	while ((a = list.first()) && (b = list.next())) {
		if (!a || !b) 
			break;

		QRect urect = a -> imageExtents() | b -> imageExtents();

		// allocate out tiles if required
		a -> allocateRect(urect);
		b -> allocateRect(urect);

		// rect in layer coords (offset from tileExtents.topLeft())
		QRect rect = urect;
		rect.moveTopLeft(urect.topLeft() - a->tileExtents().topLeft());

		// workout which tiles in the layer need to be updated
		int minYTile = rect.top() / TILE_SIZE;
		int maxYTile = rect.bottom() / TILE_SIZE;
		int minXTile = rect.left() / TILE_SIZE;
		int maxXTile = rect.right() / TILE_SIZE;

		for (int y = minYTile; y <= maxYTile; y++) {
			for(int x = minXTile; x <= maxXTile; x++) {
				KisTile *dst = a -> getTile(x, y);
				KisTile *src = b -> getTile(x, y);

				renderTile(dst, src, b);
			}
		}

		list.remove(b);
		m_layers.remove(b);

		if (m_activeLayer == b)
			m_activeLayer = m_layers.count() ? m_layers.at(0) : 0;

		delete b;
	}

	emit layersUpdated();
	compositeImage(newRect);
}

void KisImage::resizeImage(KisLayer *lay, const QRect& rect)
{
	int tmpXTiles = lay -> xTiles();
	int tmpYTiles = lay -> yTiles();

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

void KisImage::renderTile(KisTile *dst, const KisTile *src, const KisPaintDevice *srcDevice)
{
	uchar src_opacity = srcDevice -> opacity();
	uchar opacity;
	uchar inverseOpacity;
	const QRgb *srgb;
	QRgb *drgb;
	uchar sr, sg, sb, sa;
	uchar dr, dg, db, da;

	if (!src -> data())
		return;
	
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

void KisImage::destroyPixmap()
{
	for (int y = 0; y < m_yTiles; y++)
		for (int x = 0; x < m_xTiles; x++)
			delete m_pixmapTiles[y * m_xTiles + x];

	delete[] m_pixmapTiles;
}

void KisImage::renderBg(KisPaintDevice *srcDevice, int tileNo)
{
	uint *ptr = srcDevice -> getTile(tileNo, tileNo) -> data();

	for (int y = 0; y < TILE_SIZE; y++)
		for (int x = 0; x < TILE_SIZE; x++) {
			uchar v = 128 + 63 * ((x / 16 + y / 16) % 2);

			*(ptr + (y * TILE_SIZE) + x) = qRgba(v, v, v, 255);
		}

	compositeImage();
}

#include "kis_image.moc"

