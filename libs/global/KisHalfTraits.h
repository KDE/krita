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
#include <halfLimits.h>

#include <type_traits>

namespace std {

template<>
  struct __is_integral_helper<half>
  : public false_type { };

template<>
  struct __is_floating_point_helper<half>
  : public true_type { };

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
