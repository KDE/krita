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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <magick/api.h>

#include <qfile.h>
#include <qstring.h>

#include <kdeversion.h>
#include <kapplication.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include <koColor.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_undo_adapter.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kispixeldata.h"
#include "kis_image_magick_converter.h"
#include "kis_colorspace_registry.h"

#include "../../../config.h"

namespace {
	inline
	void pp2tile(KisPixelDataSP pd, const PixelPacket *pp)
	{
		Q_INT32 i;
		Q_INT32 j;
		QUANTUM *pixel = pd -> data;

		for (j = 0; j < pd -> height; j++) {
			for (i = 0; i < pd -> width; i++) {
				pixel[PIXEL_RED] = pp -> red;
				pixel[PIXEL_GREEN] = pp -> green;
				pixel[PIXEL_BLUE] = pp -> blue;
				pixel[PIXEL_ALPHA] = OPACITY_OPAQUE - pp -> opacity;
				pixel += pd -> depth;
				pp++;
			}
		}
	}

	inline
	void tile2pp(PixelPacket *pp, const KisPixelDataSP pd)
	{
		Q_INT32 i;
		Q_INT32 j;
		QUANTUM *pixel = pd -> data;

		for (j = 0; j < pd -> height; j++) {
			for (i = 0; i < pd -> width; i++) {
				pp -> red = Upscale(pixel[PIXEL_RED]);
				pp -> green = Upscale(pixel[PIXEL_GREEN]);
				pp -> blue = Upscale(pixel[PIXEL_BLUE]);
				pp -> opacity = OPACITY_OPAQUE - Upscale(pixel[PIXEL_ALPHA]);
				pixel += pd -> depth;
				pp++;
			}
		}
	}

	void InitGlobalMagick()
	{
		static bool init = false;

		if (!init) {
			KApplication *app = KApplication::kApplication();

			InitializeMagick(*app -> argv());
			atexit(DestroyMagick);
			init = true;
		}
	}

	/*
	 * ImageMagick progress monitor callback.  Unfortunately it doesn't support passing in some user
	 * data which complicates things quite a bit.  The plan was to allow the user start multiple
	 * import/scans if he/she so wished.  However, without passing user data it's not possible to tell
	 * on which task we have made progress on.
	 *
	 * Additionally, ImageMagick is thread-safe, not re-entrant... i.e. IM does not relinquish held
	 * locks when calling user defined callbacks, this means that the same thread going back into IM
	 * would deadlock since it would try to acquire locks it already holds.
	 */
#ifdef HAVE_MAGICK6
	MagickBooleanType monitor(const char *text, const ExtendedSignedIntegralType, const ExtendedUnsignedIntegralType, ExceptionInfo *)
	{
		KApplication *app = KApplication::kApplication();

		// TODO : Figure something out for above problems
		Q_ASSERT(app);

		if (app -> hasPendingEvents())
			app -> processEvents();

		printf("%s\n", text);
		return MagickTrue;
	}
#else
	unsigned int monitor(const char *text, const ExtendedSignedIntegralType, const ExtendedUnsignedIntegralType, ExceptionInfo *)
	{
		KApplication *app = KApplication::kApplication();

		// TODO : Figure something out for above problems
		Q_ASSERT(app);

		if (app -> hasPendingEvents())
			app -> processEvents();

		printf("%s\n", text);
		return true;
	}
#endif

}

KisImageMagickConverter::KisImageMagickConverter(KisDoc *doc, KisUndoAdapter *adapter)
{
	InitGlobalMagick();
	init(doc, adapter);
	SetMonitorHandler(monitor);
	m_stop = false;
}

KisImageMagickConverter::~KisImageMagickConverter()
{
}

