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

#include "kis_illuminant_profile_qp.h"

#include <QString>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

KisIlluminantProfileQP::KisIlluminantProfileQP(const QString &fileName)
    : parent(fileName), m_data(0), m_s(0)
{
    load();
}

KisIlluminantProfileQP::~KisIlluminantProfileQP()
{
    deleteAll();
}

bool KisIlluminantProfileQP::load()
{
    bool success;

    if (success = parent::load()) {
        setName(QString("%1 - QP").arg(illuminant()));

        deleteAll();

        int n  = wavelengths();
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
            int prev = (int)gsl_vector_get(m_P, i-1);
            int curr = (int)gsl_vector_get(m_P, i);
            gsl_matrix_set(m_data->Q, prev, prev, gsl_matrix_get(m_data->Q,prev,prev)+1.0);
            gsl_matrix_set(m_data->Q, curr, curr,  1.0);
            gsl_matrix_set(m_data->Q, prev, curr, -1.0);
            gsl_matrix_set(m_data->Q, curr, prev, -1.0);
        }

        gsl_matrix_memcpy(m_data->A, m_T);

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

    return success;
}

void KisIlluminantProfileQP::rgbToReflectance() const
{
    double min, max;
    gsl_vector_minmax(m_rgbvec, &min, &max);
    if (min == 1.0) {
        gsl_vector_set_all(m_refvec, 1.0);
        return;
    }
    if (max == 0.0) {
        gsl_vector_set_all(m_refvec, 0.0);
        return;
    }

    gsl_vector_memcpy(m_data->b, parent::m_rgbvec);

    gsl_cqpminimizer_set(m_s, m_data);

    do {
        gsl_cqpminimizer_iterate(m_s);
    } while (gsl_cqpminimizer_test_convergence(m_s, 1e-4, 1e-4) == GSL_CONTINUE);

    gsl_vector_memcpy(parent::m_refvec, gsl_cqpminimizer_x(m_s));
}

void KisIlluminantProfileQP::deleteAll()
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
