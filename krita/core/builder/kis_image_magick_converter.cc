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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <qimage.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <klocale.h>
#include <ksharedptr.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <koColor.h>
#include <Magick++.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kispixeldata.h"
#include "kis_layer.h"
#include "kis_image_magick_converter.h"

typedef QValueList<Magick::Image> mimglist;

namespace {
	inline
	void pp2tile(KisPixelDataSP pd, const Magick::PixelPacket *pp)
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
	void pp2tile(Magick::PixelPacket *pp, const KisPixelDataSP pd)
	{
		Q_INT32 i;
		Q_INT32 j;
		QUANTUM *pixel = pd -> data;

		for (j = 0; j < pd -> height; j++) {
			for (i = 0; i < pd -> width; i++) {
				pp -> red = pixel[PIXEL_RED];
				pp -> green = pixel[PIXEL_GREEN];
				pp -> blue = pixel[PIXEL_BLUE];
				pp -> opacity = OPACITY_OPAQUE - pixel[PIXEL_ALPHA];
				pixel += pd -> depth;
				pp++;
			}
		}
	}
}

KisImageMagickConverter::KisImageMagickConverter(KisDoc *doc)
{
	init(doc);
}

KisImageMagickConverter::~KisImageMagickConverter()
{
}

KisImageBuilder_Result KisImageMagickConverter::buildImage(const KURL& uri)
{
	try {
		mimglist mimages;
		KisImageSP img;

		if (uri.isEmpty())
			return KisImageBuilder_RESULT_NO_URI;

		if (!KIO::NetAccess::exists(uri))
			return KisImageBuilder_RESULT_NOT_EXIST;

		if (!uri.isLocalFile())
			return KisImageBuilder_RESULT_NOT_LOCAL;

		Magick::readImages(&mimages, uri.path().latin1());

		if (mimages.empty())
			return KisImageBuilder_RESULT_EMPTY;

		img = new KisImage(m_doc, 0, 0, OPACITY_OPAQUE, IMAGE_TYPE_RGBA, m_doc -> nextImageName());

		for (mimglist::iterator it = mimages.begin(); it != mimages.end(); it++) {
			Magick::Image& magick = *it;
			Magick::Geometry geo = magick.size();

			if (geo.width() && geo.height()) {
				KisLayerSP layer = new KisLayer(img, geo.width(), geo.height(), img -> nextLayerName(), OPACITY_OPAQUE);
				KisTileMgrSP tm = layer -> data();
				Q_INT32 w = TILE_WIDTH;
				Q_INT32 h = TILE_HEIGHT;

				img -> add(layer, -1);

				for (Q_INT32 y = 0; y < img -> height(); y += TILE_HEIGHT) {
					if ((y + h) > img -> height())
						h = TILE_HEIGHT + img -> height() - (y + h);

					for (Q_INT32 x = 0; x < img -> width(); x += TILE_WIDTH) {
						if ((x + w) > img -> width())
							w = TILE_WIDTH + img -> width() - (x + w);

						const Magick::PixelPacket *pp = magick.getConstPixels(x, y, w, h);
						KisPixelDataSP pd = tm -> pixelData(x, y, x + w - 1, y + h - 1, TILEMODE_RW);

						if (!pd || !pp)
							return KisImageBuilder_RESULT_FAILURE;

						pp2tile(pd, pp);
						tm -> releasePixelData(pd);
						w = TILE_WIDTH;
					}

					h = TILE_HEIGHT;
				}
			}
		}

		img -> invalidate();
		m_img = img;
	} catch (...) {
		return KisImageBuilder_RESULT_FAILURE;
	}

	return KisImageBuilder_RESULT_OK;
}

KisImageSP KisImageMagickConverter::image()
{
	return m_img;
}

void KisImageMagickConverter::init(KisDoc *doc)
{
	m_doc = doc;
}

KisImageBuilder_Result buildFile(const KURL&, KisImageSP)
{
	return KisImageBuilder_RESULT_UNSUPPORTED;
}

KisImageBuilder_Result KisImageMagickConverter::buildFile(const KURL& uri, KisLayerSP layer)
{
	try {
		Magick::Geometry geo;
		Magick::Image image;
		Q_INT32 w;
		Q_INT32 h;
		KisTileMgrSP tm;

		if (!layer)
			return KisImageBuilder_RESULT_INVALID_ARG;

		if (uri.isEmpty())
			return KisImageBuilder_RESULT_NO_URI;

		if (!uri.isLocalFile())
			return KisImageBuilder_RESULT_NOT_LOCAL;

		geo = Magick::Geometry(layer -> width(), layer -> height());

		if (!geo.width() || !geo.height())
			return KisImageBuilder_RESULT_EMPTY;

		tm = layer -> data();
		image.size(geo);
		image.matte(layer -> alpha());
		w = TILE_WIDTH;
		h = TILE_HEIGHT;

		for (Q_UINT32 y = 0; y < geo.height(); y += TILE_HEIGHT) {
			if ((y + h) > geo.height())
				h = TILE_HEIGHT + geo.height() - (y + h);

			for (Q_UINT32 x = 0; x < geo.width(); x += TILE_WIDTH) {
				if ((x + w) > geo.width())
					w = TILE_WIDTH + geo.width() - (x + w);

				KisPixelDataSP pd = tm -> pixelData(x, y, x + w - 1, y + h - 1, TILEMODE_READ);
				Magick::PixelPacket *pp = image.getPixels(x, y, w, h);

				if (!pd || !pp)
					return KisImageBuilder_RESULT_FAILURE;

				pp2tile(pp, pd);
				image.syncPixels();
				w = TILE_WIDTH;
			}

			h = TILE_HEIGHT;
		}

		image.write(uri.path().latin1());
		return KisImageBuilder_RESULT_OK;
	} catch (...) {
		return KisImageBuilder_RESULT_FAILURE;
	}
}

