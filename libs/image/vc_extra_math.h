/*
 *  SPDX-FileCopyrightText: 2018 Iván Santa María <ghevan@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef VC_ADDITIONAL_MATH_H
#define VC_ADDITIONAL_MATH_H

#include <xsimd_extensions/xsimd.hpp>

#if defined(HAVE_XSIMD) && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)

class VcExtraMath
{
public:

    // vectorized erf function, precision 1e-5
    template<typename A>
    static inline xsimd::batch<float, A> erf(const xsimd::batch<float, A> x)
    {
        /**
         * Our version of erf() is about 10% faster than the version in
         * xsimd::erf(), because we require less precision. We don't need
         * too much of precision to calculate 8-bit masks anyway.
         */

        using float_v = xsimd::batch<float, A>;
        using float_m = typename float_v::batch_bool_type;
        float_v xa = xsimd::abs(x);
        float_m precisionLimit = xa >= float_v(9.3f); // wrong result for any number beyond this
        xa = xsimd::set_zero(xa, precisionLimit);
        float_v sign(1.0f);
        float_m invertMask = x < float_v(0.f);
        sign = xsimd::select(invertMask, float_v(-1.f), sign);

        // CONSTANTS
        float a1 =  0.254829592f;
        float a2 = -0.284496736f;
        float a3 =  1.421413741f;
        float a4 = -1.453152027f;
        float a5 =  1.061405429f;
        float p  =  0.3275911f;

        float_v t = 1.0f / (1.0f + p * xa);
        float_v y = 1.0f - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-xa * xa);
        y = xsimd::set_one(y, precisionLimit);
        return sign * y;
    }
};
#endif /* defined HAVE_XSIMD */


#endif // VC_ADDITIONAL_MATH_H
