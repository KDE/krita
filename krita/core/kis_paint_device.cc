/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qrect.h>
#include <qwmatrix.h>

#include <kcommand.h>
#include <klocale.h>
#include <kdebug.h>

#include <koStore.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kispixeldata.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_strategy_colorspace.h"
#include "kis_colorspace_factory.h"
#include "kis_iterators.h"

namespace {
        class KisResizeDeviceCmd : public KNamedCommand {
                typedef KNamedCommand super;

        public:
                KisResizeDeviceCmd(KisUndoAdapter *adapter, KisPaintDeviceSP dev, KisTileMgrSP before, KisTileMgrSP after) : super(i18n("Resize"))
                {
                        m_adapter = adapter;
                        m_dev = dev;
                        m_before = before;
                        m_after = after;
                }

                virtual ~KisResizeDeviceCmd()
                {
                }

        public:
                virtual void execute()
                {
                        KisImageSP owner;

                        m_adapter -> setUndo(false);
                        m_dev -> data(m_after);
                        m_adapter -> setUndo(true);
                        owner = m_dev -> image();
                        owner -> notify();
                }

                virtual void unexecute()
                {
                        KisImageSP owner;

                        m_adapter -> setUndo(false);
                        m_dev -> data(m_before);
                        m_adapter -> setUndo(true);
                        owner = m_dev -> image();
                        owner -> notify();
                }

        private:
                KisUndoAdapter *m_adapter;
                KisPaintDeviceSP m_dev;
                KisTileMgrSP m_before;
                KisTileMgrSP m_after;
        };
}

KisPaintDevice::KisPaintDevice(Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name)
{
        init();
        m_width = width;
        m_height = height;
        m_imgType = imgType;
        m_alpha = ::imgTypeHasAlpha(imgType);
        m_depth = ::imgTypeDepth(imgType);
        m_x = 0;
        m_y = 0;
        m_offX = 0;
        m_offY = 0;
        m_offW = 0;
        m_offH = 0;
        m_tiles = new KisTileMgr(m_depth, width, height);
        m_visible = true;
        m_owner = 0;
        m_name = name;
        m_projectionValid = false;
	m_compositeOp = COMPOSITE_OVER;

	KisColorSpaceFactoryInterface *factory = KisColorSpaceFactoryInterface::singleton();
	Q_ASSERT(factory);
	m_colorStrategy = factory -> create(m_imgType);
}

KisPaintDevice::KisPaintDevice(KisImageSP img, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name)
{
        init();
        configure(img, width, height, imgType, name, COMPOSITE_OVER);
}

KisPaintDevice::KisPaintDevice(KisTileMgrSP tm, KisImageSP img, const QString& name)
{
        init();
        m_width = tm -> width();
        m_height = tm -> height();
        m_imgType = img -> imgType();
        m_depth = img -> depth();
        m_alpha = img -> alpha();
        m_x = 0;
        m_y = 0;
        m_offX = 0;
        m_offY = 0;
        m_offW = 0;
        m_offH = 0;
        m_tiles = tm;
        m_visible = true;
        m_owner = img;
        m_name = name;
        m_projectionValid = false;
	m_compositeOp = COMPOSITE_OVER;

	KisColorSpaceFactoryInterface *factory = KisColorSpaceFactoryInterface::singleton();
	Q_ASSERT(factory);
	m_colorStrategy = factory -> create(m_imgType);
}

KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs) : QObject(), super(rhs)
{
        if (this != &rhs) {
                m_owner = 0;

                if (rhs.m_tiles)
                        m_tiles = new KisTileMgr(*rhs.m_tiles);

                if (rhs.m_shadow)
                        m_shadow = new KisTileMgr(*rhs.m_shadow);

                m_visible = rhs.m_visible;
                m_x = rhs.m_x;
                m_y = rhs.m_y;
                m_width = rhs.m_width;
                m_height = rhs.m_height;
                m_depth = rhs.m_depth;
                m_offX = rhs.m_offX;
                m_offY = rhs.m_offY;
                m_offW = rhs.m_offW;
                m_offH = rhs.m_offH;
                m_quantumSize = rhs.m_quantumSize;
                m_imgType = rhs.m_imgType;
                m_alpha = rhs.m_alpha;
                m_projectionValid = false;
                m_name = rhs.m_name;
		m_compositeOp = COMPOSITE_OVER;
		m_colorStrategy = rhs.m_colorStrategy;
        }
}

