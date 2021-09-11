/*
 *  SPDX-FileCopyrightText: 2018 Iván Santa María <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    static Vc_ALWAYS_INLINE Vc::float_v erf(Vc::float_v::AsArg x) {
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
