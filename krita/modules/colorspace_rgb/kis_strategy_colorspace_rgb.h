/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
#ifndef KIS_STRATEGY_COLORSPACE_RGB_H_
#define KIS_STRATEGY_COLORSPACE_RGB_H_

#include <qcolor.h>

#include <qcolor.h>

#include "kis_global.h"
#include "kis_strategy_colorspace.h"
#include "kis_pixel.h"

const PIXELTYPE PIXEL_BLUE = 0;
const PIXELTYPE PIXEL_GREEN = 1;
const PIXELTYPE PIXEL_RED = 2;
const PIXELTYPE PIXEL_ALPHA = 3;

class KisStrategyColorSpaceRGB : public KisStrategyColorSpace {
public:
	KisStrategyColorSpaceRGB();
	virtual ~KisStrategyColorSpaceRGB();

public:
	virtual void nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP profile = 0);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP profile = 0);

	virtual void toQColor(const QUANTUM *src, QColor *c, KisProfileSP profile = 0);
	virtual void toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

	virtual KisPixelRO toKisPixelRO(const QUANTUM *src, KisProfileSP profile = 0)
		{ return KisPixelRO (src, src + PIXEL_ALPHA, this, profile); }
	virtual KisPixel toKisPixel(QUANTUM *src, KisProfileSP profile = 0)
		{ return KisPixel (src, src + PIXEL_ALPHA, this, profile); }

	virtual Q_INT8 difference(const QUANTUM* src1, const QUANTUM* src2);

	virtual vKisChannelInfoSP channels() const;
	virtual bool alpha() const;
	virtual Q_INT32 nChannels() const;
	virtual Q_INT32 nColorChannels() const;
	virtual Q_INT32 pixelSize() const;

	virtual QImage convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent);

	virtual KisCompositeOpList userVisiblecompositeOps() const;

protected:

	virtual void bitBlt(Q_INT32 pixelSize,
			    QUANTUM *dst,
			    Q_INT32 dstRowStride,
			    const QUANTUM *src,
			    Q_INT32 srcRowStride,
			    QUANTUM opacity,
			    Q_INT32 rows,
			    Q_INT32 cols,
			    const KisCompositeOp& op);

	void compositeOver(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeMultiply(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeDivide(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeScreen(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeOverlay(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeDodge(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeBurn(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeDarken(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeLighten(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeHue(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeSaturation(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeValue(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeColor(QUANTUM *dst, Q_INT32 dstRowStride, const QUANTUM *src, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);

private:
	vKisChannelInfoSP m_channels;


};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_
