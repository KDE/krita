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
#include <unistd.h>

#include <magick/api.h>

#include <qfile.h>
#include <qstring.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>

#include <qcolor.h>
#include <qstring.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_undo_adapter.h"
#include "kis_magick_converter.h"
#include "kis_colorspace_registry.h"
#include "kis_iterators_pixel.h"
#include "kis_profile.h"
#include "kis_strategy_colorspace.h"
#include "kis_image_builder.h"
#include "../../../config.h"


namespace {

	const PIXELTYPE PIXEL_BLUE = 0;
	const PIXELTYPE PIXEL_GREEN = 1;
	const PIXELTYPE PIXEL_RED = 2;
	const PIXELTYPE PIXEL_ALPHA = 3;

	/**
	 * Make this more flexible -- although... ImageMagick 
	 * isn't that flexible either.
	 */
	KisStrategyColorSpaceSP getColorSpaceForColorType(ColorspaceType type) {

		if (type == GRAYColorspace) {
			KisColorSpaceRegistry::instance() -> get("CMYK");
		}
		else if (type == CMYKColorspace) {
			return KisColorSpaceRegistry::instance() -> get("GRAY");
		}
		else if (type == RGBColorspace || type == sRGBColorspace || type == TransparentColorspace) {
			return KisColorSpaceRegistry::instance() -> get("RGBA");
		}
		return 0;
	
	}

}

KisMagickConverter::KisMagickConverter()
{
	m_stop = false;
}

KisMagickConverter::~KisMagickConverter()
{
}


KisPaintDeviceSP KisMagickConverter::fromMagickImage(Image * image, const QString name &)
{
	ExceptionInfo ei;
	GetExceptionInfo(&ei);
	ViewInfo *vi = OpenCacheView(image);

	if (image -> columns && image -> rows) {

		// Determine image type -- rgb, grayscale or cmyk
		KisStrategyColorSpaceSP cs = getColorSpaceForColorType(image -> colorspace);

		if (cs == 0) {
			CloseCacheView(vi);
			DestroyExceptionInfo(&ei);
			emit notifyProgressError(this);
			return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
		}

		KisPaintDeviceSP device = new KisPaintDeviceSP(cs, name);

		// Retrieve the profile -- if any
		ProfileInfo p = image -> profile;

		
		for (Q_UINT32 y = 0; y < image->rows; y ++)
		{
			const PixelPacket *pp = AcquireCacheView(vi, 0, y, image->columns, 1, &ei);
			
			if(!pp)
			{
				CloseCacheView(vi);
				DestroyExceptionInfo(&ei);
				emit notifyProgressError(this);
				return KisImageBuilder_RESULT_FAILURE;
			}
			
			KisHLineIteratorPixel hiter = device -> createHLineIterator(0, y, image->columns, true);
			while(! hiter.isDone())
			{
				Q_UINT8 *ptr= hiter.rawData();
				// XXX: not colorstrategy and bitdepth independent
				*(ptr++) = pp->blue;
				*(ptr++) = pp->green;
				*(ptr++) = pp->red;
				*(ptr++) = OPACITY_OPAQUE - pp->opacity;
				
				pp++;
				hiter++;
			}
			
			emit notifyProgress(this, y * 100 / image->rows);
			
			if (m_stop) {
				CloseCacheView(vi);
				DestroyExceptionInfo(&ei);
				m_img = 0;
				return 0;
			}
		}
		
		emit notifyProgressDone(this);
		CloseCacheView(vi);
		DestroyExceptionInfo(&ei);
		return device;
		
	}
	emit notifyProgressDone(this);

	CloseCacheView(vi);
	DestroyExceptionInfo(&ei);
	
	return 0;
}

KisImageBuilder_Result KisMagickConverter::buildImage(const KURL& uri)
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

KisImageBuilder_Result KisMagickConverter::buildFile(const KURL& uri, KisLayerSP layer)
{
#if 0 //AUTOLAYER
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
		emit notifyProgressError(this);
		return KisImageBuilder_RESULT_PATH;
	}

	if (!layer -> width() || !layer -> height())
		return KisImageBuilder_RESULT_EMPTY;

	image = AllocateImage(ii);
	tm = layer -> tiles();
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
				emit notifyProgressError(this);
				return KisImageBuilder_RESULT_FAILURE;
			}

			ntile++;
			emit notifyProgressStage(this, i18n("Saving..."), ntile * 100 / totalTiles);
			tile2pp(pp, pd);
			SyncImagePixels(image);
			w = TILE_WIDTH;
		}

		h = TILE_HEIGHT;
	}

	WriteImage(ii, image);
	DestroyExceptionInfo(&ei);
	DestroyImage(image);
	emit notifyProgressDone(this);
	return KisImageBuilder_RESULT_OK;
#endif // AUTOLAYER (below return is a hack)
		return KisImageBuilder_RESULT_EMPTY;
}

void KisMagickConverter::cancel()
{
	m_stop = true;
}

#include "kis_magick_converter.moc"

