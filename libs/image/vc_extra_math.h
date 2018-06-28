/*
 *  Copyright (c) 2018 Iván Santa María <ghevan@gmail.com>
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

#ifndef VC_ADDITIONAL_MATH_H
#define VC_ADDITIONAL_MATH_H

#include <config-vc.h>

#if defined HAVE_VC

#include <Vc/Vc>
#include <Vc/IO>

class VcExtraMath
{
public:
    // vectorized erf function, precision 1e-5
    static inline Vc::float_v erf(Vc::float_v x) {
        Vc::float_v xa = abs(x);
        Vc::float_m precisionLimit(xa >= 9.3f); // wrong result for any number beyond this
        xa(precisionLimit) = 0;
        Vc::float_v sign(Vc::One);
        Vc::float_m invertMask = x < 0.f;
        sign(invertMask) = -1.f;

        // CONSTANTS
        float a1 =  0.254829592;
        float a2 = -0.284496736;
        float a3 =  1.421413741;
        float a4 = -1.453152027;
        float a5 =  1.061405429;
        float p  =  0.3275911;

        Vc::float_v t = 1.0f / (1.0f + p * xa);
        Vc::float_v y = 1.0f - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-xa * xa);
        y(precisionLimit) = 1.0f;
        return sign * y;
    }
};
#endif /* defined HAVE_VC */


#endif // VC_ADDITIONAL_MATH_H