KisPaintDevice::~KisPaintDevice()
{
}

Q_INT32 KisPaintDevice::tileNum(Q_INT32, Q_INT32) const
{
        return 0;
}

void KisPaintDevice::validate(Q_INT32)
{
}

void KisPaintDevice::invalidate(Q_INT32 tileno)
{
        data() -> invalidate(tileno);
}

void KisPaintDevice::invalidate(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
        Q_INT32 dx = x + w + 1;
        Q_INT32 dy = y + h + 1;
        Q_INT32 x1;
        Q_INT32 y1;

        m_projectionValid = false;

        for (y1 = y; y1 < dy; y1 += TILE_HEIGHT - y1 % TILE_HEIGHT)
                for (x1 = x; x1 < dx; x1 += TILE_WIDTH - x1 % TILE_WIDTH)
                        data() -> invalidate(x1, y1);
}

void KisPaintDevice::invalidate(const QRect& rc)
{
        invalidate(rc.x(), rc.y(), rc.width(), rc.height());
}

void KisPaintDevice::invalidate()
{
        invalidate(0, 0, width(), height());
}

void KisPaintDevice::configure(KisImageSP image, 
			       Q_INT32 width, Q_INT32 height, 
			       const enumImgType& imgType, 
			       const QString& name, 
			       CompositeOp compositeOp)
{
        if (image == 0 || name.isEmpty())
                return;

        m_width = width;
        m_height = height;
        m_imgType = imgType;
        m_depth = image -> depth();
        m_alpha = image -> alpha();
        m_x = 0;
        m_y = 0;
        m_offX = 0;
        m_offY = 0;
        m_offW = 0;
        m_offH = 0;
        m_tiles = new KisTileMgr(m_depth, width, height);
        m_visible = true;
        m_owner = image;
        m_name = name;
        m_projectionValid = false;
	kdDebug() << "composite op: " << compositeOp << "\n";
	m_compositeOp = compositeOp;	

	KisColorSpaceFactoryInterface *factory = KisColorSpaceFactoryInterface::singleton();
	Q_ASSERT(factory);
	m_colorStrategy = factory -> create(m_imgType);
}

void KisPaintDevice::update()
{
        update(0, 0, width(), height());
}

void KisPaintDevice::update(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
        if (x < m_offX)
                x = m_offX;

        if (y < m_offY)
                y = m_offY;

        if (w > m_offW)
                w = m_offW;

        if (h > m_offH)
                h = m_offH;

        invalidate(x, y, w, h);
}

void KisPaintDevice::move(Q_INT32 x, Q_INT32 y)
{
        m_x = x;
        m_y = y;
        emit positionChanged(this);
}

void KisPaintDevice::move(const QPoint& pt)
{
        move(pt.x(), pt.y());
}

bool KisPaintDevice::contains(Q_INT32 x, Q_INT32 y) const
{
        QRect rc(m_x, m_y, m_width, m_height);

        return rc.contains(x, y);
}

bool KisPaintDevice::contains(const QPoint& pt) const
{
        return contains(pt.x(), pt.y());
}

bool KisPaintDevice::shouldDrawBorder() const
{
        return false;
}

QString KisPaintDevice::name()
{
        return m_name;
}

void KisPaintDevice::setName(const QString& name)
{
        if (!name.isEmpty())
                m_name = name;
}

void KisPaintDevice::maskBounds(Q_INT32 *, Q_INT32 *, Q_INT32 *, Q_INT32 *)
{
}

