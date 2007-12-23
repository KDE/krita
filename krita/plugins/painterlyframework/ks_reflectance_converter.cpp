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

#include <cmath>

#include "ks_reflectance_converter.h"

KSReflectanceConverter::KSReflectanceConverter(float whiteS, float blackK)
: S_w(whiteS), K_b(blackK)
{
}

KSReflectanceConverter::~KSReflectanceConverter()
{
}

void KSReflectanceConverter::KS2Reflectance(float K, float S, float &R) const
{
    if (S == 0.0) {
        R = 0.0;
        return;
    }

    if (K == 0.0) {
        R = 1.0;
        return;
    }

    float Q = K/S;
    R = 1.0 + Q - sqrt( Q*Q + 2.0*Q );
}

void KSReflectanceConverter::Reflectance2KS(float R, float &K, float &S) const
{
    if ( R == 0.0 ) {
        K = 0.8 * K_b;
        S = 0.0;
        return;
    }
    if ( R == 1.0 ) {
        K = 0.0;
        S = 0.8 * S_w;
        return;
    }
    if ( R <= 0.5 ) {
        float Rw = ( 1.0 + R ) / 2.0;
        float F  = ( 2.0 * R  ) / pow ( 1.0 - R , 2 );
        float Fw = ( 2.0 * Rw ) / pow ( 1.0 - Rw, 2 );
        K = S_w / ( Fw - F );
        S = K * F;
    }
    if ( R > 0.5 ) {
        float Rb = R / 2.0;
        float P  = pow ( 1.0 - R , 2 ) / ( 2.0 * R  );
        float Pb = pow ( 1.0 - Rb, 2 ) / ( 2.0 * Rb );
        S = K_b / ( Pb - P );
        K = S * P;
    }
}
