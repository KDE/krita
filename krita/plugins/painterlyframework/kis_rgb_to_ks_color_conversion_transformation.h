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

#ifndef KIS_RGB_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_
#define KIS_RGB_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

#include "channel_converter.h"
#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace_traits.h"

#include <gsl/gsl_vector.h>

template< int _N_ >
class KisRGBToKSColorConversionTransformation : public KoColorConversionTransformation {
typedef KoColorConversionTransformation parent;

public:
    KisRGBToKSColorConversionTransformation(const KoColorSpace *srcCs, const KoColorSpace *dstCs)
    : parent(srcCs, dstCs), m_rgbvec(0), m_refvec(0), m_converter(0), m_profile(0)
    {
        m_profile = dynamic_cast<const KisIlluminantProfile*>(parent::dstColorSpace()->profile());
        m_converter = new ChannelConverter(m_profile->Kblack(), m_profile->Sblack());
        m_rgbvec = gsl_vector_calloc(3);
        m_refvec = gsl_vector_calloc(_N_);
    }

    ~KisRGBToKSColorConversionTransformation()
    {
        delete m_converter;
        gsl_vector_free(m_rgbvec);
        gsl_vector_free(m_refvec);
    }

    void transform(const quint8 *src8, quint8 *dst, qint32 nPixels) const
    {
        // For each pixel we do this:
        // 1 - convert raw bytes to quint16
        // 2 - find reflectances using the transformation matrix of the profile.
        // 3 - convert reflectances to K/S

        const float *src = reinterpret_cast<const float *>(src8);
        float c;

        for ( ; nPixels > 0; nPixels-- ) {
            for (int i = 0; i < 3; i++) {
                m_converter->sRGBToRGB(src[2-i], c);
                gsl_vector_set(m_rgbvec, i, c);
            }

            RGBToReflectance();

            for (int i = 0; i < _N_; i++) {
                m_converter->reflectanceToKS((float)gsl_vector_get(m_refvec, i),
                                                KisKSColorSpaceTrait<_N_>::K(dst, i),
                                                KisKSColorSpaceTrait<_N_>::S(dst, i));
            }

            KisKSColorSpaceTrait<_N_>::setAlpha(dst, KoColorSpaceMaths<float,quint8>::scaleToA(src[3]), 1);

            src += 4;
            dst += KisKSColorSpaceTrait<_N_>::pixelSize;
        }
    }

protected:
    virtual void RGBToReflectance() const = 0;

protected:
    gsl_vector *m_rgbvec;
    gsl_vector *m_refvec;

    ChannelConverter *m_converter;
    const KisIlluminantProfile *m_profile;
};

#endif // KIS_RGB_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_
