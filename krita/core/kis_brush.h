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
#include <qvaluevector.h>
#include <kio/job.h>
#include "kis_krayon.h"
#include "kis_resource.h"

class QPoint;
class QPixmap;

class KisBrush : public KisResource {
	typedef KisResource super;
	Q_OBJECT

public:
	KisBrush(const QString& filename);
	virtual ~KisBrush();

public:
	virtual bool loadAsync();
	virtual bool saveAsync();
	virtual QImage img() const;
	virtual QImage frame(Q_INT32 n) const;

public:
	void setSpacing(Q_INT32 s) { m_spacing = s; }
	Q_INT32 spacing() const { return m_spacing; }
	void setHotSpot(QPoint);
	QPoint hotSpot() const { return m_hotSpot; }

	uchar value(Q_INT32 x, Q_INT32 y) const;
	uchar *scanline(Q_INT32 i) const;
	uchar *bits() const;

private slots:
	void ioData(KIO::Job *job, const QByteArray& data);
	void ioResult(KIO::Job *job);

private:
	QValueVector<Q_UINT8> m_data;
	Q_INT32 m_spacing;
	QPoint m_hotSpot;
	QImage m_img;
};

#endif // KIS_BRUSH_

