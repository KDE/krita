/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Ref: GPL-2.0-or-later
 */

#ifndef KIS_IMAGE_EXPORT
#define KIS_IMAGE_EXPORT

#include "kritaimage_export.h"

/* See https://reviews.llvm.org/D61118 */

#if defined(__MINGW64__) || defined(__MINGW32__)
#define KRITAIMAGE_EXPORT_TEMPLATE KRITAIMAGE_EXPORT
#define KRITAIMAGE_EXPORT_INSTANCE
#elif defined(_MSC_VER)
#define KRITAIMAGE_EXPORT_TEMPLATE
#define KRITAIMAGE_EXPORT_INSTANCE KRITAIMAGE_EXPORT
#else
#define KRITAIMAGE_EXPORT_TEMPLATE KRITAIMAGE_EXPORT
#define KRITAIMAGE_EXPORT_INSTANCE KRITAIMAGE_EXPORT
#endif

#endif
