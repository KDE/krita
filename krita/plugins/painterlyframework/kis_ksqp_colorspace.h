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

#ifndef KIS_KSQP_COLORSPACE_H_
#define KIS_KSQP_COLORSPACE_H_

extern "C" {
    #include "cqp/gsl_cqp.h"
}
#include <cstdio>

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace_traits.h"
#include "kis_ks_colorspace.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

template<int _N_>
class KisKSQPColorSpace : public KisKSColorSpace<_N_>
{
    typedef KisKSColorSpace<_N_> parent;

    public:

        KisKSQPColorSpace(KoColorProfile *p);
        ~KisKSQPColorSpace();

        KoID colorModelId() const;

    protected:
        void RGBToReflectance() const;

    private:
        gsl_cqp_data *m_data;
        gsl_cqpminimizer *m_s;
        const gsl_cqpminimizer_type *m_T;

};

////////////////////////////////////////////
//            IMPLEMENTATION              //
////////////////////////////////////////////

template<int _N_>
KisKSQPColorSpace<_N_>::KisKSQPColorSpace(KoColorProfile *p)
: parent(p, "KS"+QString::number(_N_)+"QPF32",
          i18n(QString("%1-pairs Absorption-Scattering QP (32 Bits Float)").arg(_N_).toUtf8().data())),
          m_data(0), m_s(0)
{
    if (!parent::profileIsCompatible(p))
        return;

    int n  = _N_ + 1;
    int me = 1; // m_rgbvec->size; // == 3
    int mi = 6+2*n; // each reflectance is bounded between 0 and 1, two inequalities, and the transformation matrix

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
    for (int i = 1; i < n-1; i++) {
        int prev = (int)gsl_vector_get(parent::m_profile->P(), i-1);
        int curr = (int)gsl_vector_get(parent::m_profile->P(), i);
        //         int prev = i-1, curr = i;
        gsl_matrix_set(m_data->Q, prev, prev, gsl_matrix_get(m_data->Q,prev,prev)+1.0);
        //         gsl_matrix_set(m_data->Q, prev, prev,  2.0);
        gsl_matrix_set(m_data->Q, curr, curr,  1.0);
        gsl_matrix_set(m_data->Q, prev, curr, -1.0);
        gsl_matrix_set(m_data->Q, curr, prev, -1.0);
    }
    gsl_matrix_set(m_data->Q, n-1, n-1, 1.0);

    gsl_matrix_set(m_data->A, 0, n-1, 1.0);
    gsl_vector_set(m_data->b, 0, 1.0);

    gsl_matrix_view subm;
    subm = gsl_matrix_submatrix(m_data->C, 0, 0, 3, n-1);
    gsl_matrix_memcpy(&subm.matrix, parent::m_profile->T());
    subm = gsl_matrix_submatrix(m_data->C, 3, 0, 3, n-1);
    gsl_matrix_memcpy(&subm.matrix, parent::m_profile->T());
    gsl_matrix_scale(&subm.matrix, -1.0);

    // The C matrix and d vector represent the inequalities that the variables must
    // consider. In our case, they're: x_i >= 0, -x_i >= -1 (you can only define >= inequalities)
    for (int i = 0; i < n-1; i++) {
        int j = 6+2*i;
        gsl_matrix_set(m_data->C, j+0, i,  1.0);
        gsl_matrix_set(m_data->C, j+1, i, -1.0);

        gsl_vector_set(m_data->d, j+0,  0.0);
        gsl_vector_set(m_data->d, j+1, -1.0);
    }

    m_T = gsl_cqpminimizer_mg_pdip;
    m_s = gsl_cqpminimizer_alloc(m_T, n, me, mi);
}

template<int _N_>
KisKSQPColorSpace<_N_>::~KisKSQPColorSpace()
{
    if (m_data) {
        gsl_vector_free(m_data->d);
        gsl_vector_free(m_data->b);
        gsl_matrix_free(m_data->C);
        gsl_matrix_free(m_data->A);
        gsl_vector_free(m_data->q);
        gsl_matrix_free(m_data->Q);
        delete m_data;
    }
    if (m_s)
        gsl_cqpminimizer_free(m_s);
}

template<int _N_>
KoID KisKSQPColorSpace<_N_>::colorModelId() const
{
    return KoID("KS"+QString::number(_N_)+"QP",
                 i18n(QString("%1-pairs Absorption-Scattering QP").arg(_N_).toUtf8().data()));
}

template<int _N_>
void KisKSQPColorSpace<_N_>::RGBToReflectance() const
{
    for (int i = 0; i < 3; i++) {
        gsl_vector_set(m_data->d, 0+i,  ( gsl_vector_get(parent::m_rgbvec, i) - 0.0 ) );
        gsl_vector_set(m_data->d, 3+i, -( gsl_vector_get(parent::m_rgbvec, i) + 0.0 ) );
    }

    //// SOLVER INITIALIZATION
    const size_t max_iter = 100;
    size_t iter = 1;
    int status;

    status = gsl_cqpminimizer_set(m_s, m_data);

    do {
        status = gsl_cqpminimizer_iterate(m_s);
        status = gsl_cqpminimizer_test_convergence(m_s, 1e-10, 1e-10);

        if(status != GSL_SUCCESS)
            iter++;

    } while(status == GSL_CONTINUE && iter<=max_iter);

    for (uint i = 0; i < parent::m_refvec->size; i++) {
        double curr = gsl_vector_get(gsl_cqpminimizer_x(m_s), i);

        if (fabs(curr - 0.0) < 1e-6)
            gsl_vector_set(parent::m_refvec, i, 0.0);
        else if (fabs(curr - 1.0) < 1e-6)
            gsl_vector_set(parent::m_refvec, i, 1.0);
        else
            gsl_vector_set(parent::m_refvec, i, curr);
    }
}

#endif // KIS_KSQP_COLORSPACE_H_
