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

#include <qmap.h>
#include <qcolor.h>

#include <ksharedptr.h>
#include <koColor.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_channelinfo.h"

class QPainter;
class KisIteratorPixel;
class KisPixel;
class KisPixelRO;

class KisStrategyColorSpace : public KShared {


public:
	KisStrategyColorSpace();
	/**
	 * Create a new colorspace strategy.
	 * 
	 * @param name The user-friendly name of this strategy
	 * @param cmType The littlecms colorstrategy type we wrap.
	 */
	KisStrategyColorSpace(const QString& name, Q_UINT32 cmType);
	virtual ~KisStrategyColorSpace();

public:
        // The nativeColor methods take a given color that can be defined in any
        // colorspace and fills a byte array with the corresponding color in the
        // the colorspace managed by this strategy. 
	virtual void nativeColor(const KoColor& c, QUANTUM *dst) = 0;
	virtual void nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst) = 0;

	virtual void toKoColor(const QUANTUM *src, KoColor *c) = 0;
	virtual void toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity) = 0;

	virtual KisPixelRO toKisPixelRO(QUANTUM *src) = 0;
	virtual KisPixel toKisPixel(QUANTUM *src) = 0;
	
	// Return a vector describing all the channels this color model has.
	virtual vKisChannelInfoSP channels() const = 0;
	
	virtual Q_UINT32 colorModelType() { return m_cmType; };

	/**
	 * The total number of channels for a single pixel in this color model
	 */
	virtual Q_INT32 depth() const = 0;
	
	/**
	 * The total number of color channels (excludes alpha and substance) for a single
	 * pixel in this color model.
	 */
	virtual Q_INT32 nColorChannels() const = 0;

	/**
	 * Whether this color model has a channel of type ALPHA
	 */
	virtual bool alpha() const = 0;

	/**
	 * The user-friendly name of this color model.
	 */
	inline QString name() const { return m_name; };

	/**
	 * This function is used to convert a KisPixelRepresentation to another color strategy.
	 */
	virtual void convertTo(KisPixel& src, KisPixel& dst,  KisStrategyColorSpaceSP cs);
	
	virtual QImage convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride) const = 0;

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

	/**
	 * Set the icm profile for conversion between color spaces
	 * 
	 * XXX: make user-definable for certain transforms. Maybe
	 * only in the switch color space dialog for the whole image.
	 */
	void setProfile(cmsHPROFILE profile) { m_profile = profile; }

	/**
	 * Get the icm profile for conversion between color spaces.
	 */
	cmsHPROFILE getProfile() { return m_profile; }


protected:

	/**
	 * Convert a byte array of srcLen pixels *src in the color space
	 * srcSpace to the current color model and put the converted
	 * bytes into the prepared byte array *dst.
	 *
	 * This uses littlecms by default with default icm profiles.
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


	QString m_name;    // The user-friendly name
	Q_UINT32 m_cmType; // The colorspace type as defined by littlecms
	
	typedef QMap<Q_UINT32, cmsHTRANSFORM>  TransformMap;
	TransformMap m_transforms; // Cache for existing transforms

 	cmsHPROFILE m_profile; // THe default profile for this color strategy

private:

	KisStrategyColorSpace(const KisStrategyColorSpace&);
	KisStrategyColorSpace& operator=(const KisStrategyColorSpace&);
	
	
};

#endif // KIS_STRATEGY_COLORSPACE_H_