void KisPaintDevice::maskBounds(QRect *rc)
{
        Q_INT32 x1;
        Q_INT32 y1;
        Q_INT32 x2;
        Q_INT32 y2;

        maskBounds(&x1, &y1, &x2, &y2);
        rc -> setRect(x1, y1, x2 - x1, y2 - y1);
}

bool KisPaintDevice::alpha() const
{
        return m_alpha;
}

enumImgType KisPaintDevice::type() const {
	return m_imgType;
}

enumImgType KisPaintDevice::typeWithoutAlpha() const
{
        switch (m_imgType) {
        case IMAGE_TYPE_INDEXEDA:
                return IMAGE_TYPE_INDEXED;
        case IMAGE_TYPE_GREYA:
                return IMAGE_TYPE_GREY;
        case IMAGE_TYPE_RGBA:
                return IMAGE_TYPE_RGB;
        case IMAGE_TYPE_CMYKA:
                return IMAGE_TYPE_CMYK;
        case IMAGE_TYPE_LABA:
                return IMAGE_TYPE_LAB;
        case IMAGE_TYPE_YUVA:
                return IMAGE_TYPE_YUV;
        default:
                return m_imgType;
        }

        return m_imgType;
}

enumImgType KisPaintDevice::typeWithAlpha() const
{
        switch (m_imgType) {
                case IMAGE_TYPE_INDEXED:
                        return IMAGE_TYPE_INDEXEDA;
                case IMAGE_TYPE_GREY:
                        return IMAGE_TYPE_GREYA;
                case IMAGE_TYPE_RGB:
                        return IMAGE_TYPE_RGBA;
                case IMAGE_TYPE_CMYK:
                        return IMAGE_TYPE_CMYKA;
                case IMAGE_TYPE_LAB:
                        return IMAGE_TYPE_LABA;
                case IMAGE_TYPE_YUV:
                        return IMAGE_TYPE_YUVA;
                default:
                        return m_imgType;
        }

        return m_imgType;

}

KisTileMgrSP KisPaintDevice::data()
{
        return m_tiles;
}

const KisTileMgrSP KisPaintDevice::data() const
{
        return m_tiles;
}

KisTileMgrSP KisPaintDevice::shadow()
{
        return m_shadow;
}

const KisTileMgrSP KisPaintDevice::shadow() const
{
        return m_shadow;
}

Q_INT32 KisPaintDevice::quantumSize() const
{
        return 0;
}

Q_INT32 KisPaintDevice::quantumSizeWithAlpha() const
{
        return 0;
}

QRect KisPaintDevice::bounds() const
{
        return QRect(m_x, m_y, m_width, m_height);
}

Q_INT32 KisPaintDevice::x() const
{
        return m_x;
}

void KisPaintDevice::setX(Q_INT32 x)
{
        m_x = x;
}

Q_INT32 KisPaintDevice::y() const
{
        return m_y;
}

void KisPaintDevice::setY(Q_INT32 y)
{
        m_y = y;
}

Q_INT32 KisPaintDevice::width() const
{
        return m_width;
}

Q_INT32 KisPaintDevice::height() const
{
        return m_height;
}

const bool KisPaintDevice::visible() const
{
        return m_visible;
}

void KisPaintDevice::visible(bool v)
{
        if (m_visible != v) {
                m_visible = v;
                emit visibilityChanged(this);
        }
}

QRect KisPaintDevice::clip() const
{
        return QRect(m_offX, m_offY, m_offW, m_offH);
}

void KisPaintDevice::clip(Q_INT32 *offx, Q_INT32 *offy, Q_INT32 *offw, Q_INT32 *offh) const
{
        if (offx && offy && offw && offh) {
                *offx = m_offX;
                *offy = m_offY;
                *offw = m_offW;
                *offh = m_offH;
        }
}

