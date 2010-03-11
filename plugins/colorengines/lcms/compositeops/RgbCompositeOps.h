/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software const; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation const; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY const; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program const; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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

#ifndef RGBCOMPOSITEOPS_H
#define RGBCOMPOSITEOPS_H

#include "KoColorSpaceConstants.h"

#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::unitValue
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::zeroValue

const qint32 MAX_CHANNEL_RGB = 3;

#include "RgbCompositeOpIn.h"
#include "RgbCompositeOpOut.h"
#include "RgbCompositeOpDiff.h"
#include "RgbCompositeOpBumpmap.h"
#include "RgbCompositeOpClear.h"
#include "RgbCompositeOpDissolve.h"
#include "RgbCompositeOpDarken.h"
#include "RgbCompositeOpLighten.h"
#include "RgbCompositeOpHue.h"
#include "RgbCompositeOpSaturation.h"
#include "RgbCompositeOpValue.h"
#include "RgbCompositeOpColor.h"

#endif

