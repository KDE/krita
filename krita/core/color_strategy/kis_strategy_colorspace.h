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
#if !defined KIS_STRATEGY_COLORSPACE_H_
#define KIS_STRATEGY_COLORSPACE_H_

#include <map>
#include <qcolor.h>

#include <ksharedptr.h>
#include <koColor.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_channelinfo.h"
#include "kis_compositeop.h"

class QPainter;
class KisIteratorPixel;
class KisPixelRepresentation;
class KisPixelRepresentationRGB;

class KisStrategyColorSpace : public KShared {
	typedef std::map<QString, KisCompositeOp*> compositeOpStorage;
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

	// XXX: make this a proper vector. Pointers to arrays are _so_ seventies, and
	// Stroustrup assures us a vector is as effecient a mem array anyway.
	virtual ChannelInfo * channelsInfo() const = 0;
	virtual Q_INT32 depth() const = 0;
	virtual bool alpha() const = 0;
	inline QString name() { return m_name; };

	virtual void render(KisImageSP projection, QPainter& painter, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) = 0;

	/** This function is used to convert a KisPixelRepresentation to an other color strategy.
		* When implementing a color space, there is no need to implement a conversion to all strategy,
		* if there is no direct conversion facilities, the function should use the conversion to/from RGBA
		*/
	virtual void convertTo(KisPixelRepresentation& src, KisPixelRepresentation& dst,  KisStrategyColorSpaceSP cs);
	/** This function convert a pixel to RGBA */
	virtual void convertToRGBA(KisPixelRepresentation& src, KisPixelRepresentationRGB& dst) =0;
	/** This function convert a pixel from RGBA */
	virtual void convertFromRGBA(KisPixelRepresentationRGB& src, KisPixelRepresentation& dst) =0;
	
	virtual QImage convertToImage(KisImageSP image, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const = 0;
	virtual QImage convertToImage(KisTileMgrSP tm, Q_UINT32 depth, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const = 0;

	virtual void tileBlt(Q_INT32 stride,
			     QUANTUM *dst, 
			     Q_INT32 dststride,
			     QUANTUM *src, 
			     Q_INT32 srcstride,
			     Q_INT32 rows, 
			     Q_INT32 cols, 
			     CompositeOp op) const = 0;
	
	virtual void tileBlt(Q_INT32 stride,
			     QUANTUM *dst, 
			     Q_INT32 dststride,
			     QUANTUM *src, 
			     Q_INT32 srcstride,
			     QUANTUM opacity,
			     Q_INT32 rows, 
			     Q_INT32 cols, 
			     CompositeOp op) const = 0;
	
	virtual void computeDuplicatePixel(KisIteratorPixel* dst, KisIteratorPixel* dab, KisIteratorPixel* src) =0;
	
	void addCompositeOp(KisCompositeOp* newco);
	KisCompositeOp* compositeOp(const QString& name);

private:
	KisStrategyColorSpace(const KisStrategyColorSpace&);
	KisStrategyColorSpace& operator=(const KisStrategyColorSpace&);
	compositeOpStorage m_compositeOpStorage;
	QString m_name;
};

inline void KisStrategyColorSpace::addCompositeOp(KisCompositeOp* newco)
{
	m_compositeOpStorage.insert(compositeOpStorage::value_type( newco->name(),newco));
}
inline KisCompositeOp* KisStrategyColorSpace::compositeOp(const QString& name)
{
	return m_compositeOpStorage.find(name)->second;
}


#endif // KIS_STRATEGY_COLORSPACE_H_

