/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_KS_COLOR_SPACE_ENGINE_H_
#define _KIS_KS_COLOR_SPACE_ENGINE_H_

#include <KoColorSpaceEngine.h>

#include "kis_ks_to_ks_color_conversion_transformation.h"

#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KLocale>

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

template< int _N_ >
class KisKSColorSpaceEngine : public KoColorSpaceEngine
{
public:
    KisKSColorSpaceEngine();
    ~KisKSColorSpaceEngine();
    KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;
};

template< int _N_ >
KisKSColorSpaceEngine<_N_>::KisKSColorSpaceEngine() : KoColorSpaceEngine(QString("ks%1").arg(_N_), i18n("KS%1 Engine", _N_))
{
}

template< int _N_ >
KisKSColorSpaceEngine<_N_>::~KisKSColorSpaceEngine()
{
}

template< int _N_ >
KoColorConversionTransformation* KisKSColorSpaceEngine<_N_>::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    Q_UNUSED(renderingIntent)

    if (srcColorSpace->colorDepthId().id() == "F32" && dstColorSpace->colorDepthId().id() == "F32")
        return new KisKSToKSColorConversionTransformation<float, float, _N_>(srcColorSpace, dstColorSpace);
#ifdef HAVE_OPENEXR
    else if (srcColorSpace->colorDepthId().id() == "F32" && dstColorSpace->colorDepthId().id() == "F16")
        return new KisKSToKSColorConversionTransformation<float, half, _N_>(srcColorSpace, dstColorSpace);
    else if (srcColorSpace->colorDepthId().id() == "F16" && dstColorSpace->colorDepthId().id() == "F32")
        return new KisKSToKSColorConversionTransformation<half, float, _N_>(srcColorSpace, dstColorSpace);
    else
        return new KisKSToKSColorConversionTransformation<half, half, _N_>(srcColorSpace, dstColorSpace);
#endif
}

#endif
