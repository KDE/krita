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
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_undo_adapter.h"
#include "tiles/kis_iterator.h"
#include "kis_iterators_pixel.h"
#include "kis_scale_visitor.h"
#include "kis_rotate_visitor.h"
#include "kis_profile.h"


KisPaintDevice::KisPaintDevice(KisStrategyColorSpaceSP colorStrategy, const QString& name)
{
	init();
	m_x = 0;
	m_y = 0;
	m_datamanager = new KisDataManager(colorStrategy -> depth());
	m_visible = true;
	m_owner = 0;
	m_name = name;
	m_compositeOp = COMPOSITE_OVER;
	m_colorStrategy = colorStrategy;
	m_hasSelection = false;
	m_selection = 0;
	m_profile = 0;
}

KisPaintDevice::KisPaintDevice(KisImage *img, KisStrategyColorSpaceSP colorStrategy, const QString& name)
{
        init();
        configure(img, colorStrategy, name, COMPOSITE_OVER);
	m_profile = 0;
}

KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs) : QObject(), super(rhs)
{
        if (this != &rhs) {
                m_owner = 0;

                if (rhs.m_datamanager)
                        m_datamanager = new KisDataManager(*rhs.m_datamanager);

                m_visible = rhs.m_visible;
                m_x = rhs.m_x;
                m_y = rhs.m_y;
                m_name = rhs.m_name;
                m_compositeOp = COMPOSITE_OVER;
		m_colorStrategy = rhs.m_colorStrategy;
		m_hasSelection = false;
		m_selection = 0;
		m_profile = rhs.m_profile;
        }
}

KisPaintDevice::~KisPaintDevice()
{
}

void KisPaintDevice::configure(KisImage *image,
                               KisStrategyColorSpaceSP colorStrategy,
                               const QString& name,
                               CompositeOp compositeOp)
{
	if (image == 0 || name.isEmpty() || colorStrategy == 0)
		return;

	m_x = 0;
	m_y = 0;
	m_datamanager = new KisDataManager(colorStrategy -> depth());
	m_visible = true;
	m_owner = image;
	m_name = name;
	m_compositeOp = compositeOp;
	m_colorStrategy = colorStrategy;
	m_hasSelection = false;
		m_selection = 0;
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

void KisPaintDevice::init()
{
        m_visible = false;
        m_x = 0;
        m_y = 0;

	m_colorStrategy = 0;
	m_hasSelection = false;
		m_selection = 0;
}

void KisPaintDevice::extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const
{
	m_datamanager -> extent(x, y, w, h);
}

void KisPaintDevice::accept(KisScaleVisitor& visitor)
{
        visitor.visitKisPaintDevice(this);
}

void KisPaintDevice::accept(KisRotateVisitor& visitor)
{
        visitor.visitKisPaintDevice(this);
}


void KisPaintDevice::scale(double xscale, double yscale, KisProgressDisplayInterface *m_progress, enumFilterType ftype)
{
        KisScaleVisitor visitor;
        accept(visitor);
        visitor.scale(xscale, yscale, m_progress, ftype);
}

void KisPaintDevice::rotate(double angle, KisProgressDisplayInterface *m_progress)
{
        KisRotateVisitor visitor;
        accept(visitor);
        visitor.rotate(angle, m_progress);
}

void KisPaintDevice::shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress)
{
        KisRotateVisitor visitor;
        accept(visitor);
        visitor.shear(angleX, angleY, m_progress);
}

// XXX: also allow transform on part of paint device?
void KisPaintDevice::transform(const QWMatrix & matrix)
{
#if 0 //AUTOLAYER
        if (tiles() == 0) {
//                 kdDebug() << "No tilemgr.\n";
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
//                 kdDebug() << "Scaling layer: " << m_name << "\n";
                if ( mat.m11() == 1.0F && mat.m22() == 1.0F ) {
//                         kdDebug() << "Identity matrix, do nothing.\n";
                        return;
                }
                targetW = qRound( mat.m11() * width() );
                targetH = qRound( mat.m22() * height() );
                targetW = QABS( targetW );
                targetH = QABS( targetH );
        } else {
//                 kdDebug() << "Rotating or shearing layer " << m_name << "\n";
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
//                 kdDebug() << "Error, return null image\n";
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
#endif //AUTOLAYER
}

void KisPaintDevice::mirrorX()
{
#if 0 //AUTOLAYER
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
#endif  //AUTOLAYER
}

void KisPaintDevice::mirrorY()
{
#if 0 //AUTOLAYER
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
#endif  //AUTOLAYER
}

bool KisPaintDevice::write(KoStore *store)
{
	bool retval = m_datamanager->write(store);
	emit ioProgress(100);

        return retval;
}

bool KisPaintDevice::read(KoStore *store)
{
	bool retval = m_datamanager->read(store);
	emit ioProgress(100);

        return retval;
}

void KisPaintDevice::convertTo(KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile, Q_INT32 renderingIntent)
{
#if 0 //AUTOLAYER

	// XXX:  Given the way lcms works, we'd better not do
	// this with iterators but with the largest chunks of
	// contiguous bytes we can get.
	KisPaintDevice dst(width(), height(), dstColorStrategy, "");
	dst.setProfile(dstProfile);

	KisStrategyColorSpaceSP srcColorStrategy = colorStrategy();
	if(srcColorStrategy == dstColorStrategy)

//XXX: Not bit depth independent, assumes both src and dst occupies same number of bytes per pixel
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
			KisPixel srcPr = srcIt;
			KisPixel dstPr = dstIt;
			srcColorStrategy -> convertTo(srcPr, dstPr, renderingIntent);
			++dstIt; ++srcIt;
		}
		++dstLIt; ++srcLIt;
	}
	setTiles(dst.tiles());

	m_colorStrategy = dstColorStrategy;
	setProfile(dstProfile);
