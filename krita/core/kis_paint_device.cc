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

#include <qapplication.h>
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
#include "kis_canvas_controller.h"


namespace {

class MoveCommand : public KNamedCommand {
	typedef KNamedCommand super;

public:
	MoveCommand(KisCanvasControllerInterface * controller,  KisPaintDeviceSP device, const QPoint& oldpos, const QPoint& newpos);
	virtual ~MoveCommand();

	virtual void execute();
	virtual void unexecute();

private:
	void moveTo(const QPoint& pos);

private:
	KisPaintDeviceSP m_device;
	KisCanvasControllerInterface * m_controller;
	QPoint m_oldPos;
	QPoint m_newPos;
};

MoveCommand::MoveCommand(KisCanvasControllerInterface * controller, KisPaintDeviceSP device, const QPoint& oldpos, const QPoint& newpos) :
	super(i18n("Moved layer"))
{
	m_controller = controller;
	m_device = device;
	m_oldPos = oldpos;
	m_newPos = newpos;
}

MoveCommand::~MoveCommand()
{
}

void MoveCommand::execute()
{
	moveTo(m_newPos);
}

void MoveCommand::unexecute()
{
	moveTo(m_oldPos);
}

void MoveCommand::moveTo(const QPoint& pos)
{
	m_device -> move(pos.x(), pos.y());
	m_controller -> updateCanvas();
}

}

KisPaintDevice::KisPaintDevice(KisStrategyColorSpaceSP colorStrategy, const QString& name) :
	KShared()
{
	Q_ASSERT(colorStrategy != 0);
	Q_ASSERT(name.isEmpty() == false);
	if (name.isEmpty()) kdDebug() << "Empty name";
	m_x = 0;
	m_y = 0;

	m_pixelSize = colorStrategy -> pixelSize();
	m_nChannels = colorStrategy -> nChannels();

	m_datamanager = new KisDataManager(m_pixelSize);

	m_visible = true;
	m_owner = 0;
	m_name = name;

	m_compositeOp = COMPOSITE_OVER;

	m_colorStrategy = colorStrategy;

	m_hasSelection = false;
	m_selection = 0;
	m_profile = 0;
}

KisPaintDevice::KisPaintDevice(KisImage *img, KisStrategyColorSpaceSP colorStrategy, const QString& name) :
	KShared()
{

	Q_ASSERT(img != 0);
	Q_ASSERT(colorStrategy != 0);
	Q_ASSERT(name.isEmpty() == false);

        m_x = 0;
        m_y = 0;
	
	m_visible = true;

	m_name = name;
	m_compositeOp = COMPOSITE_OVER;
	m_hasSelection = false;
	m_selection = 0;
	m_profile = 0;

	m_owner = img;

	if (img != 0 && colorStrategy == 0) {
		m_colorStrategy = img -> colorStrategy();
	}
	else {
		m_colorStrategy = colorStrategy;
	}

	if (img != 0 && m_colorStrategy == img -> colorStrategy()) {
			m_profile = m_owner -> profile();
	}

	m_pixelSize = m_colorStrategy -> pixelSize();
	m_nChannels = m_colorStrategy -> nChannels();

	m_datamanager = new KisDataManager(m_pixelSize);

}

KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs) : QObject(), KShared(rhs)
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
		m_pixelSize = rhs.m_pixelSize;
		m_nChannels = rhs.m_nChannels;
        }
}

