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
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <koColor.h>
#include <Magick++.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_nameserver.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kispixeldata.h"
#include "kis_layer.h"
#include "kis_image_builder.h"

typedef QValueList<Magick::Image> mimglist;

class KisImageBuilderPriv {
public:
	KisImageSP image;
	KURL uri;
	KisDoc *doc;
};

namespace {
	inline
	void pp2tile(KisPixelDataSP pd, const Magick::PixelPacket *pp)
	{
		register Q_INT32 i;
		register Q_INT32 j;
		register QUANTUM *pixel = pd -> data;

		for (j = 0; j < pd -> height; j++) {
			for (i = 0; i < pd -> width; i++) {
				pixel[PIXEL_RED] = pp -> red;
				pixel[PIXEL_GREEN] = pp -> green;
				pixel[PIXEL_BLUE] = pp -> blue;
				pixel[PIXEL_ALPHA] = pp -> opacity;
				pixel += pd -> depth;
				pp++;
			}
		}
	}
}

KisImageBuilder::KisImageBuilder(KisDoc *doc, const QString& filename)
{
	KURL uri(filename);

	init(doc, uri);
}

KisImageBuilder::KisImageBuilder(KisDoc *doc, const KURL& uri)
{
	init(doc, uri);
}

KisImageBuilder::~KisImageBuilder()
{
	delete m_members;
	delete m_nserver;
}

KisImageBuilder_Result KisImageBuilder::buildImage()
{
	KURL uri = m_members -> uri;
	mimglist mimages;
	KisImageSP img;

	if (uri.isEmpty())
		return KisImageBuilder_RESULT_NO_URI;

	if (!KIO::NetAccess::exists(uri))
		return KisImageBuilder_RESULT_NOT_EXIST;

	if (!uri.isLocalFile())
		return KisImageBuilder_RESULT_NOT_LOCAL;

	if (uri.isLocalFile()) {
		Magick::readImages(&mimages, uri.path().latin1());
	} else {
		KTempFile tf;
		QString tmpname = tf.name();

		if (!KIO::NetAccess::download(uri, tmpname))
			return KisImageBuilder_RESULT_BAD_FETCH;

		Magick::readImages(&mimages, tmpname.latin1());
	}

	if (mimages.empty())
		return KisImageBuilder_RESULT_EMPTY;

	img = new KisImage(m_members -> doc, 0, 0, 4, 0, IMAGE_TYPE_RGBA, m_members -> doc -> nextImageName());

	for (mimglist::iterator it = mimages.begin(); it != mimages.end(); it++) {
		Magick::Image& magick = *it;
		Magick::Geometry geo = magick.size();

		if (geo.width() && geo.height()) {
			KisLayerSP layer = new KisLayer(img, geo.width(), geo.height(), m_nserver -> name(), 0);
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

					pp2tile(pd, pp);
					tm -> releasePixelData(pd);
					w = TILE_WIDTH;
				}

				h = TILE_HEIGHT;
			}
		}
	}

	img -> invalidate();
	m_members -> image = img;
	return KisImageBuilder_RESULT_OK;
}

KisImageSP KisImageBuilder::image()
{
	return m_members -> image;
}

void KisImageBuilder::init(KisDoc *doc, const KURL& uri)
{
	m_nserver = new KisNameServer("Layer %1", 1);
	m_members = new KisImageBuilderPriv;
	m_members -> uri = uri;
	m_members -> doc = doc;
}

