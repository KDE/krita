/*
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
#include "kis_imagepipe_brush.h"
#include "kis_brush.h"
#include "kis_alpha_mask.h"

KisImagePipeBrush::KisImagePipeBrush(const QString& filename) : super(filename)
{
	m_brushType = INVALID;
	m_numOfBrushes = 0;
	m_currentBrush = 0;

}

KisImagePipeBrush::~KisImagePipeBrush()
{
	m_brushes.setAutoDelete(true);
	m_brushes.clear();
}

bool KisImagePipeBrush::loadAsync()
{
	KIO::Job *job = KIO::get(filename(), false, false);

	connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(ioData(KIO::Job*, const QByteArray&)));
	connect(job, SIGNAL(result(KIO::Job*)), SLOT(ioResult(KIO::Job*)));
	return true;
}

bool KisImagePipeBrush::saveAsync()
{
	return false;
}

QImage KisImagePipeBrush::img()
{
	if (m_brushes.isEmpty()) return 0;
	// XXX: This does not follow the instructions in the 'parasite'
	if (m_currentBrush == m_brushes.count()) {
		m_currentBrush = 0;
	}
	m_currentBrush++;
	return m_brushes.at(m_currentBrush - 1) -> img();
}

KisAlphaMask *KisImagePipeBrush::mask(Q_INT32 scale)
{
	if (m_brushes.isEmpty()) return 0;
	// XXX: This does not follow the instructions in the 'parasite'
	if (m_currentBrush == m_brushes.count()) {
		m_currentBrush = 0;
	}
	m_currentBrush++;
	return m_brushes.at(m_currentBrush - 1) -> mask(scale);
}

void KisImagePipeBrush::setHotSpot(QPoint pt)
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


void KisImagePipeBrush::setParasite(const QString& parasite)
{
	m_parasite = parasite;
}


enumBrushType KisImagePipeBrush::brushType() const {
	return m_brushType;
}

void KisImagePipeBrush::ioData(KIO::Job * /*job*/, const QByteArray& data)
{
	if (!data.isEmpty()) {
		Q_INT32 startPos = m_data.size();

		m_data.resize(m_data.size() + data.count());
		memcpy(&m_data[startPos], data.data(), data.count());
	}
}

void KisImagePipeBrush::ioResult(KIO::Job * /*job*/)
{
	// XXX: this doesn't correctly load the image pipe brushes yet.

	// XXX: This stuff is in utf-8, too.
	// The first line contains the name -- this means we look until we arrive at the first newline
	QValueVector<char> line1;

	Q_UINT32 i = 0;

	while (m_data[i] != '\n' && i < m_data.size()) {
		line1.append(m_data[i]);
		i++;
	}
	setName(QString::fromUtf8(&line1[0], i));

	i++; // Skip past the first newline

	// The second line contains the number of brushes, separated by a space from the parasite

	// XXX: This stuff is in utf-8, too.
 	QValueVector<char> line2;
 	while (m_data[i] != '\n' && i < m_data.size()) {
		line2.append(m_data[i]);
 		i++;
 	}

	QString paramline = QString::fromUtf8((&line2[0]), line2.size());
	Q_UINT32 m_numOfBrushes = paramline.left(paramline.find(' ')).toUInt();
	m_parasite = paramline.mid(paramline.find(' ') + 1);
	
	i++; // Skip past the second newline

 	Q_UINT32 numOfBrushes = 0;
  	while (numOfBrushes < m_numOfBrushes && i < m_data.size()){
		KisBrush * brush = new KisBrush(name() + "_" + numOfBrushes,
						m_data,
						i);
		m_brushes.append(brush);
		
 		numOfBrushes++;
 	}

	if (!m_brushes.isEmpty()) {
		setValid(true);
		if (m_brushes.at( 0 ) -> brushType() == MASK) {
			m_brushType = PIPE_MASK;
		}
		else {
			m_brushType = PIPE_IMAGE;
		}
	}

	emit loadComplete(this);
}
#include "kis_imagepipe_brush.moc"

