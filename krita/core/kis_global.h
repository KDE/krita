/*
 *  kis_global.h - part of KImageShop
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
#if !defined KISGLOBAL_H_
#define KISGLOBAL_H_

#include <limits.h>
#include <qglobal.h>
#include <kglobal.h>
#include <koGlobal.h>
#include <koUnit.h>

/**
 * Mime type for this app - not same as file type, but file types
 * can be associated with a mime type and are opened with applications
 * associated with the same mime type
 */
#define APP_MIMETYPE "application/x-krita"


/**
 * Default size for a tile,  Usually, all
 * tiles are sqr(TILE_SIZE).  Only tiles
 * on the edge of an canvas are exempt from
 * this rule.
 */
const int TILE_SIZE = 64;

/**
 * Default width of a tile.
 */
const int TILE_WIDTH = TILE_SIZE;

/**
 * Default height of a tile.
 */
const int TILE_HEIGHT = TILE_SIZE;

/**
 * Size of a quantum
 */
typedef Q_UINT16 QUANTUM;

const QUANTUM QUANTUM_MAX = USHRT_MAX;
const QUANTUM OPACITY_TRANSPARENT = 0;
const QUANTUM OPACITY_OPAQUE = QUANTUM_MAX;
const QUANTUM MAXCHANNELS = 5;

enum enumComposite {
	COMPOSITE_UNDEF,
	COMPOSITE_OVER,
	COMPOSITE_IN,
	COMPOSITE_ATOP,
	COMPOSITE_XOR,
	COMPOSITE_PLUS,
	COMPOSITE_MINUS,
	COMPOSITE_ADD,
	COMPOSITE_SUBTRACT,
	COMPOSITE_DIFF,
	COMPOSITE_MULT,
	COMPOSITE_BUMPMAP,
	COMPOSITE_COPY,
	COMPOSITE_COPY_RED,
	COMPOSITE_COPY_GREEN,
	COMPOSITE_COPY_BLUE,
	COMPOSITE_COPY_OPACITY,
	COMPOSITE_CLEAR,
	COMPOSITE_DISSOLVE,
	COMPOSITE_DISPLACE,
	COMPOSITE_MODULATE,
	COMPOSITE_THRESHOLD
};

enum enumImgType { 
	IMAGE_TYPE_UNKNOWN,
	IMAGE_TYPE_INDEXED,
	IMAGE_TYPE_INDEXEDA,
	IMAGE_TYPE_GREY,
	IMAGE_TYPE_GREYA,
	IMAGE_TYPE_RGB,
	IMAGE_TYPE_RGBA,
	IMAGE_TYPE_CMYK,
	IMAGE_TYPE_CMYKA,
	IMAGE_TYPE_LAB,
	IMAGE_TYPE_LABA,
	IMAGE_TYPE_YUV,
	IMAGE_TYPE_YUVA };

typedef Q_UINT8 CHANNELTYPE;
typedef Q_UINT8 PIXELTYPE;

const CHANNELTYPE REDCHANNEL = 0;
const CHANNELTYPE GREENCHANNEL = 1;
const CHANNELTYPE BLUECHANNEL = 2;
const CHANNELTYPE GRAYCHANNEL = 3;
const CHANNELTYPE INDEXEDCHANNEL = 4;
const CHANNELTYPE ALPHACHANNEL = 5;

const PIXELTYPE PIXEL_UNDEF = 255;
const PIXELTYPE PIXEL_GRAY = 0;
const PIXELTYPE PIXEL_GRAY_ALPHA = 1;
const PIXELTYPE PIXEL_RED = 0;
const PIXELTYPE PIXEL_GREEN = 1;
const PIXELTYPE PIXEL_BLUE = 2;
const PIXELTYPE PIXEL_ALPHA = 3;
const PIXELTYPE PIXEL_INDEXED = 0;
const PIXELTYPE PIXEL_INDEXED_ALPHA = 1;

const Q_INT32 IMG_DEFAULT_WIDTH = 512;
const Q_INT32 IMG_DEFAULT_HEIGHT = 512;
const Q_INT32 IMG_DEFAULT_DEPTH = 4;

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

#define downscale(quantum)  ((unsigned char) ((quantum)/257UL))
#define upscale(value)  ((QUANTUM) (257UL*(value)))

#endif // KISGLOBAL_H_

