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
#if !defined KIS_STRATEGY_COLORSPACE_RGB_H_
#define KIS_STRATEGY_COLORSPACE_RGB_H_

#include <qcolor.h>
#include <qpixmap.h>
#include <kpixmapio.h>
#include <koColor.h>
#include "kis_global.h"
#include "kis_strategy_colorspace.h"

class KisStrategyColorSpaceRGB : public KisStrategyColorSpace {
public:
	KisStrategyColorSpaceRGB();
	virtual ~KisStrategyColorSpaceRGB();

public:
	virtual void nativeColor(const KoColor& c, QUANTUM *dst);
	virtual void nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst);
	virtual void nativeColor(const QColor& c, QUANTUM *dst);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst);
	virtual void nativeColor(QRgb rgb, QUANTUM *dst);
	virtual void nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst);
	
	virtual ChannelInfo* channelsInfo() const;
	
	virtual void render(KisImageSP image, QPainter& painter, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height);

	virtual QImage convertToImage(KisImageSP image, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const;
	virtual QImage convertToImage(KisTileMgrSP tm, Q_UINT32 depth, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const;

	virtual void tileBlt(Q_INT32 stride,
			QUANTUM *dst, 
			Q_INT32 dststride,
			QUANTUM *src, 
			Q_INT32 srcstride,
			Q_INT32 rows, 
			Q_INT32 cols, 
			CompositeOp op) const;
	virtual void tileBlt(Q_INT32 stride,
			QUANTUM *dst, 
			Q_INT32 dststride,
			QUANTUM *src, 
			Q_INT32 srcstride,
			QUANTUM opacity,
			Q_INT32 rows, 
			Q_INT32 cols, 
			CompositeOp op) const;
	virtual void computeDuplicatePixel(KisIteratorQuantum* dst, KisIteratorQuantum* dab, KisIteratorQuantum* src);

private:
	KPixmapIO m_pixio;
	QPixmap m_pixmap;
	QUANTUM *m_buf;

	static ChannelInfo channelInfo[3];
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_
