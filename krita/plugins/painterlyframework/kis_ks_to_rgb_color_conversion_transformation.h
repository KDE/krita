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

#ifndef KIS_KS_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_
#define KIS_KS_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_

#include "channel_converter.h"
#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace.h"

#include <KoColorConversionTransformation.h>
#include <KoColorConversionTransformationFactory.h>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

template< typename _TYPE_, int _N_ >
class KisKSToRGBColorConversionTransformation : public KoColorConversionTransformation {

typedef KoColorConversionTransformation parent;
typedef KisKSColorSpaceTrait<_TYPE_,_N_> CSTrait;

public:

    KisKSToRGBColorConversionTransformation(const KoColorSpace *srcCs, const KoColorSpace *dstCs)
    : parent(srcCs, dstCs)
    {
        m_profile = static_cast<const KisIlluminantProfile*>(parent::srcColorSpace()->profile());
        m_converter = new ChannelConverter<_TYPE_>(m_profile->Kblack(), m_profile->Sblack());
        m_rgbvec = gsl_vector_calloc( 3 );
        m_refvec = gsl_vector_calloc(_N_);
    }

    ~KisKSToRGBColorConversionTransformation()
    {
        delete m_converter;
        gsl_vector_free(m_rgbvec);
        gsl_vector_free(m_refvec);
    }

    void transform(const quint8 *src, quint8 *dst8, qint32 nPixels) const
    {
        quint16 *dst = reinterpret_cast<quint16*>(dst8);

        for ( ; nPixels > 0; nPixels-- ) {
            m_checkcolor = 0;
            for (int i = 0; i < _N_; i++) {
                m_converter->KSToReflectance(CSTrait::K(src, i), CSTrait::S(src, i), m_current);
                gsl_vector_set(m_refvec, i, m_current);
                m_checkcolor += m_current;
            }

            if (m_checkcolor <= 0.0)
                gsl_vector_set_all(m_rgbvec, 0.0);
            else if (m_checkcolor >= _N_)
                gsl_vector_set_all(m_rgbvec, 1.0);
            else
                gsl_blas_dgemv(CblasNoTrans, 1.0, m_profile->T(), m_refvec, 0.0, m_rgbvec);

            for (int i = 0; i < 3; i++)
                dst[2-i] = KoColorSpaceMaths<double,quint16>::scaleToA(gsl_vector_get(m_rgbvec, i));

            dst[3] = KoColorSpaceMaths<_TYPE_,quint16>::scaleToA(CSTrait::alpha(src));

            src += CSTrait::pixelSize;
            dst += 4;
        }
    }

private:
    gsl_vector *m_rgbvec;
    gsl_vector *m_refvec;

    ChannelConverter<_TYPE_> *m_converter;
    const KisIlluminantProfile *m_profile;

    mutable double m_current;
    mutable double m_checkcolor;

};

template< typename _TYPE_, int _N_ >
class KisKSToRGBColorConversionTransformationFactory : public KoColorConversionTransformationFactory {

public:
    KisKSToRGBColorConversionTransformationFactory(const QString &convid)
    : KoColorConversionTransformationFactory("KS"+convid+QString::number(_N_),
                                             KisKSColorSpace<_TYPE_,_N_>::ColorDepthId().id(),
                                             RGBAColorModelID.id(), Integer16BitsColorDepthID.id()) {}

    KoColorConversionTransformation *createColorTransformation(const KoColorSpace* srcColorSpace,
                                                               const KoColorSpace* dstColorSpace,
                                                               KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const
    {
        Q_UNUSED(renderingIntent);
        Q_ASSERT(canBeSource(srcColorSpace));
        Q_ASSERT(canBeDestination(dstColorSpace));
        return new KisKSToRGBColorConversionTransformation<_TYPE_,_N_>(srcColorSpace, dstColorSpace);
    }

    bool conserveColorInformation() const { return true; }
    bool conserveDynamicRange() const { return false; }

};

#endif // KIS_KS_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_
