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
#include <qstringlist.h>
#include <qpair.h>

#include <ksharedptr.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_channelinfo.h"
#include "kis_profile.h"

class QPainter;
class KisIteratorPixel;
class KisPixel;
class KisPixelRO;


// XXX: This is not enough to identify all the transforms we need: we
// need something that combines the two profiles + the rendering
// intent + the lcms colortype -- a certain profile can be used with
// many varieties of, e.g. RGB(a).
typedef QPair<KisProfileSP, KisProfileSP> KisProfilePair;

// This would be better, I guess.
struct transform {
	KisProfile srcProfile;
	Q_UINT32 srcCmType;
	KisProfile dstProfile;
	Q_UINT32 dstType;
	Q_UINT32 renderIntent;
};

class KisStrategyColorSpace : public KShared {


public:

	/**
	 * Create a new colorspace strategy.
	 *
	 * @param name The internal Krita name for this color strategy that we can use for loading and saving
	 * @param description i18n'able The user-friendly description of this strategy
	 * @param cmType The littlecms colorstrategy type we wrap.
	 * @param colorSpaceSignature The icc signature for the colorspace we are.
	 */
	KisStrategyColorSpace(const QString& name, const QString& description, Q_UINT32 cmType, icColorSpaceSignature colorSpaceSignature);

	virtual ~KisStrategyColorSpace();

public:

        /**
	 * The nativeColor methods take a given color that can be defined in any
	 * colorspace and fills a byte array with the corresponding color in the
	 * the colorspace managed by this strategy.
	 *
	 * The profile parameter is the profile of the paint device; the other profile
	 * is the display profile -- since we are moving from QColor that have most likely
	 * been picked from the display itself.
	 */
	virtual void nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP profile = 0) = 0;
	virtual void nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP profile = 0) = 0;

	// XXX: Add profile support
 	virtual void toQColor(const QUANTUM *src, QColor *c, KisProfileSP profile= 0 ) = 0;
 	virtual void toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0) = 0;

	virtual KisPixelRO toKisPixelRO(QUANTUM *src, KisProfileSP profile) = 0;
	virtual KisPixel toKisPixel(QUANTUM *src, KisProfileSP profile) = 0;

	// Return a vector describing all the channels this color model has.
	virtual vKisChannelInfoSP channels() const = 0;

	/**
	 * The total number of channels for a single pixel in this color model
	 */
	virtual Q_INT32 nChannels() const = 0;

	/**
	 * The total number of color channels (excludes alpha and substance) for a single
	 * pixel in this color model.
	 */
	virtual Q_INT32 nColorChannels() const = 0;

	/**
	 * The total number of substance channels for a single pixel
	 * in this color model
	 */
	virtual Q_INT32 nSubstanceChannels() const { return 0; };

	/**
	 * The size in bytes of a single pixel in this color model
	 */
	virtual Q_INT32 pixelSize() const = 0;

	/**
	 * Whether this color model has a channel of type ALPHA
	 */
	virtual bool alpha() const = 0;

	/**
	 * The internal identification of this color model
	 */
	inline QString name() const { return m_name; };


	/**
	 * The user-friendly name of this color model.
	 */
	inline QString description() const { return m_description; };

	/**
	 * This function is used to convert a KisPixelRepresentation from this color strategy to the specified
	 * color strategy.
	 */
	virtual bool convertTo(KisPixel& src, KisPixel& dst, Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

	/**
	 * Convert the pixels in data to (8-bit BGRA) QImage using the specified profiles.
	 * The pixels are supposed to be encoded in this color model.
	 *
	 * @param data A pointer to a contiguous memory region containing width * height pixels
	 * @param width in pixels
	 * @param height in pixels
	 */
	virtual QImage convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent = INTENT_PERCEPTUAL) = 0;

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
			    CompositeOp op,
			    KisProfileSP srcProfile = 0,
			    KisProfileSP dstProfile = 0);


	void setColorSpaceType(Q_UINT32 type) { m_cmType = type; }
	Q_UINT32 colorSpaceType() { return m_cmType; }

	void setColorSpaceSignature(icColorSpaceSignature signature) { m_colorSpaceSignature = signature; }
	icColorSpaceSignature colorSpaceSignature() { return m_colorSpaceSignature; }
	/**
	 * Get a list of profiles that apply to this color space
	 */
	vKisProfileSP profiles() { return m_profiles; }

	/**
	 * Reload the profiles from disk
	 */
	void resetProfiles();

	/**
	 * Return the number of profiles available for this color space
	 */
	Q_INT32 profileCount() const { return m_profiles.count(); }

	/**
	 * Return the profile associated with the given product name,
	 * or 0.
	 */
	KisProfileSP getProfileByName(const QString & name);

	virtual bool valid() { return true; }

protected:

	/**
	 * Convert a byte array of srcLen pixels *src to the specified color space
	 * and put the converted bytes into the prepared byte array *dst.
	 *
	 * Returns false if the conversion failed, true if it succeeded
	 */
	virtual bool convertPixelsTo(QUANTUM * src, KisProfileSP srcProfile,
				     QUANTUM * dst, KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile,
				     Q_UINT32 length,
				     Q_INT32 renderingIntent = INTENT_PERCEPTUAL);


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

	virtual cmsHTRANSFORM createTransform(KisStrategyColorSpaceSP dstColorStrategy,
					      KisProfileSP srcProfile,
					      KisProfileSP dstProfile,
					      Q_INT32 renderingIntent);

private:

	QString m_name;                              // The internal name
	QString m_description;                       // The user-friendly description
	Q_UINT32 m_cmType;                           // The colorspace type as defined by littlecms
	icColorSpaceSignature m_colorSpaceSignature; // The colorspace signature as defined in icm/icc files

	typedef QMap<KisProfilePair, cmsHTRANSFORM>  TransformMap;
	TransformMap m_transforms; // Cache for existing transforms

	KisStrategyColorSpace(const KisStrategyColorSpace&);
	KisStrategyColorSpace& operator=(const KisStrategyColorSpace&);

	vKisProfileSP m_profiles;
	QStringList m_profileFilenames;
};

#endif // KIS_STRATEGY_COLORSPACE_H_