KisImageBuilder_Result KisImageMagickConverter::decode(const KURL& uri, bool isBlob)
{
	Image *image;
	Image *images;
	ExceptionInfo ei;
	ImageInfo *ii;

	if (m_stop) {
		m_img = 0;
		return KisImageBuilder_RESULT_INTR;
	}

	GetExceptionInfo(&ei);
	ii = CloneImageInfo(0);

	if (isBlob) {
		// TODO : Test.  Does BlobToImage even work?
		Q_ASSERT(uri.isEmpty());
		images = BlobToImage(ii, &m_data[0], m_data.size(), &ei);
	} else {
		qstrncpy(ii -> filename, QFile::encodeName(uri.path()), MaxTextExtent - 1);

		if (ii -> filename[MaxTextExtent - 1]) {
			emit notify(this, KisImageBuilder_STEP_ERROR, 0);
			return KisImageBuilder_RESULT_PATH;
		}

		images = ReadImage(ii, &ei);
	}

	if (ei.severity != UndefinedException)
		CatchException(&ei);

	if (images == 0) {
		DestroyImageInfo(ii);
		DestroyExceptionInfo(&ei);
		emit notify(this, KisImageBuilder_STEP_ERROR, 0);
		return KisImageBuilder_RESULT_FAILURE;
	}
	m_img = new KisImage(m_adapter, 0, 0, KisColorSpaceRegistry::singleton()->colorSpace("RGBA"), m_doc -> nextImageName());
	emit notify(this, KisImageBuilder_STEP_TILING, 0);

	while ((image = RemoveFirstImageFromList(&images))) {
		ViewInfo *vi = OpenCacheView(image);

		if (image -> columns && image -> rows) {
			Q_INT32 totalTiles = ((image -> columns + TILE_WIDTH - 1) / TILE_WIDTH) * ((image -> rows + TILE_HEIGHT - 1) / TILE_HEIGHT);
			Q_INT32 ntile = 0;
			KisLayerSP layer = new KisLayer(m_img, image -> columns, image -> rows, m_img -> nextLayerName(), OPACITY_OPAQUE);
			KisTileMgrSP tm = layer -> data();
			Q_INT32 w = TILE_WIDTH;
			Q_INT32 h = TILE_HEIGHT;

			m_img -> add(layer, 0);

			for (Q_INT32 y = 0; y < m_img -> height(); y += TILE_HEIGHT) {
				if ((y + h) > m_img -> height())
					h = TILE_HEIGHT + m_img -> height() - (y + h);

				for (Q_INT32 x = 0; x < m_img -> width(); x += TILE_WIDTH) {
					if ((x + w) > m_img -> width())
						w = TILE_WIDTH + m_img -> width() - (x + w);

					const PixelPacket *pp = AcquireCacheView(vi, x, y, w, h, &ei);
					KisPixelDataSP pd = tm -> pixelData(x, y, x + w - 1, y + h - 1, TILEMODE_RW);

					if (!pd || !pp) {
						CloseCacheView(vi);
						DestroyImageList(images);
						DestroyImageInfo(ii);
						DestroyExceptionInfo(&ei);
						emit notify(this, KisImageBuilder_STEP_ERROR, 0);
						return KisImageBuilder_RESULT_FAILURE;
					}

					pp2tile(pd, pp);
					tm -> releasePixelData(pd);
					w = TILE_WIDTH;
					ntile++;
					emit notify(this, KisImageBuilder_STEP_TILING, ntile * 100 / totalTiles);

					if (m_stop) {
						CloseCacheView(vi);
						DestroyImageList(images);
						DestroyImageInfo(ii);
						DestroyExceptionInfo(&ei);
						m_img = 0;
						return KisImageBuilder_RESULT_INTR;
					}

				}

				h = TILE_HEIGHT;
			}
		}

		emit notify(this, KisImageBuilder_STEP_DONE, 100);
		CloseCacheView(vi);
		DestroyImage(image);
	}

	emit notify(this, KisImageBuilder_STEP_DONE, 100);
	DestroyImageList(images);
	DestroyImageInfo(ii);
	DestroyExceptionInfo(&ei);
	m_img -> invalidate();
	return KisImageBuilder_RESULT_OK;
}

