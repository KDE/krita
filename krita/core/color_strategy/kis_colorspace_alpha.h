/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_COLORSPACE_ALPHA_H_
#define KIS_COLORSPACE_ALPHA_H_

#include <qcolor.h>

#include <koColor.h>

#include "kis_global.h"
#include "kis_strategy_colorspace.h"
#include "kis_pixel.h"

const PIXELTYPE PIXEL_MASK = 0;

/**
 * The alpha mask is a special color strategy that treats all pixels as 
 * alpha value with a colour common to the mask. The default color is white.
 */
class KisColorSpaceAlpha : public KisStrategyColorSpace {
public:
	KisColorSpaceAlpha();
	virtual ~KisColorSpaceAlpha();

public:
	virtual void nativeColor(const KoColor& c, QUANTUM *dst);
	virtual void nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst);
	
	virtual void toKoColor(const QUANTUM *src, KoColor *c);
	virtual void toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity);

	virtual KisPixelRO toKisPixelRO(QUANTUM *src, KisProfileSP profile = 0) { return KisPixelRO (src, src, this, profile); }
	virtual KisPixel toKisPixel(QUANTUM *src, KisProfileSP profile = 0) { return KisPixel (src, src, this, profile); }

	virtual vKisChannelInfoSP channels() const;
	virtual bool alpha() const;
	virtual Q_INT32 depth() const;
	virtual Q_INT32 nColorChannels() const { return 0; };
	virtual QImage convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, 
				       KisProfileSP srcProfile, KisProfileSP dstProfile, 
				       Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

	virtual void setMaskColor(KoColor c) { m_maskColor = c; }
	virtual void setInverted(bool b) { m_inverted = b; }

protected:

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

	KoColor m_maskColor;
	bool m_inverted;
};

#endif // KIS_COLORSPACE_ALPHA_H_
