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
#ifndef KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
#define KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_

#include <qcolor.h>

#include <koColor.h>

#include "kis_global.h"
#include "kis_strategy_colorspace.h"
#include "kis_pixel.h"

// XXX: move into namespace.
const PIXELTYPE PIXEL_GRAY = 0;
const PIXELTYPE PIXEL_GRAY_ALPHA = 1;


class KisStrategyColorSpaceGrayscale : public KisStrategyColorSpace {
public:
	KisStrategyColorSpaceGrayscale(bool alpha);
	virtual ~KisStrategyColorSpaceGrayscale();

public:

	virtual void nativeColor(const KoColor& c, QUANTUM *dst);
	virtual void nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst);

	virtual void toKoColor(const QUANTUM *src, KoColor *c);
	virtual void toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity);

	virtual KisPixelRO toKisPixelRO(QUANTUM *src) { return KisPixelRO (src, src + PIXEL_GRAY_ALPHA); }
	virtual KisPixel toKisPixel(QUANTUM *src) { return KisPixel (src, src + PIXEL_GRAY_ALPHA); }

	virtual vKisChannelInfoSP channels() const;
	virtual bool alpha() const;
	virtual Q_INT32 depth() const;
	virtual Q_INT32 nColorChannels() const;
	
	virtual QImage convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride) const;


	virtual void bitBlt(Q_INT32 stride,
			    QUANTUM *dst, 
			    Q_INT32 dststride,
			    QUANTUM *src, 
			    Q_INT32 srcstride,
			    QUANTUM opacity,
			    Q_INT32 rows, 
			    Q_INT32 cols, 
			    CompositeOp op);

private:
	vKisChannelInfoSP m_channels;
	bool m_alpha;
};

#endif // KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
