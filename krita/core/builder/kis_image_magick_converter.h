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
#if !defined KIS_IMAGE_MAGICK_CONVERTER_H_
#define KIS_IMAGE_MAGICK_CONVERTER_H_

#include <qobject.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_image_builder.h"

class QString;
class KURL;
class KisDoc;
class KisNameServer;

/**
 * Build a KisImage representation of an image file.
 */
class KisImageMagickConverter {
public:
	KisImageMagickConverter(KisDoc *doc);
	virtual ~KisImageMagickConverter();

public:
	KisImageBuilder_Result buildImage(const KURL& uri);
	KisImageBuilder_Result buildFile(const KURL& uri, KisImageSP img);
	KisImageBuilder_Result buildFile(const KURL& uri, KisLayerSP layer);
	KisImageSP image();

private:
	KisImageMagickConverter(const KisImageMagickConverter&);
	KisImageMagickConverter& operator=(const KisImageMagickConverter&);
	void init(KisDoc *doc);

private:
	KisImageSP m_img;
	KisDoc *m_doc;
};

#endif // KIS_IMAGE_MAGICK_CONVERTER_H_

