/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "xsimd_extensions/xsimd.hpp"

#ifndef KIS_SUPPORTED_ARCHITECTURES_H
#define KIS_SUPPORTED_ARCHITECTURES_H

template<typename S>
class KisSupportedArchitectures
{
public:
    static S currentArchitecture()
    {
        return xsimd::current_arch::name();
    }

    static S supportedInstructionSets()
    {
        S archs;
#ifdef HAVE_XSIMD
        xsimd::all_architectures::for_each(is_supported_arch{archs});
#endif
        return archs;
    }

private:
    struct is_supported_arch
    {
        is_supported_arch(S &log)
            : l (log) {}

        template<typename A>
        void operator()(A) const
        {
#ifdef HAVE_XSIMD
            if (A::version() <= xsimd::available_architectures().best) {
                l.append(A::name()).append(" ");
            }
#endif
        }

        S &l;
    };
};

#endif // KIS_SUPPORTED_ARCHITECTURES_H