void KisPaintDevice::setClip(Q_INT32 offx, Q_INT32 offy, Q_INT32 offw, Q_INT32 offh)
{
        m_offX = offx;
        m_offY = offy;
        m_offW = offw;
        m_offH = offh;
}

bool KisPaintDevice::cmap(KoColorMap& cm)
{
        cm.clear();
        return false;
}

KoColor KisPaintDevice::colorAt()
{
        return KoColor();
}

KisImageSP KisPaintDevice::image()
{
        return m_owner;
}

const KisImageSP KisPaintDevice::image() const
{
        return m_owner;
}

void KisPaintDevice::setImage(KisImageSP image)
{
        m_owner = image;
}

void KisPaintDevice::init()
{
        m_visible = false;
        m_width = 0;
        m_height = 0;
        m_depth = 0;
        m_alpha = false;
        m_quantumSize = 0;
        m_offX = 0;
        m_offY = 0;
        m_offW = 0;
        m_offH = 0;
        m_x = 0;
        m_y = 0;
        m_projectionValid = false;
}

bool KisPaintDevice::pixel(Q_INT32 x, Q_INT32 y, KoColor *c, QUANTUM *opacity)
{
	// XXX: this should use the colour strategies!

        KisTileMgrSP tm = data();
        KisPixelDataSP pd = tm -> pixelData(x - m_x, y - m_y, x - m_x, y - m_y, TILEMODE_READ);
        QUANTUM *data;
        Q_INT32 tmp;

        if (!pd)
                return false;

        *opacity = OPACITY_OPAQUE;
        data = pd -> data;
        Q_ASSERT(data);

        switch (m_alpha ? typeWithAlpha() : typeWithoutAlpha()) {
        case IMAGE_TYPE_INDEXEDA:
        case IMAGE_TYPE_INDEXED:
                break; // TODO
        case IMAGE_TYPE_GREYA:
                *opacity = data[PIXEL_GRAY_ALPHA];
        case IMAGE_TYPE_GREY:
                tmp = downscale(data[PIXEL_GRAY]);
                c -> setRGB(tmp, tmp, tmp);
                break;
        case IMAGE_TYPE_RGBA:
                *opacity = data[PIXEL_ALPHA];
        case IMAGE_TYPE_RGB:
                c -> setRGB(downscale(data[PIXEL_RED]), downscale(data[PIXEL_GREEN]), downscale(data[PIXEL_BLUE]));
                break;
        default:
                kdDebug() << "Not Implemented.\n";
                break;
        }

        return true;
}

bool KisPaintDevice::setPixel(Q_INT32 x, Q_INT32 y, const KoColor& c, QUANTUM opacity)
{

	// XXX: this should use the colour strategies! 
        KisTileMgrSP tm = data();
        KisPixelDataSP pd = tm -> pixelData(x - m_x, y - m_y, x - m_x, y - m_y, TILEMODE_WRITE);
        QUANTUM *data;

        if (!pd)
                return false;

        data = pd -> data;
        Q_ASSERT(data);

        switch (m_alpha ? typeWithAlpha() : typeWithoutAlpha()) {
        case IMAGE_TYPE_INDEXEDA:
        case IMAGE_TYPE_INDEXED:
                break; // TODO
        case IMAGE_TYPE_GREYA:
                data[PIXEL_GRAY_ALPHA] = opacity;
        case IMAGE_TYPE_GREY:
                data[PIXEL_GRAY] = upscale(c.R());
                break;
        case IMAGE_TYPE_RGBA:
                data[PIXEL_ALPHA] = opacity;
        case IMAGE_TYPE_RGB:
                data[PIXEL_RED] = upscale(c.R());
                data[PIXEL_GREEN] = upscale(c.G());
                data[PIXEL_BLUE] = upscale(c.B());
                break;
        default:
                kdDebug() << "Not Implemented.\n";
                break;
        }

        tm -> releasePixelData(pd);
        return true;
}

