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
#include <kdebug.h>
#include "kis_global.h"

#if 0
Q_INT32 imgTypeDepth(const enumImgType& type)
{
	Q_INT32 n;

	Q_ASSERT(type != IMAGE_TYPE_UNKNOWN);

	switch (type) {
		case IMAGE_TYPE_INDEXED:
		case IMAGE_TYPE_GREY:
			n = 1;
			break;
		case IMAGE_TYPE_INDEXEDA:
		case IMAGE_TYPE_GREYA:
			n = 2;
			break;
		case IMAGE_TYPE_RGB:
		case IMAGE_TYPE_LAB:
		case IMAGE_TYPE_YUV:
			n = 3;
			break;
		case IMAGE_TYPE_RGBA:
		case IMAGE_TYPE_LABA:
		case IMAGE_TYPE_YUVA:
		case IMAGE_TYPE_CMYK:
			n = 4;
			break;
		case IMAGE_TYPE_CMYKA:
			n = 5;
			break;
		default:
			kdDebug() << "depthForType: Unknow image type.\n";
			n = -1;
	}

	return n;
}

bool imgTypeHasAlpha(const enumImgType& type)
{
	Q_ASSERT(type != IMAGE_TYPE_UNKNOWN);
	return static_cast<Q_INT32>(type) % 2 == 0;
}
#endif
