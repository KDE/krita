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

#include <qpoint.h>
#include <qsize.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qfileinfo.h>

#include <kimageeffect.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include "kis_pattern.h"

#define THUMB_SIZE 30


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

QImage KisPattern::img() const
{
	return m_img;
}

QImage KisPattern::frame(Q_INT32) const
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
	// load via QImage
	m_img = QImage(m_data);
	m_img.setAlphaBuffer(true);

	if (m_img.isNull()) {
		emit ioFailed(this);
		return;
	}

	m_img = m_img.convertDepth(32);

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
	
	// We used to have patterninfo files , analogous to
	// brushinfo files that have also disappeared.
	// We should use Gimp .pat patterns here, too, instead
	// of plain image files.
	kdDebug() << "pattern loaded\n";
	emit loadComplete(this);
}

#include "kis_pattern.moc"
