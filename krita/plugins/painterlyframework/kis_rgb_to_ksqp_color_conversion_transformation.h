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

#ifndef KIS_RGB_TO_KSQP_COLOR_CONVERSION_TRANSFORMATION_H_
#define KIS_RGB_TO_KSQP_COLOR_CONVERSION_TRANSFORMATION_H_

#include "kis_illuminant_profile.h"
#include "kis_rgb_to_ks_color_conversion_transformation.h"
#include <KoColorConversionTransformationFactory.h>

extern "C" {
    #include "cqp/gsl_cqp.h"
}

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

template< typename _TYPE_, int _N_ >
class KisRGBToKSQPColorConversionTransformation : public KisRGBToKSColorConversionTransformation< _TYPE_,_N_ > {

typedef KisRGBToKSColorConversionTransformation< _TYPE_,_N_ > parent;

public:

    KisRGBToKSQPColorConversionTransformation(const KoColorSpace *srcCs, const KoColorSpace *dstCs)
    : parent(srcCs, dstCs)
    {
        int n  = _N_;
        int me = 3;
        int mi = 2*n; // each reflectance is bounded between 0 and 1, two inequalities, and the transformation matrix

        //// ALLOCATION
        m_data = new gsl_cqp_data;
        m_data->Q = gsl_matrix_calloc(n,n);
        m_data->q = gsl_vector_calloc(n); // This will remain zero
        m_data->A = gsl_matrix_calloc(me,n);
        m_data->C = gsl_matrix_calloc(mi,n);
        m_data->b = gsl_vector_calloc(me);
        m_data->d = gsl_vector_calloc(mi);

        //// INITIALIZATION
        // The Q matrix represents this function:
        // z = ( R_P[0] - R_P[1] )^2 + ( R_P[1] - R_P[2] )^2 + ... + ( R_P[n-2] - R_P[n-1] )^2
        for (int i = 1; i < n; i++) {
            int prev = (int)gsl_vector_get(parent::m_profile->P(), i-1);
            int curr = (int)gsl_vector_get(parent::m_profile->P(), i);
            gsl_matrix_set(m_data->Q, prev, prev, gsl_matrix_get(m_data->Q,prev,prev)+1.0);
            gsl_matrix_set(m_data->Q, curr, curr,  1.0);
            gsl_matrix_set(m_data->Q, prev, curr, -1.0);
            gsl_matrix_set(m_data->Q, curr, prev, -1.0);
        }

        gsl_matrix_memcpy(m_data->A, parent::m_profile->T());

        // The C matrix and d vector represent the inequalities that the variables must
        // consider. In our case, they're: x_i >= 0, -x_i >= -1 (you can only define >= inequalities)
        for (int i = 0; i < n; i++) {
            int j = 2*i;
            gsl_matrix_set(m_data->C, j+0, i,  1.0);
            gsl_matrix_set(m_data->C, j+1, i, -1.0);

            gsl_vector_set(m_data->d, j+0,  0.0);
            gsl_vector_set(m_data->d, j+1, -1.0);
        }

        m_s = gsl_cqpminimizer_alloc(gsl_cqpminimizer_mg_pdip, n, me, mi);
    }

    ~KisRGBToKSQPColorConversionTransformation()
    {
        gsl_vector_free(m_data->d);
        gsl_vector_free(m_data->b);
        gsl_matrix_free(m_data->C);
        gsl_matrix_free(m_data->A);
        gsl_vector_free(m_data->q);
        gsl_matrix_free(m_data->Q);
        delete m_data;

        gsl_cqpminimizer_free(m_s);
    }

protected:

    virtual void RGBToReflectance() const
    {
        gsl_vector_memcpy(m_data->b, parent::m_rgbvec);

        gsl_cqpminimizer_set(m_s, m_data);

        do {
            gsl_cqpminimizer_iterate(m_s);
        } while (gsl_cqpminimizer_test_convergence(m_s, 1e-4, 1e-4) == GSL_CONTINUE);

        gsl_vector_memcpy(parent::m_refvec, gsl_cqpminimizer_x(m_s));
    }

protected:

    gsl_cqp_data *m_data;
    gsl_cqpminimizer *m_s;

};

template< typename _TYPE_, int _N_ >
class KisRGBToKSQPColorConversionTransformationFactory : public KoColorConversionTransformationFactory {

public:
    KisRGBToKSQPColorConversionTransformationFactory()
    : KoColorConversionTransformationFactory(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(),
                                             "KSQP"+QString::number(_N_), KisKSColorSpace<_TYPE_,_N_>::ColorDepthId().id()) {}

    KoColorConversionTransformation *createColorTransformation(const KoColorSpace* srcColorSpace,
                                                               const KoColorSpace* dstColorSpace,
                                                               KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const
    {
        Q_UNUSED(renderingIntent);
        Q_ASSERT(canBeSource(srcColorSpace));
        Q_ASSERT(canBeDestination(dstColorSpace));
        return new KisRGBToKSQPColorConversionTransformation<_TYPE_,_N_>(srcColorSpace, dstColorSpace);
    }

    bool conserveColorInformation() const { return true; }
    bool conserveDynamicRange() const { return false; }

};

#endif // KIS_RGB_TO_KSQP_COLOR_CONVERSION_TRANSFORMATION_H_
