/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <kis_types.h>
#include <kis_global.h>
#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_undo_adapter.h>
#include <kis_colorspace_registry.h>
#include <kis_iterators_pixel.h>
#include <kis_profile.h>
#include <kis_strategy_colorspace.h>
#include "kis_magick_converter.h"
#include "../../../config.h"


namespace {

	const PIXELTYPE PIXEL_BLUE = 0;
	const PIXELTYPE PIXEL_GREEN = 1;
	const PIXELTYPE PIXEL_RED = 2;
	const PIXELTYPE PIXEL_ALPHA = 3;

}

KisMagickConverter::KisMagickConverter()
{
	m_stop = false;
}

KisMagickConverter::~KisMagickConverter()
{
}


KisPaintDeviceSP KisMagickConverter::fromMagickImage(Image * image, const QString & name)
{
	return 0;
}

Image * KisMagickConverter::toMagickImage(KisPaintDeviceSP device)
{
	return 0;
}

void KisMagickConverter::cancel()
{
	m_stop = true;
}

#include "kis_magick_converter.moc"

