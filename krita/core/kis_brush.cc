/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <netinet/in.h>
#include <limits.h>
#include <stdlib.h>

#include <qimage.h>
#include <qpoint.h>
#include <qvaluevector.h>

#include <kdebug.h>

#include "kis_global.h"
#include "kis_brush.h"
#include "kis_alpha_mask.h"

namespace {
	struct GimpBrushHeader {
		Q_UINT32 header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
		Q_UINT32 version;      /*  brush file version #  */
		Q_UINT32 width;        /*  width of brush  */
		Q_UINT32 height;       /*  height of brush  */
		Q_UINT32 bytes;        /*  depth of brush in bytes */
		Q_UINT32 magic_number; /*  GIMP brush magic number  */
		Q_UINT32 spacing;      /*  brush spacing  */
	};
}

KisBrush::KisBrush(const QString& filename) : super(filename)
{
	m_brushType = INVALID;
}

KisBrush::KisBrush(const QString& filename,
		   const QValueVector<Q_UINT8> & data,
		   Q_UINT32 & dataPos) : super(filename)
{
	m_brushType = INVALID;

	m_data.clear();
	// XXX: This can be done more efficiently, I guess.
	for (Q_UINT32 i = dataPos; i < data.size(); i++) {
		m_data.append(data[i]);
	}
	ioResult(0);
	dataPos += m_header_size + (m_width * m_height * m_bytes);
}


KisBrush::~KisBrush()
{
	m_masks.setAutoDelete(true);
	m_masks.clear();
}

bool KisBrush::loadAsync()
{
	KIO::Job *job = KIO::get(filename(), false, false);

	connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(ioData(KIO::Job*, const QByteArray&)));
	connect(job, SIGNAL(result(KIO::Job*)), SLOT(ioResult(KIO::Job*)));
	return true;
}

bool KisBrush::saveAsync()
{
	return false;
}

QImage KisBrush::img()
{
	return m_img;
}

KisAlphaMask *KisBrush::mask(Q_INT32 scale)
{
	if (m_masks.isEmpty()) {
		createMasks(m_img);
	}

	if (scale >= PRESSURE_LEVELS || (uint)scale >= m_masks.count())
		scale = m_masks.count() - 1;

	if (scale < 0) scale = 0;

	return m_masks.at(scale);
}

void KisBrush::setHotSpot(QPoint pt)
{
	Q_INT32 x = pt.x();
	Q_INT32 y = pt.y();

	if (x < 0)
		x = 0;
	else if (x >= width())
		x = width() - 1;

	if (y < 0)
		y = 0;
	else if (y >= height())
		y = height() - 1;

	m_hotSpot = QPoint(x, y);
}

enumBrushType KisBrush::brushType() const {
	return m_brushType;
}

void KisBrush::ioData(KIO::Job * /*job*/, const QByteArray& data)
{
	if (!data.isEmpty()) {
		Q_INT32 startPos = m_data.size();

		m_data.resize(m_data.size() + data.count());
		memcpy(&m_data[startPos], data.data(), data.count());
	}
}

void KisBrush::ioResult(KIO::Job * /*job*/)
{
	GimpBrushHeader bh;
	Q_INT32 k;
	QValueVector<char> name;

	if (sizeof(GimpBrushHeader) > m_data.size()) {
		emit ioFailed(this);
		return;
	}

	memcpy(&bh, &m_data[0], sizeof(GimpBrushHeader));
	bh.header_size = ntohl(bh.header_size);
	m_header_size = bh.header_size;

	bh.version = ntohl(bh.version);
	m_version = bh.version;

	bh.width = ntohl(bh.width);
	m_width = bh.width;

	bh.height = ntohl(bh.height);
	m_height = bh.height;

	bh.bytes = ntohl(bh.bytes);
	m_bytes = bh.bytes;

	bh.magic_number = ntohl(bh.magic_number);
	m_magic_number = bh.magic_number;

	bh.spacing = ntohl(bh.spacing);

	if (bh.header_size > m_data.size() || bh.header_size == 0) {
		emit ioFailed(this);
		return;
	}

        if (bh.spacing > m_width || bh.spacing > m_height) {
                setSpacing(m_width);
        }
        else {
                setSpacing(bh.spacing / 10);
        }

	name.resize(bh.header_size - sizeof(GimpBrushHeader));
	memcpy(&name[0], &m_data[sizeof(GimpBrushHeader)], name.size());

	if (name[name.size() - 1]) {
		emit ioFailed(this);
		return;
	}

	setName(&name[0]);

	if (bh.width == 0 || bh.height == 0 || !m_img.create(bh.width, bh.height, 32)) {
		emit ioFailed(this);
		return;
	}



	k = bh.header_size;

	if (bh.bytes == 1) {
		// Grayscale
		m_brushType = MASK;

		Q_INT32 val;
		for (Q_UINT32 y = 0; y < bh.height; y++) {
			for (Q_UINT32 x = 0; x < bh.width; x++, k++) {
				if (static_cast<Q_UINT32>(k) > m_data.size()) {
					emit ioFailed(this);
					return;
				}

				val = 255 - m_data[k];
				m_img.setPixel(x, y, qRgb(val, val, val));
			}
		}
	} else if (bh.bytes == 4) {
		// Has alpha
		m_brushType = IMAGE;

		for (Q_UINT32 y = 0; y < bh.height; y++) {
			for (Q_UINT32 x = 0; x < bh.width; x++) {
				if (static_cast<Q_UINT32>(k + 4) > m_data.size()) {
					emit ioFailed(this);
					return;
				}
				m_img.setPixel(x, y, qRgba(255 - m_data[k++],
							   255 - m_data[k++],
							   255 - m_data[k++],
							   255 - m_data[k++]));
			}
		}
	} else {
		emit ioFailed(this);
		return;
	}

	setWidth(m_img.width());
	setHeight(m_img.height());
	//createMasks(m_img);
	m_data.clear(); // Save some memory, we're using enough of it as it is.
 	kdDebug() << "Brush: " << &name[0] << " spacing: " << spacing() << "\n";
	setValid(true);
	emit loadComplete(this);
}


void KisBrush::createMasks(const QImage & img) {
	if (!m_masks.isEmpty())
		m_masks.clear();

	double scale = 0.0;
	for (Q_INT32 i = 0; i < PRESSURE_LEVELS; i++) {
		// 100 levels, 50 smaller, 50 bigger, smallest image is 0%, biggest 200$, value 50
		// is 100%, so 0-50 describes 0-100%, compute x & y: 50 becomes 1, 0 becomes 0.01,
		// every step is 2%
		scale += 0.02;
		m_masks.append(new KisAlphaMask(img, scale));
	}
}

#include "kis_brush.moc"

