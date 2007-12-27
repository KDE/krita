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

#include "channel_converter.h"

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

ChannelConverter::ChannelConverter(float Kblack, float Sblack)
: Kb(Kblack), Sb(Sblack)
{
    double q1, q2, k1, k2, D, Sh;
    float r0; // Represent the reflectance of the reference black (no, it's not zero)

    KSToReflectance(Kb, Sb, r0);
    q1 = Kb / ( 1.0 + Kb*PHI(r0) );
    k1 = 1.0 + q1 - sqrt( q1*q1 + 2.0*q1 );

    // First system: retrieve w1 and w0
    // r0*w1 + w0 = k1
    //    w1 + w0 = 1
    D = r0 - 1.0;
    w1 = (float) ( k1 - 1.0 ) / D;
    w0 = (float) ( r0 - k1 ) / D;

    Sh = S(0.5);
    q1 = Kb / ( 1.0 + Sb );
    q2 = 0.25 * ( 4.0*Kb + Sh ) / ( Sb + Sh );
    k1 = 1.0 + q1 - sqrt( q1*q1 + 2.0*q1 );
    k2 = 1.0 + q2 - sqrt( q2*q2 + 2.0*q2 );
    // Second system: retrieve b2, b1 and b0
    // b2 + b1 + b0 = k1
    // b2/4 + b1/2 + b0 = k2
    // r0^2*b2 + r0*b1 + b0 = r0
    double marray[9] = { 1.0, 1.0, 1.0, 0.25, 0.5, 1.0, r0*r0, r0, 1.0 };
    double barray[3] = { k1, k2, r0 };
    gsl_matrix_view M = gsl_matrix_view_array(marray, 3, 3);
    gsl_vector_view b = gsl_vector_view_array(barray, 3);
    gsl_vector *x = gsl_vector_alloc(3);
    int s;
    gsl_permutation *p = gsl_permutation_alloc(3);
    gsl_linalg_LU_decomp(&M.matrix, p, &s);
    gsl_linalg_LU_solve(&M.matrix, p, &b.vector, x);

    b2 = gsl_vector_get(x, 0);
    b1 = gsl_vector_get(x, 1);
    b0 = gsl_vector_get(x, 2);

    gsl_permutation_free(p);
    gsl_vector_free(x);

}

ChannelConverter::~ChannelConverter()
{
}
