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
#if !defined KIS_IMAGE_MAGICK_CONVERTER_H_
#define KIS_IMAGE_MAGICK_CONVERTER_H_

#include <qobject.h>
#include <qvaluevector.h>
#include <kio/job.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_image_builder.h"
#include "kis_progress_subject.h"

class QString;
class KURL;
class KisDoc;
class KisNameServer;
class KisUndoAdapter;

/**
 * Build a KisImage representation of an image file.
 */
class KisImageMagickConverter : public KisProgressSubject {
	typedef QObject super;
	Q_OBJECT

public:
	KisImageMagickConverter(KisDoc *doc, KisUndoAdapter *adapter);
	virtual ~KisImageMagickConverter();

public slots:
	virtual void cancel();

public:
	KisImageBuilder_Result buildImage(const KURL& uri);
	KisImageBuilder_Result buildFile(const KURL& uri, KisImageSP img);
	KisImageBuilder_Result buildFile(const KURL& uri, KisLayerSP layer);
	KisImageSP image();

public:
	static QString readFilters();
	static QString writeFilters();

private slots:
	void ioData(KIO::Job *job, const QByteArray& data);
	void ioResult(KIO::Job *job);
	void ioTotalSize(KIO::Job *job, KIO::filesize_t size);

private:
	KisImageMagickConverter(const KisImageMagickConverter&);
	KisImageMagickConverter& operator=(const KisImageMagickConverter&);
	void init(KisDoc *doc, KisUndoAdapter *adapter);
	KisImageBuilder_Result decode(const KURL& uri, bool isBlob);

private:
	KisImageSP m_img;
	KisDoc *m_doc;
	KisUndoAdapter *m_adapter;
	QValueVector<Q_UINT8> m_data;
	KIO::TransferJob *m_job;
	KIO::filesize_t m_size;
	bool m_stop;
};

#endif // KIS_IMAGE_MAGICK_CONVERTER_H_

