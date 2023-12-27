/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XSIMD_EXTENSIONS_H
#define XSIMD_EXTENSIONS_H

// xsimd detection and architecture setup.
#include "config/xsimd_arch.hpp"

#if defined(HAVE_XSIMD) && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
// xsimd extensions.
#include "arch/xsimd_isa.hpp"
#endif // HAVE_XSIMD

#endif // XSIMD_EXTENSIONS_H
