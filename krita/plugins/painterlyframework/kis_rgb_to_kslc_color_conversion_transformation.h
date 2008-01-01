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

#ifndef KIS_RGB_TO_KSLC_COLOR_CONVERSION_TRANSFORMATION_H_
#define KIS_RGB_TO_KSLC_COLOR_CONVERSION_TRANSFORMATION_H_

#include "kis_illuminant_profile.h"
#include "kis_rgb_to_ksqp_color_conversion_transformation.h"
#include <KoColorConversionTransformationFactory.h>

extern "C" {
    #include "cqp/gsl_cqp.h"
}

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

template< typename _TYPE_, int _N_ >
class KisRGBToKSLCColorConversionTransformation : public KisRGBToKSQPColorConversionTransformation< _TYPE_,_N_ > {

typedef KisRGBToKSQPColorConversionTransformation< _TYPE_,_N_ > parent;

public:

    KisRGBToKSLCColorConversionTransformation(const KoColorSpace *srcCs, const KoColorSpace *dstCs)
    : parent(srcCs, dstCs)
    {
        // Calculate primaries
        m_red   = gsl_vector_calloc(_N_);
        m_green = gsl_vector_calloc(_N_);
        m_blue  = gsl_vector_calloc(_N_);

        // First: red
        gsl_vector_set(parent::m_rgbvec, 0, 1.0);
        gsl_vector_set(parent::m_rgbvec, 1, 0.0);
        gsl_vector_set(parent::m_rgbvec, 2, 0.0);
        parent::RGBToReflectance();
        for (int i = 0; i < _N_; i++)
            gsl_vector_set(m_red, i, gsl_vector_get(parent::m_refvec, i));

        // Second: blue: the reflectance can't be again in the interval [0, 1],
        // we need to be in the interval [0, 1-red]
        for (int i = 0; i < _N_; i++) {
            int j = 6+2*i;
            gsl_vector_set(parent::m_data->d, j+1, -(1.0 - gsl_vector_get(m_red, i)));
        }
        gsl_vector_set(parent::m_rgbvec, 0, 0.0);
        gsl_vector_set(parent::m_rgbvec, 1, 0.0);
        gsl_vector_set(parent::m_rgbvec, 2, 1.0);
        parent::RGBToReflectance();
        for (int i = 0; i < _N_; i++)
            gsl_vector_set(m_blue, i, gsl_vector_get(parent::m_refvec, i));

        // Third: green; simply remove blue and red from pure white.
        for (int i = 0; i < _N_; i++)
            gsl_vector_set(m_green, i, 1.0 - ( gsl_vector_get(m_red, i) + gsl_vector_get(m_blue, i) ) );

        parent::m_useCache = false;
    }

    ~KisRGBToKSLCColorConversionTransformation()
    {
        gsl_vector_free(m_red);
        gsl_vector_free(m_green);
        gsl_vector_free(m_blue);
    }

protected:

    void RGBToReflectance() const
    {
        for (int i = 0; i < _N_; i++) {
            double curr = gsl_vector_get(parent::m_rgbvec, 0) * gsl_vector_get(m_red,   i) +
                          gsl_vector_get(parent::m_rgbvec, 1) * gsl_vector_get(m_green, i) +
                          gsl_vector_get(parent::m_rgbvec, 2) * gsl_vector_get(m_blue,  i);

            if (fabs(curr - 0.0) < 1e-6)
                gsl_vector_set(parent::m_refvec, i, 0.0);
            else if (fabs(curr - 1.0) < 1e-6)
                gsl_vector_set(parent::m_refvec, i, 1.0);
            else
                gsl_vector_set(parent::m_refvec, i, curr);
        }
    }

private:

    gsl_vector *m_red, *m_green, *m_blue;

};

template< typename _TYPE_, int _N_ >
class KisRGBToKSLCColorConversionTransformationFactory : public KoColorConversionTransformationFactory {

public:
    KisRGBToKSLCColorConversionTransformationFactory()
    : KoColorConversionTransformationFactory(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(),
                                             "KSLC"+QString::number(_N_), KisKSColorSpace<_TYPE_,_N_>::ColorDepthId().id()) {}

    KoColorConversionTransformation *createColorTransformation(const KoColorSpace* srcColorSpace,
                                                               const KoColorSpace* dstColorSpace,
                                                               KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const
    {
        Q_UNUSED(renderingIntent);
        Q_ASSERT(canBeSource(srcColorSpace));
        Q_ASSERT(canBeDestination(dstColorSpace));
        return new KisRGBToKSLCColorConversionTransformation<_TYPE_,_N_>(srcColorSpace, dstColorSpace);
    }

    bool conserveColorInformation() const { return true; }
    bool conserveDynamicRange() const { return false; }

};

#endif // KIS_RGB_TO_KSQP_COLOR_CONVERSION_TRANSFORMATION_H_