void KisPaintDevice::data(KisTileMgrSP mgr)
{
        Q_ASSERT(mgr);

        if (m_owner) {
                KisUndoAdapter *doc = m_owner -> undoAdapter();

                if (doc && doc -> undo())
                        doc -> addCommand(new KisResizeDeviceCmd(doc, this, m_tiles, mgr));
        }

        m_tiles = mgr;
        width(mgr -> width());
        height(mgr -> height());
}

void KisPaintDevice::width(Q_INT32 w)
{
        m_width = w;
}

void KisPaintDevice::height(Q_INT32 h)
{
        m_height = h;
}

void KisPaintDevice::resize(Q_INT32 w, Q_INT32 h)
{
        KisTileMgrSP old = data();
        KisTileMgrSP tm = new KisTileMgr(old, old -> depth(), w, h);
        Q_INT32 oldW = width();
        Q_INT32 oldH = height();
        KisPainter gc;

        data(tm);
        width(w);
        height(h);
        gc.begin(this);

        if (oldW < w)
                gc.eraseRect(oldW, 0, w, h);

        if (oldH < h)
                gc.eraseRect(0, oldH, w, h);

        gc.end();
}

void KisPaintDevice::resize(const QSize& size)
{
        resize(size.width(), size.height());
}

void KisPaintDevice::resize()
{
        KisImageSP img = image();

        if (img)
                resize(img -> bounds().size());
}

// XXX: also allow transform on part of paint device?
void KisPaintDevice::transform(const QWMatrix & matrix)
{
	if (data() == 0) {
		kdDebug() << "No tilemgr.\n";
		return;
	}

	/* No, we're NOT duplicating the entire image, at the moment krita uses
	   too much memory already */
	QUANTUM *origPixel = new QUANTUM[depth() * sizeof(QUANTUM)];

	// target image data
	Q_INT32 targetW;
	Q_INT32 targetH;

	// compute size of target image
	// (this bit seems to be mostly from QImage.xForm)
	QWMatrix mat = QPixmap::trueMatrix( matrix, width(), height() );
	if ( mat.m12() == 0.0F && mat.m21() == 0.0F ) {
		kdDebug() << "Scaling.\n";
		if ( mat.m11() == 1.0F && mat.m22() == 1.0F ) { 
			kdDebug() << "Identity matrix, do nothing.\n";
			return;
		}
		targetW = qRound( mat.m11() * width() );
		targetH = qRound( mat.m22() * height() );
		targetW = QABS( targetW );
		targetH = QABS( targetH );
	} else {
		kdDebug() << "rotation or shearing\n";
		QPointArray a( QRect(0, 0, width(), height()) );
		a = mat.map( a );
		QRect r = a.boundingRect().normalize();
		targetW = r.width();
		targetH = r.height();
	}
	
	// Create target pixel buffer which we'll read into a tile manager
	// when done.
	QUANTUM * newData = new QUANTUM[targetW * targetH * depth() * sizeof(QUANTUM)];
	/* This _has_ to be fixed; horribly layertype dependent */
	memset(newData, targetW * targetH * depth() * sizeof(QUANTUM), 0);

	bool invertible;
	QWMatrix targetMat = mat.invert( &invertible ); // invert matrix
	if ( targetH == 0 || targetW == 0 || !invertible ) {
		kdDebug() << "Error, return null image\n";
		return;
	}

	// I would have thought that one would take the source pixels,
	// do the computation, and write the target pixels.
	// Apparently, that's not true: one looks through the target
	// pixels, and computes what should be there from the source
	// pixels. I doubt I will ever completely understand this
	// stuff.
	// BC: I guess this makes it easier to make the transform anti-aliased,
	// as you can easily take the weighted mean of the square the destination
	// pixel has it's origin in.

	/* For each target line of target pixels, the original pixel is located (in the
	   surrounding of)
	   x = m11*x' + m21*y' + dx
           y = m22*y' + m12*x' + dy
	*/

	for (Q_INT32 y = 0; y < targetH; y++) {
		/* at the moment, just round the original coordinates; but the unrounded
		   values should be used for AA */
		for (Q_INT32 x = 0; x < targetW; x++) {
			Q_INT32 orX = qRound(targetMat.m11() * x + targetMat.m21() * y + targetMat.dx());
			Q_INT32 orY = qRound(targetMat.m22() * y + targetMat.m12() * x + targetMat.dy());

			int currentPos = (y*targetW+x) * depth(); // try to be at least a little efficient
			if (!(orX < 0 || orY < 0 || orX >= width() || orY >= height())) {
				data() -> readPixelData(orX, orY, orX, orY, origPixel, depth());
				for(int i = 0; i < depth(); i++)
					newData[currentPos + i] = origPixel[i];
			}
		}
	}
	
        KisTileMgrSP tm = new KisTileMgr(depth(), targetW, targetH);
	tm -> writePixelData(0, 0, targetW - 1, targetH - 1, newData, targetW * depth());
	data(tm); // Also set width and height correctly

	delete[] origPixel;
	delete[] newData;

}

