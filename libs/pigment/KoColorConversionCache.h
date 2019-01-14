/*
 * Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