#endif  //AUTOLAYER
}



void KisPaintDevice::convertFromImage(const QImage& img)
{
	// XXX: Apply import profile
	QColor c;
	QRgb rgb;
	Q_INT32 opacity;

	if (img.isNull())
		return;

	for (Q_INT32 y = 0; y < img.height(); y++) {
		for (Q_INT32 x = 0; x < img.width(); x++) {
			rgb = img.pixel(x, y);
			c.setRgb(upscale(qRed(rgb)), upscale(qGreen(rgb)), upscale(qBlue(rgb)));

			if (img.hasAlphaBuffer())
				opacity = qAlpha(rgb);
			else
				opacity = OPACITY_OPAQUE;

			setPixel(x, y, c, opacity);
		}
	}
}

KisProfileSP KisPaintDevice::profile() const
{
	if (m_profile == 0 && m_owner != 0) {
		if (colorStrategy() == m_owner -> colorStrategy()) {
			return m_owner -> profile();
		}
	}
	return m_profile;
}


void KisPaintDevice::setProfile(KisProfileSP profile)
{
	if (profile -> colorSpaceSignature() == colorStrategy() -> colorSpaceSignature() && profile -> valid()) {
		m_profile = profile;
	}
	else {
		m_profile = 0;
	}
	emit(profileChanged(m_profile));
}

QImage KisPaintDevice::convertToQImage(KisProfileSP dstProfile)
{
	Q_INT32 x1;
	Q_INT32 y1;
	Q_INT32 w;
	Q_INT32 h;

	x1 = - getX();
	y1 = - getY();
	w = image()->width();
	h = image()->height();

	return convertToQImage(dstProfile, x1, y1, w, h);
}

QImage KisPaintDevice::convertToQImage(KisProfileSP dstProfile, Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h)
{
#if 0
	// xxx: Enable this when readBytes finally works
	QImage image;

	if (w < 0)
		w = 0;

	if (h < 0)
		h = 0;

	QUANTUM * data = m_datamanager -> readBytes(x1, y1, w, h);
	image = colorStrategy() -> convertToQImage(data, w, h, m_profile, dstProfile);
	delete data;

	return image;
#else

        QImage image;
        QUANTUM  *data = new QUANTUM[depth() * w * h];
        QUANTUM *ptr=data;

        if (w < 0)
                w = 0;

        if (h < 0)
                h = 0;

        for(Q_INT32 y=y1; y <y1+h;y++)
        {
                KisHLineIterator hiter = createHLineIterator(x1, y, w, false);
                while(! hiter.isDone())
                {
                        memcpy(ptr, (Q_UINT8 *)hiter, depth());

                        ptr += depth();
                        hiter++;
                }
        }

        // XXX: determine whether to apply the monitor profile or based on the copy setting
	image = colorStrategy() -> convertToQImage(data, w, h, m_profile, dstProfile);

        delete data;

        return image;
#endif
}

KisRectIteratorPixel &KisPaintDevice::createRectIterator(Q_INT32 left, Q_INT32 top, Q_INT32 w, Q_INT32 h, bool writable)
{
        return *(new KisRectIteratorPixel(this, m_datamanager, left, top, w, h, writable));
}

KisHLineIteratorPixel  &KisPaintDevice::createHLineIterator(Q_INT32 x, Q_INT32 y, Q_INT32 w, bool writable)
{
        return *(new KisHLineIteratorPixel(this, m_datamanager, x, y, w, writable));
}

KisVLineIteratorPixel  &KisPaintDevice::createVLineIterator(Q_INT32 x, Q_INT32 y, Q_INT32 h, bool writable)
{
        return *(new KisVLineIteratorPixel(this, m_datamanager, x, y, h, writable));
}

KisSelectionSP KisPaintDevice::selection(){
	if (!m_hasSelection) {
		m_selection = new KisSelection(this, "layer selection for: " + name());

		// AUTOLAYER: m_selection -> clear(QRect(0, 0, width(), height()));
		m_selection -> setVisible(true);
		m_hasSelection = true;
		emit selectionCreated();
	}
	return m_selection;

}

void KisPaintDevice::setSelection(KisSelectionSP selection)
{
	m_selection = selection;
	m_hasSelection = true;
	emit selectionChanged();

}

void KisPaintDevice::addSelection(KisSelectionSP /*selection*/)
{
// 	m_selection = m_selection + selection;
	emit selectionChanged();

}

// void KisLayer::subtractSelection(KisSelectionSP /*selection*/)
// {
// // 	m_selection = m_selection - selection;
// 	emit selectionChanged();
//
// }


bool KisPaintDevice::hasSelection()
{
	return m_hasSelection;
}


void KisPaintDevice::removeSelection()
{
	m_selection = 0; // XXX: Does this automatically remove the selection due to the shared pointer?
	m_hasSelection = false;
	emit selectionChanged();
}


bool KisPaintDevice::pixel(Q_INT32 x, Q_INT32 y, QColor *c, QUANTUM *opacity)
{
  KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, false);

  Q_UINT8 *pix = (Q_UINT8 *)iter;
    
  if (!pix) return false;

  colorStrategy() -> toQColor(pix, c, opacity, m_profile);

  return true;
}

bool KisPaintDevice::setPixel(Q_INT32 x, Q_INT32 y, const QColor& c, QUANTUM opacity)
{
  KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);
    
  colorStrategy() -> nativeColor(c, opacity, (QUANTUM*)(iter), m_profile);

  return true;
}

#include "kis_paint_device.moc"