void KisPaintDevice::mirrorX()
{
	/* For each line, swap the liness at equal distances from the X axis*/
	/* Should be bit depth independent, but I don't have anything to test that with.
	   I don't know about colour strategy, but if bit depth works that should too */

	QUANTUM *line1 = new QUANTUM[width() * depth() * sizeof(QUANTUM)];
	QUANTUM *line2 = new QUANTUM[width() * depth() * sizeof(QUANTUM)];
	KisTileMgrSP tm = new KisTileMgr(depth(), width(), height());

	int cutoff = static_cast<int>(height()/2);

	for(int i = 0; i < cutoff; i++) {
		data() -> readPixelData(0, i, width() - 1, i, line1, width() * depth());
		data() -> readPixelData(0, height() - i - 1, width() - 1, height() - i - 1, line2, width() * depth());
		tm -> writePixelData(0, height() - i - 1, width() - 1, height() - i - 1, line1, width() * depth());
		tm -> writePixelData(0, i, width() - 1, i, line2, width() * depth());
	}

	data(tm);

	delete[] line1;
	delete[] line2;
}

void KisPaintDevice::mirrorY()
{
	/* For each line, swap the pixels at equal distances from the Y axis */
	/* Note: I get the idea that this could be done faster with direct access to
	   the pixel data. Now I have to copy the pixels twice only to get them
	   and put them back in place. */
	/* Should be bit depth and arch independent, but I don't have anything to test
	   that with I don't know about colour strategy, but if bit depth works that
	   should too */
	QUANTUM *pixel = new QUANTUM[depth() * sizeof(QUANTUM)]; // the right pixel
	QUANTUM *line = new QUANTUM[width() * depth() * sizeof(QUANTUM)];
	KisTileMgrSP tm = new KisTileMgr(depth(), width(), height());
	int cutoff = static_cast<int>(width()/2);

	for(int i = 0; i < height(); i++) {
		data() -> readPixelData(0, i, width() - 1, i, line, width() * depth());
		for(int j = 0; j < cutoff; j++) {
			for(int k = 0; k < depth(); k++) {
				pixel[k] = line[(width()-1)*depth() - j*depth() + k];
				line[(width()-1)*depth() - j*depth() + k] = line[j*depth()+k];
				line[j*depth()+k] = pixel[k];
			}
		}
		tm -> writePixelData(0, i, width() - 1, i, line, width() * depth());
	}
	
	data(tm); // Act like this is a resize; this should get it's own undo 'name'
	
	delete[] line;
	delete[] pixel;
}

void KisPaintDevice::expand(Q_INT32 w, Q_INT32 h)
{
        w = QMAX(w, width());
        h = QMAX(h, height());
        resize(w, h);
}

void KisPaintDevice::expand(const QSize& size)
{
        expand(size.width(), size.height());
}

void KisPaintDevice::anchor()
{
}

