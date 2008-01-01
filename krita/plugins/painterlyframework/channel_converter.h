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

#ifndef CHANNEL_CONVERTER_H_
#define CHANNEL_CONVERTER_H_

#include <cmath>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#include <KoColorSpaceMaths.h>

template< typename _TYPE_ >
class ChannelConverter {

    public:
        ChannelConverter() {}
        ChannelConverter(const double &Kblack, const double &Sblack);
        ~ChannelConverter();

        inline void KSToReflectance(const _TYPE_ &K, const _TYPE_ &S, double &R) const;
        inline void reflectanceToKS(const double &R, _TYPE_ &K, _TYPE_ &S) const;

    private:
        double Kb, Sb;
        double w0, w1; // For whitening
        double b0, b1, b2; // For blackening and making K and S continuous in 0.5

        inline double PHI(const double &R) const { return (2.0*R)/((1.0-R)*(1.0-R)); }
        inline double PSI(const double &R) const { return ((1.0-R)*(1.0-R))/(2.0*R); }

        inline double W(const double &R) const { return w1*R + w0; }
        inline double B(const double &R) const { return b2*R*R + b1*R + b0; }

        inline double K(const double &R) const;
        inline double S(const double &R) const;
};

template< typename _TYPE_ >
ChannelConverter<_TYPE_>::ChannelConverter(const double &Kblack, const double &Sblack)
: Kb(Kblack), Sb(Sblack)
{
    double q1, q2, k1, k2, D, Sh;
    double r0; // Represent the reflectance of the reference black (no, it's not zero)

    r0 = 1.0 + (Kb/Sb) - sqrt( (Kb/Sb)*(Kb/Sb) + 2.0*(Kb/Sb) );
    q1 = Kb / ( 1.0 + Kb*PHI(r0) );
    k1 = 1.0 + q1 - sqrt( q1*q1 + 2.0*q1 );

    // First system: retrieve w1 and w0
    // r0*w1 + w0 = k1
    //    w1 + w0 = 1
    D = r0 - 1.0;
    w1 = ( k1 - 1.0 ) / D;
    w0 = ( r0 - k1 ) / D;

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
    int s;
    gsl_matrix_view M = gsl_matrix_view_array(marray, 3, 3);
    gsl_vector_view b = gsl_vector_view_array(barray, 3);
    gsl_vector *x = gsl_vector_alloc(3);
    gsl_permutation *p = gsl_permutation_alloc(3);
    gsl_linalg_LU_decomp(&M.matrix, p, &s);
    gsl_linalg_LU_solve(&M.matrix, p, &b.vector, x);

    b2 = gsl_vector_get(x, 0);
    b1 = gsl_vector_get(x, 1);
    b0 = gsl_vector_get(x, 2);

    gsl_permutation_free(p);
    gsl_vector_free(x);
}

template< typename _TYPE_ >
ChannelConverter<_TYPE_>::~ChannelConverter()
{
}

template< typename _TYPE_ >
inline double ChannelConverter<_TYPE_>::K(const double &R) const
{
    if (R <= 0.5)
        return ( 1.0 / (PHI(W(R))-PHI(R)) );
    else
        return ( S(R)*PSI(R) );
}

template< typename _TYPE_ >
inline double ChannelConverter<_TYPE_>::S(const double &R) const
{
    if (R > 0.5)
        return ( Kb - Sb*PSI(B(R)) ) / ( PSI(B(R)) - PSI(R) );
    else
        return ( K(R)*PHI(R) );
}

template< typename _TYPE_ >
inline void ChannelConverter<_TYPE_>::KSToReflectance(const _TYPE_ &K, const _TYPE_ &S, double &R) const
{
    const double k = KoColorSpaceMaths<_TYPE_,double>::scaleToA(K);
    const double s = KoColorSpaceMaths<_TYPE_,double>::scaleToA(S);
    if (s == 0.0) {
        R = 0.0;
        return;
    }

    if (k == 0.0) {
        R = 1.0;
        return;
    }

    const double Q = k/s;
    R = 1.0 + Q - sqrt( Q*Q + 2.0*Q );
}

template< typename _TYPE_ >
inline void ChannelConverter<_TYPE_>::reflectanceToKS(const double &R, _TYPE_ &K, _TYPE_ &S) const
{
    K = KoColorSpaceMaths<double,_TYPE_>::scaleToA(this->K(R));
    S = KoColorSpaceMaths<double,_TYPE_>::scaleToA(this->S(R));
}

#endif // CHANNEL_CONVERTER_H_
