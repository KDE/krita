/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

 All compositing code except for the 'OVER' operator is derived from
 GraphicsMagick/magick/composite.c and is subject to the following
 license and copyright:

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
 */

#include "kis_compositeoprgba.h"
#include "kis_strategy_colorspace_rgb.h"

KisCompositeOpRGBAOver::KisCompositeOpRGBAOver() :
	KisCompositeOp("Over")
{ };
void KisCompositeOpRGBAOver::compose(KisPixelRepresentation dst, KisPixelRepresentation src, QUANTUM opacity)
{
	KisPixelRepresentationRGB dstRGB(dst); 
	KisPixelRepresentationRGB srcRGB(src); 
	if (opacity == OPACITY_TRANSPARENT) 
		return;

	if (opacity != OPACITY_OPAQUE) {
		int srcAlpha = (srcRGB.alpha() * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
		int dstAlpha = (dstRGB.alpha() * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;
		dstRGB.red()   = (dstRGB.red()   * dstAlpha + srcRGB.red()   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
		dstRGB.green() = (dstRGB.green() * dstAlpha + srcRGB.green() * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
		dstRGB.blue()  = (dstRGB.blue()  * dstAlpha + srcRGB.blue()  * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
		dstRGB.alpha() = (dstRGB.alpha() * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;
		if (dstRGB.alpha() != 0) {
			dstRGB.red() = (dstRGB.red() * QUANTUM_MAX) / dstRGB.alpha();
			dstRGB.green() = (dstRGB.green() * QUANTUM_MAX) / dstRGB.alpha();
			dstRGB.blue() = (dstRGB.blue() * QUANTUM_MAX) / dstRGB.alpha();
		}
	}
	else {
		if (srcRGB.alpha() == OPACITY_TRANSPARENT)
			return;
		if (dstRGB.alpha() == OPACITY_TRANSPARENT || srcRGB.alpha() == OPACITY_OPAQUE) {
	// 					memcpy(d, s, stride * sizeof(QUANTUM));
			return;
		}
		int srcAlpha = srcRGB.alpha();
		int dstAlpha = (dstRGB.alpha() * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;
		dstRGB.red()   = (dstRGB.red()   * dstAlpha + srcRGB.red()   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
		dstRGB.green() = (dstRGB.green() * dstAlpha + srcRGB.green() * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
		dstRGB.blue()  = (dstRGB.blue()  * dstAlpha + srcRGB.blue()  * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
		dstRGB.alpha() = (dstRGB.alpha() * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;

		if (dstRGB.alpha() != 0) {
			dstRGB.red() = (dstRGB.red() * QUANTUM_MAX) / dstRGB.alpha();
			dstRGB.green() = (dstRGB.green() * QUANTUM_MAX) / dstRGB.alpha();
			dstRGB.blue() = (dstRGB.blue() * QUANTUM_MAX) / dstRGB.alpha();
		}
	}
}
