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
#if !defined KIS_IMAGEPIPE_BRUSH_
#define KIS_IMAGEPIPE_BRUSH_

#include <qptrlist.h>
#include <qvaluelist.h>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_brush.h"
#include "kis_global.h"

class QCString;
class QImage;
class QPoint;
class QSize;

class KisAlphaMask;

class KisImagePipeBrush : public KisBrush {
	typedef KisBrush super;
	Q_OBJECT

public:
	KisImagePipeBrush(const QString& filename);
	virtual ~KisImagePipeBrush();

	virtual bool loadAsync();
	virtual bool saveAsync();

	/**
	  @return the next image in the pipe.
	  */
	virtual QImage img();

	/**
	   @return the next mask in the pipe.
	*/
	virtual KisAlphaMask *mask(Q_INT32 pressure = PRESSURE_DEFAULT) const;

	virtual enumBrushType brushType() const;

private slots:
	void ioData(KIO::Job *job, const QByteArray& data);
	void ioResult(KIO::Job *job);

private:

	void setParasite(const QString& parasite);

	QString m_name;
	QString m_parasite; // This contains some kind of instructions on how to use the brush
			  // That I haven't decoded yet.
	Q_UINT32 m_numOfBrushes;
	mutable Q_UINT32 m_currentBrush;

	QValueVector<Q_UINT8> m_data;
	mutable QPtrList<KisBrush> m_brushes;

	enumBrushType m_brushType;
	
};

#endif // KIS_IMAGEPIPE_BRUSH_