KisPaintDevice::~KisPaintDevice()
{
	delete m_datamanager;
	m_datamanager = 0;
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

KNamedCommand * KisPaintDevice::moveCommand(KisCanvasControllerInterface * c, Q_INT32 x, Q_INT32 y)
{
	KNamedCommand * cmd = new MoveCommand(c, this, QPoint(m_x, m_y), QPoint(x, y));
	cmd -> execute();
	return cmd;
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


void KisPaintDevice::extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const
{
	m_datamanager -> extent(x, y, w, h);
	x += m_x;
	y += m_y;
}

QRect KisPaintDevice::extent() const
{
	Q_INT32 x, y, w, h;
	extent(x, y, w, h);
	return QRect(x, y, w, h);
}


void KisPaintDevice::exactBounds(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h)
{
	QRect r = exactBounds();
	x = r.x();
	y = r.y();
	w = r.width();
	h = r.height();
}

QRect KisPaintDevice::exactBounds()
{
	Q_INT32 x, y, w, h, boundX, boundY, boundW, boundH;
	extent(x, y, w, h);

	kdDebug() << "Extent: " << x << ", " << y << ", " << w << ", " << h << "\n";
	extent(boundX, boundY, boundW, boundH);
	Q_UINT8 * emptyPixel = new Q_UINT8(m_pixelSize);
	memset(emptyPixel, 0, m_pixelSize);

	bool found = false;

	for (Q_INT32 y2 = y; y < h ; ++y2) {
		KisHLineIterator it = createHLineIterator(x, y2, w, false);
		while (!it.isDone() && found == false) {
			if (memcmp(it.rawData(), emptyPixel, m_pixelSize) != 0) {
				boundY = y2;
				found = true;
				break;
			}
			++it;
		}
		if (found) break;

	}

	found = false;
	
	for (Q_INT32 y2 = h; y2 > y ; --y2) {
		KisHLineIterator it = createHLineIterator(x, y2, w, false);
		while (!it.isDone() && found == false) {
			if (memcmp(it.rawData(), emptyPixel, m_pixelSize) != 0) {
				boundH = y2 + 1;
				found = true;
				break;
			}
			++it;
		}
		if (found) break;
	}
	found = false;
	
	for (Q_INT32 x2 = x; x2 < w ; ++x2) {
		KisVLineIterator it = createVLineIterator(x2, y, h, false);
		while (!it.isDone() && found == false) {
			if (memcmp(it.rawData(), emptyPixel, m_pixelSize) != 0) {
				boundX = x2;
				found = true;
				break;
			}
			++it;
		}
		if (found) break;
	}

	found = false;

	// Loog for right edge )
	for (Q_INT32 x2 = w; x2 > x ; --x2) {
		KisVLineIterator it = createVLineIterator(x2, y, h, false);
		while (!it.isDone() && found == false) {
			if (memcmp(it.rawData(), emptyPixel, m_pixelSize) != 0) {
				boundW = x2 + 1;
				found = true;
				break;
			}
			++it;
		}
		if (found) break;
	}
		
	delete emptyPixel;
	kdDebug() << "Bounds: " << boundX << ", " << boundY << ", " << boundW << ", " << boundH << "\n";
	return QRect(boundX, boundY, boundW, boundH);
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

void KisPaintDevice::mirrorX()
{
        /* Read a line and write it backwards into a temporary buffer. */
	Q_INT32 x, y, rx, ry, rw, rh, le;
	exactBounds(rx, ry, rw, rh);

	le = rw * m_pixelSize - m_pixelSize;
	// We need this tmpLine until we can use decrement iterators
	Q_UINT8 * tmpLine = new Q_UINT8[(rw * m_pixelSize)];
	
	for (y = ry; y < rh; ++y) {
		x = le;

		KisHLineIterator it = createHLineIterator(rx, y, rw, false);
		while (!it.isDone()) {
			memcpy(tmpLine + x, it.rawData(), m_pixelSize);
			x = x - m_pixelSize;
			++it;
		}
		writeBytes(tmpLine, rx, y, rw, 1);
		qApp -> processEvents();
	}
	delete []tmpLine;
}

void KisPaintDevice::mirrorY()
{
	/* Read a line from bottom to top and and from top to bottom and write their values to each other */
	Q_INT32 rx, ry, rw, rh;
	exactBounds(rx, ry, rw, rh);
		
	Q_INT32 y1, y2;
	for (y1 = 0, y2 = rh; y1 < rh / 2 || y2 > rh / 2; ++y1, --y2) {
		KisHLineIterator itTop = createHLineIterator(rx, y1, rw, true);
		KisHLineIterator itBottom = createHLineIterator(rx, y2, rw, true);
		while (!itTop.isDone() && !itBottom.isDone()) {
			memcpy(itBottom.rawData(), itTop.oldRawData(), m_pixelSize);
			memcpy(itTop.rawData(), itBottom.oldRawData(), m_pixelSize);
			++itBottom;
			++itTop;
		}
		qApp -> processEvents();
	}
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

	Q_INT32 x, y, w, h;
	extent(x, y, w, h);


	KisPaintDevice dst(dstColorStrategy, name());
	dst.setProfile(dstProfile);
	
	if(colorStrategy() == dstColorStrategy)
	{
		return;
	}

	for (Q_INT32 y2 = y; y2 < h; ++y2) {
		KisHLineIteratorPixel srcIt = createHLineIterator(x, y2, w, false);
		KisHLineIteratorPixel dstIt = createHLineIterator(x, y2, w, true);
		while (!srcIt.isDone()) {
// 			m_colorStrategy -> convertTo(srcIt.pixel(), dstIt.pixel(), renderingIntent)
			++srcIt;
			++dstIt;
		}
	}
	delete m_datamanager;
	m_datamanager = dst.m_datamanager;
	m_colorStrategy = dstColorStrategy;
	m_profile = dstProfile;


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


void KisPaintDevice::setProfile(KisProfileSP profile)
{
	if (profile && profile -> colorSpaceSignature() == colorStrategy() -> colorSpaceSignature() && profile -> valid()) {
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

	if (image()) {
		w = image()->width();
		h = image()->height();
	}
	else {
		extent(x1, y1, w, h);
	}

	return convertToQImage(dstProfile, x1, y1, w, h);
}

QImage KisPaintDevice::convertToQImage(KisProfileSP dstProfile, Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h)
{
	if (w < 0)
		w = 0;

	if (h < 0)
		h = 0;

	QUANTUM * data = m_datamanager -> readBytes(x1, y1, w, h);
//  	kdDebug() << m_name << ": convertToQImage. My profile: " << m_profile << ", destination profile: " << dstProfile << "\n";
	QImage image = colorStrategy() -> convertToQImage(data, w, h, m_profile, dstProfile);
	delete[] data;

	return image;
}

KisRectIteratorPixel KisPaintDevice::createRectIterator(Q_INT32 left, Q_INT32 top, Q_INT32 w, Q_INT32 h, bool writable)
{
	if(hasSelection())
		return KisRectIteratorPixel(this, m_datamanager, m_selection->m_datamanager, left, top, w, h, m_x, m_y, writable);
	else
		return KisRectIteratorPixel(this, m_datamanager, NULL, left, top, w, h, m_x, m_y, writable);
}

KisHLineIteratorPixel  KisPaintDevice::createHLineIterator(Q_INT32 x, Q_INT32 y, Q_INT32 w, bool writable)
{
	if(hasSelection())
		return KisHLineIteratorPixel(this, m_datamanager, m_selection->m_datamanager, x, y, w, m_x, m_y, writable);
	else
		return KisHLineIteratorPixel(this, m_datamanager, NULL, x, y, w, m_x, m_y, writable);
}

KisVLineIteratorPixel  KisPaintDevice::createVLineIterator(Q_INT32 x, Q_INT32 y, Q_INT32 h, bool writable)
{
	if(hasSelection())
		return KisVLineIteratorPixel(this, m_datamanager, m_selection->m_datamanager, x, y, h, m_x, m_y, writable);
	else
		return KisVLineIteratorPixel(this, m_datamanager, NULL, x, y, h, m_x, m_y, writable);
	
}

KisSelectionSP KisPaintDevice::selection(){
	if (!m_hasSelection) {
		m_selection = new KisSelection(this, "layer selection for: " + name());
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

  Q_UINT8 *pix = iter.rawData();

  if (!pix) return false;

  colorStrategy() -> toQColor(pix, c, opacity, m_profile);

  return true;
}

bool KisPaintDevice::setPixel(Q_INT32 x, Q_INT32 y, const QColor& c, QUANTUM opacity)
{
  KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);

  colorStrategy() -> nativeColor(c, opacity, iter.rawData(), m_profile);

  return true;
}



#include "kis_paint_device.moc"
