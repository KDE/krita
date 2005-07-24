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
#ifndef KIS_STRATEGY_COLORSPACE_TEMPLATESCALE_H_
#define KIS_STRATEGY_COLORSPACE_TEMPLATESCALE_H_

#include <qcolor.h>

#include "kis_global.h"
#include "kis_strategy_colorspace.h"
#include "kis_pixel.h"

class KisColorSpaceTemplate : public KisStrategyColorSpace {
public:
	KisColorSpaceTemplate();
	virtual ~KisColorSpaceTemplate();

public:

	virtual void nativeColor(const QColor& c, Q_UINT8 *dst, KisProfileSP profile = 0);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP profile = 0);

	virtual void getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha);

	virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP profile = 0);
	virtual void toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

	virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src, KisProfileSP profile = 0)
		{ return KisPixelRO (src, src + PIXEL_TEMPLATE_ALPHA, this, profile); }

	virtual KisPixel toKisPixel(Q_UINT8 *src, KisProfileSP profile = 0)
		{ return KisPixel (src, src + PIXEL_TEMPLATE_ALPHA, this, profile); }

	virtual Q_INT8 difference(const Q_UINT8* src1, const Q_UINT8* src2);
	virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

	virtual vKisChannelInfoSP channels() const;
	virtual bool hasAlpha() const;
	virtual Q_INT32 nChannels() const;
	virtual Q_INT32 nColorChannels() const;
	virtual Q_INT32 pixelSize() const;

	virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;
	virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;

	virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent = INTENT_PERCEPTUAL,
				       float exposure = 0.0f);

	virtual void adjustBrightness(Q_UINT8 *src1, Q_INT8 adjust) const;

	virtual void bitBlt(Q_INT32 stride,
			    Q_UINT8 *dst,
			    Q_INT32 dststride,
			    const Q_UINT8 *src,
			    Q_INT32 srcstride,
			    QUANTUM opacity,
			    Q_INT32 rows,
			    Q_INT32 cols,
			    const KisCompositeOp& op);

	KisCompositeOpList userVisiblecompositeOps() const;

protected:
	void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeMultiply(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeDivide(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeScreen(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeOverlay(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeDodge(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeBurn(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeDarken(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeLighten(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);

private:
	vKisChannelInfoSP m_channels;

	static const Q_UINT8 PIXEL_TEMPLATE = 0;
	static const Q_UINT8 PIXEL_TEMPLATE_ALPHA = 1;
};

#endif // KIS_STRATEGY_COLORSPACE_TEMPLATESCALE_H_
