/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef KIS_XSIMD_CONFIG_HPP
#define KIS_XSIMD_CONFIG_HPP

#include <config-xsimd.h>

#ifdef HAVE_XSIMD

// MSVC patching.
#if defined(_MSC_VER)
#if defined(_M_ARM64)
#ifndef NDEBUG
#pragma message("Patching over MSVC for aarch64.")
#endif
#define __ARM_ARCH 8
#define __aarch64__ 1
#define __ARM_NEON 1
#endif

#if defined(_M_ARM)
#ifndef NDEBUG
#pragma message("Patching over MSVC for arm-v7a.")
#endif
#define __ARM_ARCH _M_ARM
#define __ARM_NEON 1
#endif
#endif

#include <xsimd/xsimd.hpp>

#else /* HAVE_XSIMD */

namespace xsimd
{
struct generic
{
    static constexpr bool supported() noexcept { return true; }
    static constexpr bool available() noexcept { return true; }
    static constexpr char const* name() noexcept { return "xsimd not available, unvectorized"; }
};
}; // namespace xsimd

#endif /* HAVE_XSIMD */

#endif // KIS_XSIMD_CONFIG_HPP
