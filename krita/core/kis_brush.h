/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#if !defined KIS_BRUSH_
#define KIS_BRUSH_

#include <qcstring.h>
#include <qimage.h>
#include <qsize.h>
#include <qptrlist.h>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_alpha_mask.h"
#include "kis_global.h"

class QPoint;
class QPixmap;

class KisBrush : public KisResource {
	typedef KisResource super;
	Q_OBJECT

public:
	KisBrush(const QString& filename);
	virtual ~KisBrush();

	virtual bool loadAsync();
	virtual bool saveAsync();
	virtual QImage img() const;

	/**
	   @return a mask computed from the grey-level values of the
	   pixels in the brush.
	*/
	virtual KisAlphaMask *mask(Q_INT32 scale = PRESSURE_LEVELS / 2);

	void setHotSpot(QPoint);
	QPoint hotSpot() const { return m_hotSpot; }

	uchar value(Q_INT32 x, Q_INT32 y) const;
	uchar *scanline(Q_INT32 i) const;
	uchar *bits() const;

private slots:
	void ioData(KIO::Job *job, const QByteArray& data);
	void ioResult(KIO::Job *job);

private:
	void createMasks(const QImage & img);
	
	QValueVector<Q_UINT8> m_data;
	QPoint m_hotSpot;
	QImage m_img;
	QPtrList<KisAlphaMask> m_masks;
	
};
#endif // KIS_BRUSH_