KisImageBuilder_Result KisImageMagickConverter::buildImage(const KURL& uri)
{
	if (uri.isEmpty())
		return KisImageBuilder_RESULT_NO_URI;

	if (!KIO::NetAccess::exists(uri, false, qApp -> mainWidget())) {
		return KisImageBuilder_RESULT_NOT_EXIST;
	}

#if 1
	// We're not set up to handle asynchronous loading at the moment.
	KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
	QString tmpFile;

	if (KIO::NetAccess::download(uri, tmpFile, qApp -> mainWidget())) {
		result = decode(tmpFile, false);
		KIO::NetAccess::removeTempFile(tmpFile);
	}

	return result;
#else
	if (!uri.isLocalFile()) {
		if (m_job)
			return KisImageBuilder_RESULT_BUSY;

		m_data.resize(0);
		m_job = KIO::get(uri, false, false);
		connect(m_job, SIGNAL(result(KIO::Job*)), SLOT(ioResult(KIO::Job*)));
		connect(m_job, SIGNAL(totalSize(KIO::Job*, KIO::filesize_t)), this, SLOT(ioTotalSize(KIO::Job*, KIO::filesize_t)));
		connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(ioData(KIO::Job*, const QByteArray&)));
		return KisImageBuilder_RESULT_PROGRESS;
	}

	return decode(uri, false);
#endif
}

KisImageSP KisImageMagickConverter::image()
{
	return m_img;
}

void KisImageMagickConverter::init(KisDoc *doc, KisUndoAdapter *adapter)
{
	m_doc = doc;
	m_adapter = adapter;
	m_job = 0;
}

KisImageBuilder_Result buildFile(const KURL&, KisImageSP)
{
	return KisImageBuilder_RESULT_UNSUPPORTED;
}

KisImageBuilder_Result KisImageMagickConverter::buildFile(const KURL& uri, KisLayerSP layer)
{
	Image *image;
	ExceptionInfo ei;
	ImageInfo *ii;
	Q_INT32 w;
	Q_INT32 h;
	KisTileMgrSP tm;
	Q_INT32 ntile = 0;
	Q_INT32 totalTiles;

	if (!layer)
		return KisImageBuilder_RESULT_INVALID_ARG;

	if (uri.isEmpty())
		return KisImageBuilder_RESULT_NO_URI;

	if (!uri.isLocalFile())
		return KisImageBuilder_RESULT_NOT_LOCAL;

	GetExceptionInfo(&ei);
	ii = CloneImageInfo(0);
	qstrncpy(ii -> filename, QFile::encodeName(uri.path()), MaxTextExtent - 1);

	if (ii -> filename[MaxTextExtent - 1]) {
		emit notify(this, KisImageBuilder_STEP_ERROR, 0);
		return KisImageBuilder_RESULT_PATH;
	}

	if (!layer -> width() || !layer -> height())
		return KisImageBuilder_RESULT_EMPTY;

	image = AllocateImage(ii);
	tm = layer -> data();
	image -> columns = layer -> width();
	image -> rows = layer -> height();
#ifdef HAVE_MAGICK6
	if ( layer-> alpha() )
		image -> matte = MagickTrue;
	else
		image -> matte = MagickFalse;
#else       
	image -> matte = layer -> alpha();
#endif
	w = TILE_WIDTH;
	h = TILE_HEIGHT;
	totalTiles = ((image -> columns + TILE_WIDTH - 1) / TILE_WIDTH) * ((image -> rows + TILE_HEIGHT - 1) / TILE_HEIGHT);

	for (Q_INT32 y = 0; y < layer -> height(); y += TILE_HEIGHT) {
		if ((y + h) > layer -> height())
			h = TILE_HEIGHT + layer -> height() - (y + h);

		for (Q_INT32 x = 0; x < layer -> width(); x += TILE_WIDTH) {
			if ((x + w) > layer -> width())
				w = TILE_WIDTH + layer -> width() - (x + w);

			KisPixelDataSP pd = tm -> pixelData(x, y, x + w - 1, y + h - 1, TILEMODE_READ);
			PixelPacket *pp = SetImagePixels(image, x, y, w, h);

			if (!pd || !pp) {
				if (pp)
					SyncImagePixels(image);

				DestroyExceptionInfo(&ei);
				DestroyImage(image);
				emit notify(this, KisImageBuilder_STEP_ERROR, 0);
				return KisImageBuilder_RESULT_FAILURE;
			}

			ntile++;
			emit notify(this, KisImageBuilder_STEP_SAVING, ntile * 100 / totalTiles);
			tile2pp(pp, pd);
			SyncImagePixels(image);
			w = TILE_WIDTH;
		}

		h = TILE_HEIGHT;
	}

	WriteImage(ii, image);
	DestroyExceptionInfo(&ei);
	DestroyImage(image);
	emit notify(this, KisImageBuilder_STEP_DONE, 100);
	return KisImageBuilder_RESULT_OK;
}

