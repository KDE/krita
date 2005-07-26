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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_STRATEGY_COLORSPACE_WET_H_
#define KIS_STRATEGY_COLORSPACE_WET_H_

#include <qcolor.h>
#include <qstringlist.h>
#include <qmap.h>

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
typedef struct _WetPack WetPack;

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
	Q_UINT16 h;   /*  Height of paper surface XXX: This might just as well be a single
			  channel in our colour model that has two of
			  these wetpix structs for every paint device pixels*/
};

struct _WetPack {
	WetPix paint;      /* Paint layer */
	WetPix adsorb; /* Adsorbtion layer */
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

	// Semi-clever: we have only fifteen wet paint colors that are mapped to the
	// qcolors that are put in the painter by the special wet paint palette. Other
	// QColors are mapped to plain water...
	virtual void nativeColor(const QColor& c, Q_UINT8 *dst, KisProfileSP profile = 0);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP profile = 0);

	virtual void getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha);

	virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP profile = 0);
	virtual void toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

	virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src, KisProfileSP profile = 0)
		{ return 0; };

	virtual KisPixel toKisPixel(Q_UINT8 *src, KisProfileSP profile = 0)
		{ return 0; };
	
	virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;
	
	virtual vKisChannelInfoSP channels() const;
	virtual bool hasAlpha() const;
	virtual Q_INT32 nChannels() const;
	virtual Q_INT32 nColorChannels() const;
	virtual Q_INT32 nSubstanceChannels() const;
	virtual Q_INT32 pixelSize() const;

	virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;
	virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;

	virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent = INTENT_PERCEPTUAL,
					   KisRenderInformationSP renderInfo = 0);

	virtual void adjustBrightness(Q_UINT8 *src1, Q_INT8 adjust) const;

	virtual void adjustBrightnessContrast(const Q_UINT8*, Q_UINT8*, Q_INT8, Q_INT8, Q_INT32) const { /* XXX implement? */}

	virtual KisCompositeOpList userVisiblecompositeOps() const;
	
	void setPaintWetness(bool b) { m_paintwetness = b; } // XXX this needs better design!
	bool paintWetness() { return m_paintwetness; }
	void resetPhase() { phase = phasebig++; phasebig &= 3; }
protected:
	virtual void bitBlt(Q_UINT8 *dst,
			Q_INT32 dstRowSize,
			const Q_UINT8 *src,
			Q_INT32 srcRowStride,
			const Q_UINT8 *srcAlphaMask,
			Q_INT32 maskRowStride,
			QUANTUM opacity,
			Q_INT32 rows,
			Q_INT32 cols,
			const KisCompositeOp& op);
private:

	// This was static, but since we have only one instance of the color strategy,
	// it can be just as well a private member variable.
	void wet_init_render_tab();

	// Convert a single pixel from its wet representation to rgb
	void wet_composite(Q_UINT8 *rgb, WetPix * wet);

	void wet_render_wetness(Q_UINT8 * rgb, WetPack * pack);

private:
	vKisChannelInfoSP m_channels;
	Q_UINT32 * wet_render_tab;

	QStringList m_paintNames;
	QMap<QRgb, WetPix> m_conversionMap;
	
	bool m_paintwetness;
	int phase, phasebig;

};

#endif // KIS_STRATEGY_COLORSPACE_WET_H_
