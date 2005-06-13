/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *   02111-1307, USA.
 *

 Some code is derived from GraphicsMagick/magick/composite.c and is 
 subject to the following license and copyright:

  Copyright (C) 2002 GraphicsMagick Group, an organization dedicated
  to making software imaging solutions freely available.

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files
  ("GraphicsMagick"), to deal in GraphicsMagick without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of GraphicsMagick,
  and to permit persons to whom GraphicsMagick is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of GraphicsMagick.

  The software is provided "as is", without warranty of any kind, express
  or implied, including but not limited to the warranties of
  merchantability, fitness for a particular purpose and noninfringement.
  In no event shall GraphicsMagick Group be liable for any claim,
  damages or other liability, whether in an action of contract, tort or
  otherwise, arising from, out of or in connection with GraphicsMagick
  or the use or other dealings in GraphicsMagick.

  Except as contained in this notice, the name of the GraphicsMagick
  Group shall not be used in advertising or otherwise to promote the
  sale, use or other dealings in GraphicsMagick without prior written
  authorization from the GraphicsMagick Group.

 Other code is derived from gwenview/src/qxcfi.* - this is released under
  the terms of the LGPL

 */

#ifndef COMPOSITE_H_
#define COMPOSITE_H_

#include <kdebug.h>

#include <kis_global.h>

/**
 * Image composition functions that can be used by the colour strategies. 
 *
 * XXX: perhaps each composition function ought to be a strategy of itself.
 * Krita is still missing something like a capabilities database that ties 
 * together image formats, colour systems, composition functions etc., that
 * determines which goes with which and defines user visible text for all this.
 * 
 * For now, this is a quick hack; once things are working again, I'll investigate
 * doing this nicely (famous last words...)
 *
 * XXX: Except for Over, none of the operators uses the opacity parameter
 */


// Straight from image.h

#define PixelIntensity(pixel) ((unsigned int) \
   (((double)306.0 * (pixel[PIXEL_RED]) + \
     (double)601.0 * (pixel[PIXEL_GREEN]) + \
     (double)117.0 * (pixel[PIXEL_BLUE)) \
    / 1024.0))

#define PixelIntensityToQuantum(pixel) ((QUANTUM)PixelIntensity(pixel))

#define PixelIntensityToDouble(pixel) ((double)PixelIntensity(pixel))

#define RoundSignedToQuantum(value) ((QUANTUM) (value < 0 ? 0 : \
  (value > QUANTUM_MAX) ? QUANTUM_MAX : value + 0.5))

#define RoundToQuantum(value) ((QUANTUM) (value > QUANTUM_MAX ? QUANTUM_MAX : \
  value + 0.5))

// And from studio.h
#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))

void compositeCopy(Q_INT32 pixelSize,
		   Q_UINT8 *dst, 
		   Q_INT32 dstRowSize,
		   const Q_UINT8 *src, 
		   Q_INT32 srcRowSize,
		   Q_INT32 rows, 
		   Q_INT32 cols, 
		   QUANTUM /*opacity*/ = OPACITY_OPAQUE)
{
	Q_INT32 linesize = pixelSize * cols;
	Q_UINT8 *d;
	const Q_UINT8 *s;
	d = dst;
	s = src;
	
	while (rows-- > 0) {
		memcpy(d, s, linesize);
		d += dstRowSize;
		s += srcRowSize;
	}
}

#endif

