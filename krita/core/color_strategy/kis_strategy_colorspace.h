/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
#ifndef KIS_STRATEGY_COLORSPACE_H_
#define KIS_STRATEGY_COLORSPACE_H_

#include <map>
#include <qcolor.h>

#include <ksharedptr.h>
#include <koColor.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_channelinfo.h"

class QPainter;
class KisIteratorPixel;
class KisPixelRepresentation;
class KisPixelRepresentationRGB;

class KisStrategyColorSpace : public KShared {


public:
	KisStrategyColorSpace(const QString& name);
	virtual ~KisStrategyColorSpace();

public:
        // The nativeColor methods take a given color that can be defined in any
        // colorspace and fills a byte array with the corresponding color in the
        // the colorspace managed by this strategy. 
	virtual void nativeColor(const KoColor& c, QUANTUM *dst) = 0;
	virtual void nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst) = 0;
	virtual void nativeColor(const QColor& c, QUANTUM *dst) = 0;
	virtual void nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst) = 0;
	virtual void nativeColor(QRgb rgb, QUANTUM *dst) = 0;
	virtual void nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst) = 0;

	virtual void toKoColor(const QUANTUM *src, KoColor *c) = 0;
	virtual void toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity) = 0;

	// XXX: make this a proper vector. Pointers to arrays are _so_ seventies, and
	// Stroustrup assures us a vector is as effecient a mem array anyway.
	virtual ChannelInfo * channelsInfo() const = 0;

	virtual Q_INT32 depth() const = 0;
	virtual bool alpha() const = 0;
	inline QString name() const { return m_name; };

	/**
	 * This function is used to convert a KisPixelRepresentation to another color strategy.
	 * When implementing a color space, there is no need to implement a conversion to all strategies,
	 * if there is no direct conversion facilities, the function should use the conversion to/from RGBA
	 *
	 * XXX: bsar. RGBA is a bad choice for an intermediate format. Use koColor; which can be expanded to use
	 * littleCms.
	 */
	virtual void convertTo(KisPixelRepresentation& src, KisPixelRepresentation& dst,  KisStrategyColorSpaceSP cs);


	// XXX: convertToRGBA and convertFromRGBA must use LAB or XYZ; furthermore, they should
	// use koColor for now, and littlecms later. XXX2: Now that we have toKoColor, these are deprecated

	/** This function converts a pixel to RGBA */
	virtual void convertToRGBA(KisPixelRepresentation& src, KisPixelRepresentationRGB& dst) KDE_DEPRECATED = 0;

	/** This function converts a pixel from RGBA */
	virtual void convertFromRGBA(KisPixelRepresentationRGB& src, KisPixelRepresentation& dst) KDE_DEPRECATED = 0;
	
	virtual QImage convertToImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride) const = 0;

	
	virtual void computeDuplicatePixel(KisIteratorPixel* dst, KisIteratorPixel* dab, KisIteratorPixel* src) =0;

	/**
	 * Compose two arrays of pixels together. If source and target
	 * are not the same colour model, the source pixels will be
	 * converted to the target model.
	 */
	virtual void bitBlt(Q_INT32 stride,
			    QUANTUM *dst, 
			    Q_INT32 dststride,
			    KisStrategyColorSpaceSP srcSpace,
			    QUANTUM *src, 
			    Q_INT32 srcstride,
			    QUANTUM opacity,
			    Q_INT32 rows, 
			    Q_INT32 cols, 
			    CompositeOp op);

protected:

	/**
	 * Convert a byte array of srcLen pixels *src in the color space
	 * srcSpace to the current color model and put the converted
	 * bytes into the prepared byte array *dst.
	 */
	virtual void convertPixels(QUANTUM * src, KisStrategyColorSpaceSP srcSpace, QUANTUM * dst, Q_UINT32 srcLen);

	
	/**
	 * Compose two byte arrays containing pixels in the same color
	 * model together.
	 */
	virtual void bitBlt(Q_INT32 stride,
			    QUANTUM *dst, 
			    Q_INT32 dststride,
			    QUANTUM *src, 
			    Q_INT32 srcstride,
			    QUANTUM opacity,
			    Q_INT32 rows, 
			    Q_INT32 cols, 
			    CompositeOp op) = 0;
private:

	KisStrategyColorSpace(const KisStrategyColorSpace&);

	KisStrategyColorSpace& operator=(const KisStrategyColorSpace&);

private:

	QString m_name;
};

#endif // KIS_STRATEGY_COLORSPACE_H_
