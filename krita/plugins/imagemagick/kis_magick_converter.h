/*
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
#ifndef KIS_IMAGE_MAGICK_CONVERTER_H_
#define KIS_IMAGE_MAGICK_CONVERTER_H_


#include <magick/api.h>

#include <qobject.h>
#include <qvaluevector.h>
#include <qstring.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_progress_subject.h"

class QString;
class KURL;
class KisDoc;
class KisNameServer;
class KisUndoAdapter;

/**
 * Build a KisPaintDevice from an ImageMagick Image or vice versa.
 *
 * This is subtler than a simple conversion to or from a QImage since
 * it supports progress and cancel.
 */
class KisMagickConverter : public KisProgressSubject {

	typedef QObject super;
	Q_OBJECT

public:
	KisMagickConverter();
	virtual ~KisMagickConverter();

public slots:

	virtual void cancel();

public:

	KisPaintDeviceSP fromMagickImage(Image * image, const QString name &);
	Image * toMagickImage(KisPaintDeviceSP device);



private:
	KisMagickConverter(const KisMagickConverter&);
	KisMagickConverter& operator=(const KisMagickConverter&);

private:
	bool m_stop;
};

#endif // KIS_IMAGE_MAGICK_CONVERTER_H_

