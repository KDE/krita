/*
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KO_COLOR_CONVERSION_CACHE_HPP_
#define _KO_COLOR_CONVERSION_CACHE_HPP_

class KoCachedColorConversionTransformation;
class KoColorSpace;

#include "KoColorConversionTransformation.h"

/**
 * This class holds a cache of KoColorConversionTransformations.
 *
 * This class is not part of public API, and can be changed without notice.
 */
class KoColorConversionCache
{
public:
    struct CachedTransformation;
public:
    KoColorConversionCache();
    ~KoColorConversionCache();

    /**
     * This function returns a cached color transformation if available
     * or create one.
     * @param src source color space
     * @param dst destination color space
     * @param _renderingIntent rendering intent
     * @param conversionFlags conversion flags
     */
    KoCachedColorConversionTransformation cachedConverter(const KoColorSpace* src,
                                                          const KoColorSpace* dst,
                                                          KoColorConversionTransformation::Intent _renderingIntent,
                                                          KoColorConversionTransformation::ConversionFlags conversionFlags);

    /**
     * This function is called by the destructor of the color space to
     * warn the cache that any pointers to this color space is going to
     * be invalid and that the cache needs to stop using those pointers.
     * @param src source color space
     */
    void colorSpaceIsDestroyed(const KoColorSpace* src);
private:
    struct Private;
    Private* const d;
};

/**
 * This class hold a cached color conversion. It can only be created
 * by the cache and when it's deleted it return the transformation to
 * the pool of available color conversion transformation.
 *
 * This class is not part of public API, and can be changed without notice.
 */
class KoCachedColorConversionTransformation
{
    friend class KoColorConversionCache;
private:
    KoCachedColorConversionTransformation(KoColorConversionCache* cache,
                                          KoColorConversionCache::CachedTransformation* transfo);
public:
    KoCachedColorConversionTransformation(const KoCachedColorConversionTransformation&);
    ~KoCachedColorConversionTransformation();
public:
    const KoColorConversionTransformation* transformation() const;
private:
    struct Private;
    Private* const d;
};


#endif
