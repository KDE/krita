/*
 *  kis_gradient.cc - part of Krayon
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

#include "kis_gradient.h"

#define THUMB_SIZE 30

namespace {
	struct GimpGradientHeader {
		Q_UINT32 header_size;  /*  header_size = sizeof (GradientHeader) + brush name  */
		Q_UINT32 version;      /*  gradient file version #  */
		Q_UINT32 width;        /*  width of gradient */
		Q_UINT32 height;       /*  height of gradient  */
		Q_UINT32 bytes;        /*  depth of gradient in bytes : 1, 2, 3 or 4*/
		Q_UINT32 magic_number; /*  GIMP brush magic number  */
	};
}


KisGradient::KisGradient(const QString& file) : super(file)
{
	m_valid = false;
	m_hotSpot = QPoint(0, 0);
	m_pixmap = 0;
	m_thumbPixmap = 0;
}

KisGradient::~KisGradient()
{
	if (m_pixmap) delete m_pixmap;
	if (m_thumbPixmap) delete m_thumbPixmap;
}


bool KisGradient::loadAsync()
{
	KIO::Job *job = KIO::get(filename(), false, false);

	connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(ioData(KIO::Job*, const QByteArray&)));
	connect(job, SIGNAL(result(KIO::Job*)), SLOT(ioResult(KIO::Job*)));
	return true;
}

bool KisGradient::saveAsync()
{
	return false;
}

QImage KisGradient::img()
{
	return m_img;
}


QPixmap& KisGradient::pixmap() const
{
	return *m_pixmap;
}

QPixmap& KisGradient::thumbPixmap() const
{
	return *m_thumbPixmap;
}


void KisGradient::ioData(KIO::Job * /*job*/, const QByteArray& data)
{
	if (!data.isEmpty()) {
		Q_INT32 startPos = m_data.size();

		m_data.resize(m_data.size() + data.count());
		memcpy(&m_data[startPos], data.data(), data.count());
	}
}

void KisGradient::ioResult(KIO::Job * /*job*/)
{
	// load Gimp gradients
	emit loadComplete(this);
}

#include "kis_gradient.moc"
