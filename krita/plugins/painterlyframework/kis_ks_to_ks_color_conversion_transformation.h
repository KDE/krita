/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_KS_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_
#define KIS_KS_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace.h"

#include <KoColorConversionTransformation.h>
#include <KoColorConversionTransformationFactory.h>

template< typename src_TYPE_, typename dst_TYPE_, int _N_ >
class KisKSToKSColorConversionTransformation : public KoColorConversionTransformation {

typedef KoColorConversionTransformation parent;
typedef KisKSColorSpaceTrait<src_TYPE_,_N_> srcCSTrait;
typedef KisKSColorSpaceTrait<dst_TYPE_,_N_> dstCSTrait;

public:

    KisKSToKSColorConversionTransformation(const KoColorSpace *srcCs, const KoColorSpace *dstCs)
    : parent(srcCs, dstCs)
    {

    }

    void transform(const quint8 *src8, quint8 *dst8, int nPixels) const
    {
        const src_TYPE_ *src = reinterpret_cast<const src_TYPE_*>(src8);
        dst_TYPE_ *dst = reinterpret_cast<dst_TYPE_*>(dst8);

        for ( ; nPixels > 0; nPixels-- ) {

            for (int i = 0; i < _N_+1; i++)
                dst[i] = KoColorSpaceMaths<src_TYPE_,dst_TYPE_>::scaleToA(src[i]);

            src += srcCSTrait::pixelSize;
            dst += dstCSTrait::pixelSize;
        }

    }

};

template< typename src_TYPE_, typename dst_TYPE_, int _N_ >
class KisKSToKSColorConversionTransformationFactory : public KoColorConversionTransformationFactory {

public:
    KisKSToKSColorConversionTransformationFactory()
    : KoColorConversionTransformationFactory( QString("KS%1").arg(_N_),
                                              KisKSColorSpace<src_TYPE_,_N_>::ColorDepthId().id(),
                                              QString("KS%1").arg(_N_),
                                              KisKSColorSpace<dst_TYPE_,_N_>::ColorDepthId().id() ) { return; }

    KoColorConversionTransformation *createColorTransformation( const KoColorSpace* srcColorSpace,
                                                                const KoColorSpace* dstColorSpace,
                                                                KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const
    {
        Q_UNUSED(renderingIntent);
        Q_ASSERT(canBeSource(srcColorSpace));
        Q_ASSERT(canBeDestination(dstColorSpace));
        return new KisKSToKSColorConversionTransformation<src_TYPE_,dst_TYPE_,_N_>(srcColorSpace, dstColorSpace);
    }

    bool conserveColorInformation() const { return true; }
    bool conserveDynamicRange() const { return false; }

};

#endif // KIS_KS_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_
