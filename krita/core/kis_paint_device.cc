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
#include <qimage.h>

#include <kcommand.h>
#include <klocale.h>
#include <kdebug.h>

#include <koStore.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kistile.h"
#include "kispixeldata.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_quantum.h"
#include "kis_iterators_pixel.h"

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
                        m_dev -> setTiles(m_after);
                        m_adapter -> setUndo(true);
                        owner = m_dev -> image();
                        owner -> notify();
                }

                virtual void unexecute()
                {
                        KisImageSP owner;

                        m_adapter -> setUndo(false);
                        m_dev -> setTiles(m_before);
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

KisPaintDevice::KisPaintDevice(Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorStrategy, const QString& name)
{
	init();
	m_x = 0;
	m_y = 0;
	m_offX = 0;
	m_offY = 0;
	m_offW = 0;
	m_offH = 0;
	m_tiles = new KisTileMgr(colorStrategy -> depth(), width, height);
	m_visible = true;
	m_owner = 0;
	m_name = name;
	m_compositeOp = COMPOSITE_OVER;
	m_colorStrategy = colorStrategy;
}

KisPaintDevice::KisPaintDevice(KisImage *img, Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorStrategy, const QString& name)
{
        init();
        configure(img, width, height, colorStrategy, name, COMPOSITE_OVER);
}

KisPaintDevice::KisPaintDevice(KisTileMgrSP tm, KisImage *img, const QString& name)
{
        init();
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
        m_compositeOp = COMPOSITE_OVER;
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
                m_offX = rhs.m_offX;
                m_offY = rhs.m_offY;
                m_offW = rhs.m_offW;
                m_offH = rhs.m_offH;
                m_quantumSize = rhs.m_quantumSize;
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

void KisPaintDevice::configure(KisImage *image,
                               Q_INT32 width, Q_INT32 height,
                               KisStrategyColorSpaceSP colorStrategy,
                               const QString& name,
                               CompositeOp compositeOp)
{
	if (image == 0 || name.isEmpty())
		return;

	m_x = 0;
	m_y = 0;
	m_offX = 0;
	m_offY = 0;
	m_offW = 0;
	m_offH = 0;
	m_tiles = new KisTileMgr(colorStrategy -> depth(), width, height);
	m_visible = true;
	m_owner = image;
	m_name = name;
	m_compositeOp = compositeOp;
	m_colorStrategy = colorStrategy;
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
        QRect rc(m_x, m_y, width(), height());

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

QString KisPaintDevice::name() const
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

void KisPaintDevice::init()
{
        m_visible = false;
        m_quantumSize = 0;
        m_offX = 0;
        m_offY = 0;
        m_offW = 0;
        m_offH = 0;
        m_x = 0;
        m_y = 0;
	m_colorStrategy = 0;
}


void KisPaintDevice::setTiles(KisTileMgrSP mgr)
{
        Q_ASSERT(mgr);

        if (m_owner) {
                KisUndoAdapter *doc = m_owner -> undoAdapter();

                if (doc && doc -> undo())
                        doc -> addCommand(new KisResizeDeviceCmd(doc, this, m_tiles, mgr));
        }

        m_tiles = mgr;
}


void KisPaintDevice::resize(Q_INT32 w, Q_INT32 h)
{
        KisTileMgrSP old = tiles();
        KisTileMgrSP tm = new KisTileMgr(old, colorStrategy() -> depth(), w, h);
        Q_INT32 oldW = width();
        Q_INT32 oldH = height();
        KisFillPainter painter;

        setTiles(tm);

        painter.begin(this);

        if (oldW < w)
                painter.eraseRect(oldW, 0, w, h);

        if (oldH < h)
                painter.eraseRect(0, oldH, w, h);

        painter.end();
}

void KisPaintDevice::scale(double sx, double sy) 
{
	QWMatrix m;
	m.scale(sx, sy);
	transform(m);

}

// XXX: also allow transform on part of paint device?
void KisPaintDevice::transform(const QWMatrix & matrix)
{
        if (tiles() == 0) {
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
                kdDebug() << "Scaling layer: " << m_name << "\n";
                if ( mat.m11() == 1.0F && mat.m22() == 1.0F ) {
                        kdDebug() << "Identity matrix, do nothing.\n";
                        return;
                }
                targetW = qRound( mat.m11() * width() );
                targetH = qRound( mat.m22() * height() );
                targetW = QABS( targetW );
                targetH = QABS( targetH );
        } else {
                kdDebug() << "Rotating or shearing layer " << m_name << "\n";
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
        // XXX: according to man memset, this param order is wrong.
        // memset(newData, targetW * targetH * depth() * sizeof(QUANTUM), 0);
        memset(newData, 0, targetW * targetH * depth() * sizeof(QUANTUM));

        bool invertible;
        QWMatrix targetMat = mat.invert( &invertible ); // invert matrix
        if ( targetH == 0 || targetW == 0 || !invertible ) {
                kdDebug() << "Error, return null image\n";
                return;
        }

        // BSAR: I would have thought that one would take the source pixels,
        // do the computation, and write the target pixels.
        // Apparently, that's not true: one looks through the target
        // pixels, and computes what should be there from the source
        // pixels. I doubt I will ever completely understand this
        // stuff.
        // BC: I guess this makes it easier to make the transform anti-aliased,
        // as you can easily take the weighted mean of the square the destination
        // pixel has it's origin in.
	// BSAR: Ah, ok. I get it now...

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
                                tiles() -> readPixelData(orX, orY, orX, orY, origPixel, depth());
                                for(int i = 0; i < depth(); i++)
                                        newData[currentPos + i] = origPixel[i];
                        }
                }
        }

        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), targetW, targetH);
        tm -> writePixelData(0, 0, targetW - 1, targetH - 1, newData, targetW * depth());
        setTiles(tm); // Also sets width and height correctly

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
        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), width(), height());

        int cutoff = static_cast<int>(height()/2);

        for(int i = 0; i < cutoff; i++) {
                tiles() -> readPixelData(0, i, width() - 1, i, line1, width() * depth());
                tiles() -> readPixelData(0, height() - i - 1, width() - 1, height() - i - 1, line2, width() * depth());
                tm -> writePixelData(0, height() - i - 1, width() - 1, height() - i - 1, line1, width() * depth());
                tm -> writePixelData(0, i, width() - 1, i, line2, width() * depth());
        }

        setTiles(tm);

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
        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), width(), height());
        int cutoff = static_cast<int>(width()/2);

        for(int i = 0; i < height(); i++) {
                tiles() -> readPixelData(0, i, width() - 1, i, line, width() * depth());
                for(int j = 0; j < cutoff; j++) {
                        for(int k = 0; k < depth(); k++) {
                                pixel[k] = line[(width()-1)*depth() - j*depth() + k];
                                line[(width()-1)*depth() - j*depth() + k] = line[j*depth()+k];
                                line[j*depth()+k] = pixel[k];
                        }
                }
                tm -> writePixelData(0, i, width() - 1, i, line, width() * depth());
        }

        setTiles(tm); // Act like this is a resize; this should get it's own undo 'name'

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

        KisTileMgrSP old = tiles();
        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), x + old -> width(), y + old -> height());
        KisPixelDataSP dst;
        KisPixelDataSP src;

        setTiles(tm);
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
        KisTileMgrSP tm = tiles();
        Q_INT32 totalBytes = height() * width() * depth() * sizeof(QUANTUM);
        Q_INT32 nbytes = 0;

        Q_ASSERT(store);
        Q_ASSERT(tm);

        for (Q_INT32 y = 0; y < height(); y += TILE_HEIGHT) {
                for (Q_INT32 x = 0; x < width(); x += TILE_WIDTH) {
                        KisTileSP tile = tm -> tile(x, y, TILEMODE_READ);
                        Q_INT32 tileBytes = tile -> height() * tile -> width() * depth() * sizeof(QUANTUM);

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
        KisTileMgrSP tm = tiles();
        Q_INT32 totalBytes = height() * width() * depth() * sizeof(QUANTUM);
        Q_INT32 nbytes = 0;

        Q_ASSERT(store);
        Q_ASSERT(tm);

        for (Q_INT32 y = 0; y < height(); y += TILE_HEIGHT) {
                for (Q_INT32 x = 0; x < width(); x += TILE_WIDTH) {
                        KisTileSP tile = tm -> tile(x, y, TILEMODE_WRITE);
                        Q_INT32 tileBytes = tile -> height() * tile -> width() * depth() * sizeof(QUANTUM);

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

void KisPaintDevice::convertTo(KisStrategyColorSpaceSP dstCS)
{
	KisPaintDevice dst(width(), height(), dstCS, "");
	KisStrategyColorSpaceSP srcCS = colorStrategy();
	if(srcCS == dstCS)
	{
		return;
	}
	KisIteratorLinePixel dstLIt = dst.iteratorPixelBegin(0);
	KisIteratorLinePixel endLIt = dst.iteratorPixelEnd(0);
	KisIteratorLinePixel srcLIt = this->iteratorPixelBegin(0);
	while( dstLIt <= endLIt)
	{
		KisIteratorPixel dstIt = dstLIt.begin();
		KisIteratorPixel endIt = endLIt.begin();
		KisIteratorPixel srcIt = srcLIt.begin();
		while(dstIt <= endIt )
		{
			KisPixelRepresentation srcPr = srcIt;
			KisPixelRepresentation dstPr = dstIt;
			srcCS->convertTo(srcPr, dstPr, dstCS);
			++dstIt; ++srcIt;
		}
		++dstLIt; ++srcLIt;
	}
	setTiles(dst.tiles());
}



void KisPaintDevice::convertFromImage(const QImage& img)
{
	KoColor c;
	QRgb rgb;
	Q_INT32 opacity;

	if (img.isNull())
		return;

	for (Q_INT32 y = 0; y < height(); y++) {
		for (Q_INT32 x = 0; x < width(); x++) {
			rgb = img.pixel(x, y);
			c.setRGB(upscale(qRed(rgb)), upscale(qGreen(rgb)), upscale(qBlue(rgb)));

			if (img.hasAlphaBuffer())
				opacity = qAlpha(rgb);
			else
				opacity = OPACITY_OPAQUE;

			setPixel(x, y, c, opacity);
		}
	}
}

QImage KisPaintDevice::convertToImage(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	if (w < 0) {
		w = width();
	}

	if (h < 0) {
		h = height();
	}

	if (x < 0) {
		x = 0;
	}
	else
	if (x > width() - 1) {
		x = width() - 1;
	}

	if (y < 0) {
		y = 0;
	}
	else
	if (y > height() - 1) {
		y = height() - 1;
	}

	Q_INT32 x1 = x;
	Q_INT32 y1 = y;

	// These coordinates are inclusive.
	Q_INT32 x2 = x1 + w - 1;
	Q_INT32 y2 = y1 + h - 1;

	if (x2 > width() - 1) {
		x2 = width() - 1;
	}

	if (y2 > height() - 1) {
		y2 = height() - 1;
	}

	QImage image;

	if (x2 - x1 + 1 > 0 && y2 - y1 + 1 > 0 && tiles()) {
		KisPixelDataSP pd = new KisPixelData;

		pd -> mgr = 0;
		pd -> tile = 0;
		pd -> mode = TILEMODE_READ;
		pd -> x1 = x1;
		pd -> x2 = x2;
		pd -> y1 = y1;
		pd -> y2 = y2;
		pd -> width = pd -> x2 - pd -> x1 + 1;
		pd -> height = pd -> y2 - pd -> y1 + 1;
		pd -> depth = depth();
		pd -> stride = pd -> depth * pd -> width;

		// XXX: The previous code used a statically allocated buffer
		// of size RENDER_WIDTH * RENDER_HEIGHT * depth. We could do
		// this too if profiling shows this is too slow...
		pd -> owner = true;
		pd -> data = new QUANTUM[pd -> depth * pd -> width * pd -> height];
		tiles() -> readPixelData(pd);

		image = colorStrategy() -> convertToImage(pd -> data, pd -> width, pd -> height, pd -> stride);
	}

	return image;
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumBegin(KisTileCommand* command)
{
        return KisIteratorLineQuantum( this, command, 0);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumBegin(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 ystart)
{
        return KisIteratorLineQuantum( this, command, ystart, xstart, xend);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumEnd(KisTileCommand* command)
{
        return KisIteratorLineQuantum( this, command, height() - 1);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumEnd(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 yend)
{
        return KisIteratorLineQuantum( this, command, yend, xstart, xend);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionBegin(KisTileCommand* command)
{
        return KisIteratorLineQuantum( this, command, 0);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionBegin(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 ystart)
{
        return KisIteratorLineQuantum( this, command, ystart,  xstart, xend);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionEnd(KisTileCommand* command)
{
        return KisIteratorLineQuantum( this, command, height() - 1);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionEnd(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 yend)
{
        return KisIteratorLineQuantum( this, command, yend, xstart, xend);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelBegin(KisTileCommand* command)
{
        return KisIteratorLinePixel( this, command, 0);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelBegin(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 ystart)
{
        return KisIteratorLinePixel( this, command, ystart, xstart, xend);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelEnd(KisTileCommand* command)
{
        return KisIteratorLinePixel( this, command, height() - 1);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelEnd(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 yend)
{
        return KisIteratorLinePixel( this, command, yend, xstart, xend);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelSelectionBegin(KisTileCommand* command)
{
        return KisIteratorLinePixel( this, command, 0);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelSelectionBegin(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 ystart)
{
        return KisIteratorLinePixel( this, command, ystart,  xstart, xend);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelSelectionEnd(KisTileCommand* command)
{
        return KisIteratorLinePixel( this, command, height() - 1);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelSelectionEnd(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 yend)
{
        return KisIteratorLinePixel( this, command, yend, xstart, xend);
}


#include "kis_paint_device.moc"

