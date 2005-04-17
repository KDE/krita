/*
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
#ifndef KIS_COLORSPACE_WET_STICKY_H_
#define KIS_COLORSPACE_WET_STICKY_H_

#include <qcolor.h>

#include <qcolor.h>

#include "kis_global.h"
#include "kis_strategy_colorspace.h"
#include "kis_pixel.h"

namespace WetAndSticky {

	/**
         * A color is specified as a vector in HLS space.  Hue is a value
	 * in the range 0..360 degrees with 0 degrees being red.  Saturation
         * and Lightness are both in the range [0,1].  A lightness of 0 means
	 * black, with 1 being white.  A totally saturated color has saturation
	 * of 1.
	 */

	typedef struct hls_color {float hue, saturation, lightness; }
		HLS_COLOR;

	typedef struct rgb_color {Q_UINT8 red; Q_UINT8 green; Q_UINT8 blue;}
		RGB_COLOR;


	enum enumDirection {
		NORTH,
		EAST,
		SOUTH,
		WEST
	};


	typedef struct paint {
		HLS_COLOR color;
		Q_UINT8        liquid_content;  /*  [0,100].  */
		Q_UINT8        drying_rate;     /*  [0,100].  */
		Q_UINT8        miscibility;     /*  [0,inf].  */
	} PAINT, *PAINT_PTR;


	typedef struct rgba {
		RGB_COLOR color;
		Q_UINT8 alpha;
	} RGBA, *RGBA_PTR;


	/**
	 * Defines the strength and direction of gravity for a cell.
	 */
	typedef struct gravity {
		enumDirection  direction;
		Q_UINT8    strength;     /*  [0,Infinity). */
	} GRAVITY, *GRAVITY_PTR;



	/**
	 * Defines the contents and attributes of a cell on the canvas.
	 */
	typedef struct cell {
		RGBA     representation; /* The on-screen representation of this cell. */
		PAINT    contents;    /* The paint in this cell. */
		GRAVITY  gravity;     /* This cell's gravity.  */
		Q_UINT8  absorbancy;  /* How much paint can this cell hold? */
		Q_UINT8  volume;      /* The volume of paint. */
	} CELL, *CELL_PTR;


}



class KisColorSpaceWetSticky : public KisStrategyColorSpace {
public:
	KisColorSpaceWetSticky();
	virtual ~KisColorSpaceWetSticky();

public:



	virtual void nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP profile = 0);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP profile = 0);

	virtual void toQColor(const QUANTUM *src, QColor *c, KisProfileSP profile = 0);
	virtual void toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

	virtual KisPixelRO toKisPixelRO(const QUANTUM *src, KisProfileSP profile = 0);

	virtual KisPixel toKisPixel(QUANTUM *src, KisProfileSP profile = 0);

	virtual vKisChannelInfoSP channels() const;
	virtual bool alpha() const;
	virtual Q_INT32 nChannels() const;
	virtual Q_INT32 nColorChannels() const;
	virtual Q_INT32 nSubstanceChannels() const;
	virtual Q_INT32 pixelSize() const;

	virtual QImage convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent = INTENT_PERCEPTUAL);


	virtual void bitBlt(Q_INT32 stride,
			    QUANTUM *dst,
			    Q_INT32 dststride,
			    const QUANTUM *src,
			    Q_INT32 srcstride,
			    QUANTUM opacity,
			    Q_INT32 rows,
			    Q_INT32 cols,
			    CompositeOp op);

protected:
	virtual bool convertPixelsTo(const QUANTUM * src, KisProfileSP srcProfile,
				     QUANTUM * dst, KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile,
				     Q_UINT32 numPixels,
				     Q_INT32 renderingIntent = INTENT_PERCEPTUAL);


private:
	vKisChannelInfoSP m_channels;
};

#endif // KIS_COLORSPACE_WET_STICKY_H_
