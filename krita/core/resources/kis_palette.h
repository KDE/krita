/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PALETTE_
#define KIS_PALETTE_

#include <qcstring.h>
#include <qsize.h>
#include <qimage.h>
#include <qcolor.h>
#include <qvaluevector.h>

#include <kio/job.h>
#include <kpalette.h>

#include "kis_types.h"
#include "kis_resource.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_point.h"
#include "kis_gradient.h"
#include "kis_alpha_mask.h"

class QPoint;
class QPixmap;
class KisPaintDevice;

struct KisPaletteEntry {
	QColor color;
	QString name;
};

/**
 * Open Gimp, Photoshop or RIFF palette files. This is a straight port
 * from the Gimp.
 */
class KisPalette : public KisResource {
	typedef KisResource super;

	Q_OBJECT

public:
	/**
	 * Create a palette from the colours in an image
	 */
	KisPalette(const QImage * img, Q_INT32 nColors, const QString & name);

	/**
	 * Create a palette from the colours in a paint device
	 */
	KisPalette(const KisPaintDeviceSP device, Q_INT32 nColors, const QString & name);

	/**
	 * Create a palette from the colours in a gradient
	 */
	KisPalette(const KisGradient * gradient, Q_INT32 nColors, const QString & name);

	/**
	 * Load a palette from a file. This can be a Gimp
	 * palette, a RIFF palette or a Photoshop palette.
	 */
	KisPalette(const QString& filename);

	virtual ~KisPalette();

	virtual bool loadAsync();
	virtual bool saveAsync();
	virtual QImage img();


public:

	void add(const KisPaletteEntry &);
	void remove(const KisPaletteEntry &);
	KisPaletteEntry getColor(Q_UINT32 index);
	Q_INT32 nColors();

	
private slots:

	void ioData(KIO::Job *job, const QByteArray& data);
	void ioResult(KIO::Job *job);

private:

	QByteArray m_data;
	bool m_ownData;
	QImage m_img;
	QString m_name;
	QString m_comment;
	Q_INT32 m_columns;
	QValueVector<KisPaletteEntry> m_colors;
	
};
#endif // KIS_PALETTE_