void KisPaintDevice::offsetBy(Q_INT32 x, Q_INT32 y)
{
        if (x < 0)
                x = 0;

        if (y < 0)
                y = 0;

        KisTileMgrSP old = data();
        KisTileMgrSP tm = new KisTileMgr(old -> depth(), x + old -> width(), y + old -> height());
        KisPixelDataSP dst;
        KisPixelDataSP src;

        data(tm);
        width(tm -> width());
        height(tm -> height());
        src = old -> pixelData(0, 0, old -> width() - 1, old -> height() - 1, TILEMODE_READ);
        Q_ASSERT(src);
        dst = tm -> pixelData(x, y, x + old -> width() - 1, y + old -> height() - 1, TILEMODE_WRITE);
        Q_ASSERT(dst);
        memcpy(dst -> data, src -> data, sizeof(QUANTUM) * src -> width * src -> height * src -> depth);
        tm -> releasePixelData(dst);
        m_x -= x;
        m_y -= y;
}

bool KisPaintDevice::write(KoStore *store)
{
        KisTileMgrSP tm = data();
        Q_INT32 totalBytes = m_height * m_width * m_depth * sizeof(QUANTUM);
        Q_INT32 nbytes = 0;

        Q_ASSERT(store);
        Q_ASSERT(tm);

        for (Q_INT32 y = 0; y < m_height; y += TILE_HEIGHT) {
                for (Q_INT32 x = 0; x < m_width; x += TILE_WIDTH) {
                        KisTileSP tile = tm -> tile(x, y, TILEMODE_READ);
                        Q_INT32 tileBytes = tile -> height() * tile -> width() * m_depth * sizeof(QUANTUM);

                        tile -> lock();

                        if (store -> write(reinterpret_cast<char*>(tile -> data()), tileBytes) != tileBytes) {
                                tile -> release();
                                return false;
                        }

                        tile -> release();
                        nbytes += tileBytes;
                        emit ioProgress(nbytes * 100 / totalBytes);
                }
        }

        return true;
}

bool KisPaintDevice::read(KoStore *store)
{
        KisTileMgrSP tm = data();
        Q_INT32 totalBytes = m_height * m_width * m_depth * sizeof(QUANTUM);
        Q_INT32 nbytes = 0;

        Q_ASSERT(store);
        Q_ASSERT(tm);

        for (Q_INT32 y = 0; y < m_height; y += TILE_HEIGHT) {
                for (Q_INT32 x = 0; x < m_width; x += TILE_WIDTH) {
                        KisTileSP tile = tm -> tile(x, y, TILEMODE_WRITE);
                        Q_INT32 tileBytes = tile -> height() * tile -> width() * m_depth * sizeof(QUANTUM);

                        tile -> lock();

                        if (store -> read(reinterpret_cast<char*>(tile -> data()), tileBytes) != tileBytes) {
                                tile -> release();
                                return false;
                        }

                        tile -> release();
                        nbytes += tileBytes;
                        emit ioProgress(nbytes * 100 / totalBytes);
                }
        }

        return true;
}

KisTileMgrSP KisPaintDevice::tiles() const
{
        return m_tiles;
}

Q_INT32 KisPaintDevice::depth() const
{
	return m_depth;
}

KisStrategyColorSpaceSP KisPaintDevice::colorStrategy() const
{
	return m_colorStrategy;
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumBegin(KisTileCommand* command)
{
	return KisIteratorLineQuantum( this, command, 0);
}
KisIteratorLineQuantum KisPaintDevice::iteratorQuantumEnd(KisTileCommand* command)
{
	return KisIteratorLineQuantum( this, command, height() - 1);
}
KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionBegin(KisTileCommand* command)
{
	return KisIteratorLineQuantum( this, command, 0);
}
KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionEnd(KisTileCommand* command)
{
	return KisIteratorLineQuantum( this, command, height() - 1);
}

#include "kis_paint_device.moc"

