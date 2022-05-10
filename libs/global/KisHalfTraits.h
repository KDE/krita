/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISHALFTRAITS_H
#define KISHALFTRAITS_H

#include <KoConfig.h>
#ifdef HAVE_OPENEXR

#include <half.h>

#if (IMATH_VERSION_MAJOR <= 3) && (IMATH_VERSION_MINOR <= 1) && (IMATH_VERSION_PATCH <= 4)
#include <halfLimits.h>
#endif

#include <type_traits>

namespace std {

#if defined __GLIBCXX__

template<>
  struct __is_integral_helper<half>
  : public false_type { };

template<>
  struct __is_floating_point_helper<half>
  : public true_type { };

#elif defined _LIBCPP_VERSION

template<>
  struct __libcpp_is_integral<half>
  : public false_type { };

template<>
  struct __libcpp_is_floating_point<half>
  : public true_type { };


#else
  // these are fallback implementations that
  // don't support cv-qualifiers removal

  template<>
  struct is_integral<half>
    : public false_type
    { };

  template<>
  struct is_floating_point<half>
    : public true_type
    { };

#endif

  inline bool
  isfinite(half __x)
  { return __x.isFinite(); }

  inline bool
  isinf(half __x)
  { return __x.isInfinity(); }

  inline bool
  isnan(half __x)
  { return __x.isNan(); }

  inline bool
  isnormal(half __x)
  { return __x.isNormalized(); }

  inline bool
  signbit(half __x)
  { return __x.isNegative(); }

}

#endif

#endif // KISHALFTRAITS_H
