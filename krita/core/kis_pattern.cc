/*
 *  kis_pattern.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qpoint.h>
#include <qsize.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qvaluevector.h>

#include <kdebug.h>

#include "kis_pattern.h"

#define THUMB_SIZE 30

namespace {
	struct GimpPatternHeader {
		Q_UINT32 header_size;  /*  header_size = sizeof (PatternHeader) + brush name  */
		Q_UINT32 version;      /*  pattern file version #  */
		Q_UINT32 width;        /*  width of pattern */
		Q_UINT32 height;       /*  height of pattern  */
		Q_UINT32 bytes;        /*  depth of pattern in bytes : 1, 2, 3 or 4*/
		Q_UINT32 magic_number; /*  GIMP brush magic number  */
	};
}


KisPattern::KisPattern(const QString& file) : super(file)
{
	m_valid = false;
	m_hotSpot = QPoint(0, 0);
	m_pixmap = 0;
	m_thumbPixmap = 0;
}

KisPattern::~KisPattern()
{
	if (m_pixmap) delete m_pixmap;
	if (m_thumbPixmap) delete m_thumbPixmap;
}


bool KisPattern::loadAsync()
{
	KIO::Job *job = KIO::get(filename(), false, false);

	connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(ioData(KIO::Job*, const QByteArray&)));
	connect(job, SIGNAL(result(KIO::Job*)), SLOT(ioResult(KIO::Job*)));
	return true;
}

bool KisPattern::saveAsync()
{
	return false;
}

QImage KisPattern::img()
{
	return m_img;
}


QPixmap& KisPattern::pixmap() const
{
	return *m_pixmap;
}

QPixmap& KisPattern::thumbPixmap() const
{
	return *m_thumbPixmap;
}


void KisPattern::ioData(KIO::Job * /*job*/, const QByteArray& data)
{
	if (!data.isEmpty()) {
		Q_INT32 startPos = m_data.size();

		m_data.resize(m_data.size() + data.count());
		memcpy(&m_data[startPos], data.data(), data.count());
	}
}

void KisPattern::ioResult(KIO::Job * /*job*/)
{
	// load Gimp patterns
	GimpPatternHeader bh;
	Q_INT32 k;
	QValueVector<char> name;

	if (sizeof(GimpPatternHeader) > m_data.size()) {
		emit ioFailed(this);
		return;
	}

	memcpy(&bh, &m_data[0], sizeof(GimpPatternHeader));
	bh.header_size = ntohl(bh.header_size);
	bh.version = ntohl(bh.version);
	bh.width = ntohl(bh.width);
	bh.height = ntohl(bh.height);
	bh.bytes = ntohl(bh.bytes);
	bh.magic_number = ntohl(bh.magic_number);

	if (bh.header_size > m_data.size() || bh.header_size == 0) {
		emit ioFailed(this);
		return;
	}

	name.resize(bh.header_size - sizeof(GimpPatternHeader));
	memcpy(&name[0], &m_data[sizeof(GimpPatternHeader)], name.size());

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
// 		kdDebug() << "Loading grayscale pattern " << &name[0] << endl;
		// Grayscale
		Q_INT32 val;

		for (Q_UINT32 y = 0; y < bh.height; y++) {
			for (Q_UINT32 x = 0; x < bh.width; x++, k++) {
				if (static_cast<Q_UINT32>(k) > m_data.size()) {
					kdDebug() << "failed in gray\n";
					emit ioFailed(this);
					return;
				}

				val = 255 - m_data[k];
				m_img.setPixel(x, y, qRgb(val, val, val));
				m_img.setAlphaBuffer(false);
			}
		}
	} else if (bh.bytes == 2) {
// 		kdDebug() << "Loading grayscale + alpha pattern " << &name[0] << endl;
		// Grayscale + A
		Q_INT32 val;
		Q_INT32 alpha;
		for (Q_UINT32 y = 0; y < bh.height; y++) {
			for (Q_UINT32 x = 0; x < bh.width; x++, k++) {
				if (static_cast<Q_UINT32>(k + 2) > m_data.size()) {
					kdDebug() << "failed in grayA\n";
					emit ioFailed(this);
					return;
				}

				val = 255 - m_data[k];
				alpha = 255 - m_data[k++];
				m_img.setPixel(x, y, qRgba(val, val, val, alpha));
				m_img.setAlphaBuffer(true);
			}
		}
	} else if (bh.bytes == 3) {
// 		kdDebug() << "Loading rgb pattern " << &name[0] << endl;
		// RGB without alpha
		for (Q_UINT32 y = 0; y < bh.height; y++) {
			for (Q_UINT32 x = 0; x < bh.width; x++) {
				if (static_cast<Q_UINT32>(k + 3) > m_data.size()) {
					kdDebug() << "failed in RGB\n";
					emit ioFailed(this);
					return;
				}

				m_img.setPixel(x, y, qRgb(255 - m_data[k++],
							  255 - m_data[k++],
							  255 - m_data[k++]));
				m_img.setAlphaBuffer(false);
			}
		}
	} else if (bh.bytes == 4) {
// 		kdDebug() << "Loading rgba pattern " << &name[0] << endl;
		// Has alpha
		for (Q_UINT32 y = 0; y < bh.height; y++) {
			for (Q_UINT32 x = 0; x < bh.width; x++) {
				if (static_cast<Q_UINT32>(k + 4) > m_data.size()) {
					kdDebug() << "failed in RGBA\n";
					emit ioFailed(this);
					return;
				}

				m_img.setPixel(x, y, qRgba(255 - m_data[k++],
							   255 - m_data[k++],
							   255 - m_data[k++],
							   255 - m_data[k++]));
				m_img.setAlphaBuffer(true);
			}
		}
	} else {
		emit ioFailed(this);
		return;
	}



	if (m_img.isNull()) {
		emit ioFailed(this);
		return;
	}

	// create pixmap for preview dialog
	m_pixmap = new QPixmap;
	m_pixmap -> convertFromImage(m_img, QPixmap::AutoColor);

	// scale a pixmap for iconview cell to size of cell
	if(m_img.width() > THUMB_SIZE || m_img.height() > THUMB_SIZE) {

		int xsize = THUMB_SIZE;
		int ysize = THUMB_SIZE;

		int picW  = m_img.width();
		int picH  = m_img.height();

		if(picW > picH) {
			float yFactor = (float)((float)(float)picH/(float)picW);
			ysize = (int)(yFactor * (float)THUMB_SIZE);
			//kdDebug() << "ysize is " << ysize << endl;
			if(ysize > 30) ysize = 30;
		}
		else if(picW < picH) {
			float xFactor = (float)((float)picW/(float)picH);
			xsize = (int)(xFactor * (float)THUMB_SIZE);
			//kdDebug() << "xsize is " << xsize << endl;
			if(xsize > 30) xsize = 30;
		}

		QImage thumbImg = m_img.smoothScale(xsize, ysize);


		if(!thumbImg.isNull()) {
			m_thumbPixmap = new QPixmap;
			m_thumbPixmap->convertFromImage(thumbImg);

			if(!m_thumbPixmap->isNull()) {
				m_validThumb = true;
			}
		}
	}

	setWidth(m_img.width());
	setHeight(m_img.height());

	setValid(true);

	int meanSize = (width() + height())/2;

	setSpacing(meanSize / 4);
	if(spacing() < 1)  setSpacing(1);
	if(spacing() > 20) setSpacing(20);

	// default hotspot
	m_hotSpot = QPoint(width()/2, height()/2);
// 	kdDebug() << "pattern loaded\n";
	emit loadComplete(this);
}

#include "kis_pattern.moc"
