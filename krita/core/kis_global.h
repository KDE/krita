/*
 *  kis_global.h - part of KImageShop
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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

#ifndef __kis_global_h__
#define __kis_global_h__

#include <qvaluevector.h>
#include <ksharedptr.h>

class KisTool;

const int CHANNEL_MAX = 255;

// size for graphic blocks - must be a power of 2
const int TILE_SIZE = 64;

// maximal number of channels
const int MAX_CHANNELS = 8;

enum ActiveColor { ac_Foreground, ac_Background};

// color spaces
enum cSpace { cs_Indexed, cs_RGB, cs_HSV, cs_CMYK, cs_Lab };

// color modes
enum cMode { cm_Indexed, cm_Greyscale, cm_RGB, cm_RGBA, cm_CMYK, cm_CMYKA, cm_Lab, cm_LabA };

// channel id's
enum cId { ci_Indexed, ci_Alpha, ci_Red, ci_Green, ci_Blue, ci_Cyan,
		   ci_Magenta, ci_Yellow, ci_Black, ci_L, ci_a, ci_b };

// background mode
enum bgMode {bm_White, bm_Transparent, bm_BackgroundColor, bm_ForegroundColor };

typedef KSharedPtr<KisTool> KisToolSP;
typedef QValueVector<KisToolSP> ktvector;
typedef ktvector::size_type ktvector_size_type;

#endif
