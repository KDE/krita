/*
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_STRATEGY_COLORSPACE_WET_H_
#define KIS_STRATEGY_COLORSPACE_WET_H_

#include <qcolor.h>

#include <qcolor.h>

#include "kis_global.h"
#include "kis_strategy_colorspace.h"
#include "kis_pixel.h"


/**
 * The wet colourspace is one of the more complicated colour spaces. Every
 * pixel actually consists of two pixels: the paint pixel and the adsorbtion
 * pixel. This corresponds to the two layers of the wetpack structure in the
 * original wetdreams code by Raph Levien.
 */

// XXX: This should really be in a namespace.

typedef struct _WetPix WetPix;
typedef struct _WetPixDbl WetPixDbl;

/*
	* White is made up of myth-red, myth-green, and myth-blue. Myth-red
	* looks red when viewed reflectively, but cyan when viewed
	* transmissively (thus, it vaguely resembles a dichroic
	* filter). Myth-red over black is red, and myth-red over white is
	* white.
	*
	* Total red channel concentration is myth-red concentration plus
	* cyan concentration.
	*/

struct _WetPix {
	Q_UINT16 rd;  /*  Total red channel concentration */
	Q_UINT16 rw;  /*  Myth-red concentration */

	Q_UINT16 gd;  /*  Total green channel concentration */
	Q_UINT16 gw;  /*  Myth-green concentration */

	Q_UINT16 bd;  /*  Total blue channel concentration */
	Q_UINT16 bw;  /*  Myth-blue concentration */

	Q_UINT16 w;   /*  Water volume */
	Q_UINT16 h;   /*  Height of paper surface */
};


struct _WetPixDbl {
	double rd;  /*  Total red channel concentration */
	double rw;  /*  Myth-red concentration */
	double gd;  /*  Total green channel concentration */
	double gw;  /*  Myth-green concentration */
	double bd;  /*  Total blue channel concentration */
	double bw;  /*  Myth-blue concentration */
	double w;   /*  Water volume */
	double h;   /*  Height of paper surface */
};


void wetPixToDouble(WetPixDbl * dst, WetPix *src);
void wetPixFromDouble(WetPix * dst, WetPixDbl *src);


class KisColorSpaceWet : public KisStrategyColorSpace {
public:
	KisColorSpaceWet();
	virtual ~KisColorSpaceWet();

public:

	virtual void nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP profile = 0);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP profile = 0);

	virtual void toQColor(const QUANTUM *src, QColor *c, KisProfileSP profile = 0);
	virtual void toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

	virtual KisPixelRO toKisPixelRO(const QUANTUM *src, KisProfileSP profile = 0)
		{ return 0; };

	virtual KisPixel toKisPixel(QUANTUM *src, KisProfileSP profile = 0)
		{ return 0; };

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

private:

	// This was static, but since we have only one instance of the color strategy,
	// it can be just as well a private member variable.
	void wet_init_render_tab();

	// Convert a single pixel from its wet representation to rgb
	void wet_composite(Q_UINT8 *rgb, WetPix * wet);

private:
	vKisChannelInfoSP m_channels;
	Q_UINT32 * wet_render_tab;


};

#endif // KIS_STRATEGY_COLORSPACE_WET_H_
