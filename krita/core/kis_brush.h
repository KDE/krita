/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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
#if !defined KIS_BRUSH_
#define KIS_BRUSH_

#include <qcstring.h>
#include <qsize.h>
#include <qptrlist.h>
#include <qimage.h>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_alpha_mask.h"
#include "kis_global.h"
#include "kis_layer.h"

class QPoint;
class QPixmap;

enum enumBrushType {
        INVALID,
	MASK,
	IMAGE,
	PIPE_MASK,
	PIPE_IMAGE
};

class KisBrush : public KisResource {
	typedef KisResource super;
	Q_OBJECT

public:
	KisBrush(const QString& filename);
	KisBrush(const QString& filename, 
		 const QByteArray & data,
		 Q_UINT32 & dataPos);
	
	virtual ~KisBrush();

	virtual bool loadAsync();
	virtual bool saveAsync();
	virtual QImage img();

	/**
	   @return a mask computed from the grey-level values of the
	   pixels in the brush.
	*/
	virtual KisAlphaMask *mask(Q_INT32 pressure = PRESSURE_DEFAULT) const;
	virtual KisLayerSP image(Q_INT32 pressure = PRESSURE_DEFAULT) const;

	void setHotSpot(QPoint);
	QPoint hotSpot(Q_INT32 pressure = PRESSURE_DEFAULT) const;

	void setSpacing(Q_INT32 s) { m_spacing = s; }
	Q_INT32 spacing() const { return m_spacing; }
	Q_INT32 xSpacing(Q_INT32 pressure = PRESSURE_DEFAULT) const;
	Q_INT32 ySpacing(Q_INT32 pressure = PRESSURE_DEFAULT) const;

	virtual void setUseColorAsMask(bool useColorAsMask) { m_useColorAsMask = useColorAsMask; }
	virtual bool useColorAsMask() const { return m_useColorAsMask; }
	virtual bool hasColor() const;

	virtual enumBrushType brushType() const;


private slots:
	void ioData(KIO::Job *job, const QByteArray& data);
	void ioResult(KIO::Job *job);

private:
	void createMasks(const QImage & img) const;
	void createImages(const QImage & img) const;

	QByteArray m_data;
	bool m_ownData;
	QPoint m_hotSpot;
	Q_INT32 m_spacing;
	bool m_useColorAsMask;
	bool m_hasColor;
	QImage m_img;
	mutable QPtrList<KisAlphaMask> m_masks;
	mutable QValueVector<KisLayerSP> m_images;

	Q_UINT32 m_header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
	Q_UINT32 m_version;      /*  brush file version #  */
	Q_UINT32 m_width;        /*  width of brush  */
	Q_UINT32 m_height;       /*  height of brush  */
	Q_UINT32 m_bytes;        /*  depth of brush in bytes */
	Q_UINT32 m_magic_number; /*  GIMP brush magic number  */

	enumBrushType m_brushType;

};
#endif // KIS_BRUSH_

