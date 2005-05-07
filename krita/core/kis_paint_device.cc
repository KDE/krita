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
#include "kis_transform_visitor.h"
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
		super(i18n("Moved Layer"))
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

	class KisConvertLayerTypeCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		KisConvertLayerTypeCmd(KisUndoAdapter *adapter, KisPaintDeviceSP paintDevice, 
				       KisDataManagerSP beforeData, KisStrategyColorSpaceSP beforeColorSpace, KisProfileSP beforeProfile, 
				       KisDataManagerSP afterData, KisStrategyColorSpaceSP afterColorSpace, KisProfileSP afterProfile
				       ) : super(i18n("&Convert Layer Type...")) //XXX: fix when string freeze over
			{
				m_adapter = adapter;
				m_paintDevice = paintDevice;
				m_beforeData = beforeData;
				m_beforeColorSpace = beforeColorSpace;
				m_beforeProfile = beforeProfile;
				m_afterData = afterData;
				m_afterColorSpace = afterColorSpace;
				m_afterProfile = afterProfile;
			}

		virtual ~KisConvertLayerTypeCmd()
			{
			}

	public:
		virtual void execute()
			{
				m_adapter -> setUndo(false);

				m_paintDevice -> setData(m_afterData, m_afterColorSpace, m_afterProfile);

				m_adapter -> setUndo(true);
				if (m_paintDevice -> image()) {
					m_paintDevice -> image() -> notify();
					m_paintDevice -> image() -> notifyLayersChanged();
				}
			}

		virtual void unexecute()
			{
				m_adapter -> setUndo(false);

				m_paintDevice -> setData(m_beforeData, m_beforeColorSpace, m_beforeProfile);

				m_adapter -> setUndo(true);
				if (m_paintDevice -> image()) {
					m_paintDevice -> image() -> notify();
					m_paintDevice -> image() -> notifyLayersChanged();
				}
			}

	private:
		KisUndoAdapter *m_adapter;

		KisPaintDeviceSP m_paintDevice;

		KisDataManagerSP m_beforeData;
		KisStrategyColorSpaceSP m_beforeColorSpace;
		KisProfileSP m_beforeProfile;

		KisDataManagerSP m_afterData;
		KisStrategyColorSpaceSP m_afterColorSpace;
		KisProfileSP m_afterProfile;
	};

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

	Q_UINT8 defPixel[6] = {0,0,0,0,0,0};//XXX should be moved to colorstrategy	
	m_datamanager = new KisDataManager(m_pixelSize, defPixel);
	Q_CHECK_PTR(m_datamanager);

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

	Q_UINT8 defPixel[6] = {0,0,0,0,0,0};//XXX should be moved to colorstrategy	
	m_datamanager = new KisDataManager(m_pixelSize, defPixel);
	Q_CHECK_PTR(m_datamanager);

}

KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs) : QObject(), KShared(rhs)
{
        if (this != &rhs) {
                m_owner = 0;

                if (rhs.m_datamanager) {
			m_datamanager = new KisDataManager(*rhs.m_datamanager);
			Q_CHECK_PTR(m_datamanager);
		}
                m_visible = rhs.m_visible;
                m_x = rhs.m_x;
                m_y = rhs.m_y;
                m_name = rhs.m_name;
                m_compositeOp = rhs.m_compositeOp;
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
	Q_CHECK_PTR(cmd);
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
	Q_UINT8 * emptyPixel = new Q_UINT8[m_pixelSize];
	Q_CHECK_PTR(emptyPixel);

	memset(emptyPixel, 0, m_pixelSize);

	bool found = false;

	for (Q_INT32 y2 = y; y2 < y + h ; ++y2) {
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
	
	for (Q_INT32 y2 = y + h; y2 > y ; --y2) {
		KisHLineIterator it = createHLineIterator(x, y2, w, false);
		while (!it.isDone() && found == false) {
			if (memcmp(it.rawData(), emptyPixel, m_pixelSize) != 0) {
				boundH = y2 - boundY + 1;
				found = true;
				break;
			}
			++it;
		}
		if (found) break;
	}
	found = false;
	
	for (Q_INT32 x2 = x; x2 < x + w ; ++x2) {
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
	for (Q_INT32 x2 = x + w; x2 > x ; --x2) {
		KisVLineIterator it = createVLineIterator(x2, y, h, false);
		while (!it.isDone() && found == false) {
			if (memcmp(it.rawData(), emptyPixel, m_pixelSize) != 0) {
				boundW = x2 - boundX + 1;
				found = true;
				break;
			}
			++it;
		}
		if (found) break;
	}
		
	delete [] emptyPixel;
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

void KisPaintDevice::accept(KisTransformVisitor& visitor)
{
        visitor.visitKisPaintDevice(this);
}


void KisPaintDevice::scale(double xscale, double yscale, KisProgressDisplayInterface * progress, enumFilterType ftype)
{
        KisScaleVisitor visitor;
        accept(visitor);
        visitor.scale(xscale, yscale, progress, ftype);
}

void KisPaintDevice::rotate(double angle, KisProgressDisplayInterface * progress)
{
        KisRotateVisitor visitor;
        accept(visitor);
        visitor.rotate(angle, progress);
}

void KisPaintDevice::shear(double angleX, double angleY, KisProgressDisplayInterface * progress)
{
        KisRotateVisitor visitor;
        accept(visitor);
        visitor.shear(angleX, angleY, progress);
}

void KisPaintDevice::transform(Q_INT32  xscale, Q_INT32  yscale, 
			Q_INT32  xshear, Q_INT32  yshear, Q_INT32  denominator,
			Q_INT32  xtranslate, Q_INT32  ytranslate, KisProgressDisplayInterface *progress)
{
        KisTransformVisitor visitor;
        accept(visitor);
        visitor.transform(xscale, yscale, xshear,  yshear, denominator, xtranslate, ytranslate, progress);
}

void KisPaintDevice::mirrorX()
{
        /* Read a line and write it backwards into a temporary buffer. */
	Q_INT32 x, y, rx, ry, rw, rh, le;
	exactBounds(rx, ry, rw, rh);

	le = rw * m_pixelSize - m_pixelSize;
	// We need this tmpLine until we can use decrement iterators
	Q_UINT8 * tmpLine = new Q_UINT8[(rw * m_pixelSize)];
	Q_CHECK_PTR(tmpLine);
	
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
	if (colorStrategy() -> id() == dstColorStrategy -> id())
	{
		return;
	}

	KisPaintDevice dst(dstColorStrategy, name());
	dst.setProfile(dstProfile);
	dst.setX(getX());
	dst.setY(getY());

	Q_INT32 x, y, w, h;
	extent(x, y, w, h);

	for (Q_INT32 row = y; row < y + h; ++row) {

		Q_INT32 column = x;
		Q_INT32 columnsRemaining = w;

		while (columnsRemaining > 0) {

			Q_INT32 numContiguousDstColumns = dst.numContiguousColumns(column, row, row);
			Q_INT32 numContiguousSrcColumns = numContiguousColumns(column, row, row);

			Q_INT32 columns = QMIN(numContiguousDstColumns, numContiguousSrcColumns); 
			columns = QMIN(columns, columnsRemaining);

			const Q_UINT8 *srcData = pixel(column, row);
			Q_UINT8 *dstData = dst.writablePixel(column, row);

			m_colorStrategy -> convertPixelsTo(srcData, m_profile, dstData, dstColorStrategy, dstProfile, columns, renderingIntent);

			column += columns;
			columnsRemaining -= columns;
		}
	}

	if (undoAdapter() && undoAdapter() -> undo()) {
		undoAdapter() -> addCommand(new KisConvertLayerTypeCmd(undoAdapter(), this, m_datamanager, m_colorStrategy, m_profile,
								       dst.m_datamanager, dstColorStrategy, dstProfile));
	}

	setData(dst.m_datamanager, dstColorStrategy, dstProfile);
}

void KisPaintDevice::setData(KisDataManagerSP data, KisStrategyColorSpaceSP colorStrategy, KisProfileSP profile)
{
	m_datamanager = data;
	m_colorStrategy = colorStrategy;
	m_pixelSize = m_colorStrategy -> pixelSize();
	m_nChannels = m_colorStrategy -> nChannels();
	m_profile = profile;
}

KisUndoAdapter *KisPaintDevice::undoAdapter() const
{
	if (m_owner) {
		return m_owner -> undoAdapter();
	}
	return 0;
}

void KisPaintDevice::convertFromImage(const QImage& img)
{
	// XXX: Apply import profile

	// XXX: Optimize this.
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

	QUANTUM * data = new QUANTUM[w * h * m_pixelSize];
	Q_CHECK_PTR(data);

	m_datamanager -> readBytes(data, x1, y1, w, h);
//  	/*kdDebug*/() << m_name << ": convertToQImage. My profile: " << m_profile << ", destination profile: " << dstProfile << "\n";
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
		Q_CHECK_PTR(m_selection);
		m_selection -> setVisible(true);
		m_hasSelection = true;
		if(m_owner)
			m_owner -> slotSelectionCreated();
	}
	return m_selection;

}

void KisPaintDevice::setSelection(KisSelectionSP selection)
{
	m_selection = selection;
	m_hasSelection = true;
	if(m_owner)
		m_owner -> slotSelectionChanged();

}


bool KisPaintDevice::hasSelection()
{
	return m_hasSelection;
}


void KisPaintDevice::removeSelection()
{
	m_selection = 0; // XXX: Does this automatically remove the selection due to the shared pointer?
	m_hasSelection = false;
	if(m_owner)
		m_owner -> slotSelectionChanged();
}

void KisPaintDevice::addSelection(KisSelectionSP selection) {
	if (!m_hasSelection) {
		setSelection(selection);
		return;
	}
	
	KisPainter painter(m_selection.data());
	Q_INT32 x, y, w, h;
	selection -> extent(x, y, w, h);
	painter.bitBlt(x, y, COMPOSITE_OVER, selection.data(), x, y, w, h);
	painter.end();
	
	if(m_owner)
		m_owner -> slotSelectionChanged();
}

void KisPaintDevice::subtractSelection(KisSelectionSP selection) {
	if (!m_hasSelection)
		return;

	Q_INT32 x, y, w, h;
	selection -> extent(x, y, w, h);
	KisPainter painter(m_selection.data());
	painter.bitBlt(x, y, COMPOSITE_ERASE, selection.data(), x, y, x, y);
	painter.end();

	if(m_owner)
		m_owner -> slotSelectionChanged();
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

Q_INT32 KisPaintDevice::numContiguousColumns(Q_INT32 x, Q_INT32 minY, Q_INT32 maxY)
{
	return m_datamanager -> numContiguousColumns(x - m_x, minY - m_y, maxY - m_y);
}

Q_INT32 KisPaintDevice::numContiguousRows(Q_INT32 y, Q_INT32 minX, Q_INT32 maxX)
{
	return m_datamanager -> numContiguousRows(y - m_y, minX - m_x, maxX - m_x);
}

Q_INT32 KisPaintDevice::rowStride(Q_INT32 x, Q_INT32 y)
{
	return m_datamanager -> rowStride(x - m_x, y - m_y);
}

const Q_UINT8* KisPaintDevice::pixel(Q_INT32 x, Q_INT32 y)
{
	return m_datamanager -> pixel(x - m_x, y - m_y);
}

Q_UINT8* KisPaintDevice::writablePixel(Q_INT32 x, Q_INT32 y)
{
	return m_datamanager -> writablePixel(x - m_x, y - m_y);
}


#include "kis_paint_device.moc"
