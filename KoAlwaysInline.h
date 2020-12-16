/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KO_ALWAYS_INLINE_H
#define __KO_ALWAYS_INLINE_H

#ifndef ALWAYS_INLINE
#if defined __GNUC__
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#elif defined _MSC_VER
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE inline
#endif
#endif

#endif /* __KO_ALWAYS_INLINE_H */