void KisImageMagickConverter::ioData(KIO::Job *job, const QByteArray& data)
{
	if (data.isNull() || data.isEmpty()) {
		emit notify(this, KisImageBuilder_STEP_LOADING, 0);
		return;
	}

	if (m_data.empty()) {
		Image *image;
		ImageInfo *ii;
		ExceptionInfo ei;

		ii = CloneImageInfo(0);
		GetExceptionInfo(&ei);
		image = PingBlob(ii, data.data(), data.size(), &ei);

		if (image == 0 || ei.severity == BlobError) {
			DestroyExceptionInfo(&ei);
			DestroyImageInfo(ii);
			job -> kill();
			emit notify(this, KisImageBuilder_STEP_ERROR, 0);
			return;
		}

		DestroyImage(image);
		DestroyExceptionInfo(&ei);
		DestroyImageInfo(ii);
		emit notify(this, KisImageBuilder_STEP_LOADING, 0);
	}

	Q_ASSERT(data.size() + m_data.size() <= m_size);
	memcpy(&m_data[m_data.size()], data.data(), data.count());
	m_data.resize(m_data.size() + data.count());
	emit notify(this, KisImageBuilder_STEP_LOADING, m_data.size() * 100 / m_size);

	if (m_stop)
		job -> kill();
}

void KisImageMagickConverter::ioResult(KIO::Job *job)
{
	m_job = 0;

	if (job -> error())
		emit notify(this, KisImageBuilder_STEP_ERROR, 0);

	decode(KURL(), true);
}

void KisImageMagickConverter::ioTotalSize(KIO::Job * /*job*/, KIO::filesize_t size)
{
	m_size = size;
	m_data.reserve(size);
	emit notify(this, KisImageBuilder_STEP_LOADING, 0);
}

void KisImageMagickConverter::intr()
{
	m_stop = true;
}

/**
 * @name readFilters
 * @return Provide a list of file formats the application can read.
 */
QString KisImageMagickConverter::readFilters()
{
	QString s;
	QString all;
	QString name;
	QString description;
	ExceptionInfo ei;
	const MagickInfo *mi;

	GetExceptionInfo(&ei);
	mi = GetMagickInfo("*", &ei);

	if (!mi)
		return s;

	for (; mi; mi = reinterpret_cast<const MagickInfo*>(mi -> next)) {
		if (mi -> stealth)
			continue;

		if (mi -> decoder) {
			name = mi -> name;
			description = mi -> description;

			if (!description.isEmpty() && !description.contains('/')) {
				all += "*." + name.lower() + " *." + name + " ";
				s += "*." + name.lower() + " *." + name + "|";
				s += i18n(description.utf8());
				s += "\n";
			}
		}
	}

	all += "|" + i18n("All Images");
	all += "\n";
	DestroyExceptionInfo(&ei);
	return all + s;
}

QString KisImageMagickConverter::writeFilters()
{
	QString s;
	QString all;
	QString name;
	QString description;
	ExceptionInfo ei;
	const MagickInfo *mi;

	GetExceptionInfo(&ei);
	mi = GetMagickInfo("*", &ei);

	if (!mi)
		return s;

	for (; mi; mi = reinterpret_cast<const MagickInfo*>(mi -> next)) {
		if (mi -> stealth)
			continue;

		if (mi -> encoder) {
			name = mi -> name;
			description = mi -> description;

			if (!description.isEmpty() && !description.contains('/')) {
				all += "*." + name.lower() + " *." + name + " ";
				s += "*." + name.lower() + " *." + name + "|";
				s += i18n(description.utf8());
				s += "\n";
			}
		}
	}

	all += "|" + i18n("All Images");
	all += "\n";
	DestroyExceptionInfo(&ei);
	return all + s;
}

#include "kis_image_magick_converter.moc"

