/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Ref: GPL-2.0-or-later
 */

#ifndef KRITAUI_EXPORT_INSTANCE_H
#define KRITAUI_EXPORT_INSTANCE_H

#include "kritaui_export.h"

/* See https://reviews.llvm.org/D61118 */

#if defined(__MINGW64__) || defined(__MINGW32__)
#define KRITAUI_EXPORT_TEMPLATE KRITAUI_EXPORT
#define KRITAUI_EXPORT_INSTANCE
#elif defined(_MSC_VER)
#define KRITAUI_EXPORT_TEMPLATE
#define KRITAUI_EXPORT_INSTANCE KRITAUI_EXPORT
#else
#define KRITAUI_EXPORT_TEMPLATE KRITAUI_EXPORT
#define KRITAUI_EXPORT_INSTANCE KRITAUI_EXPORT
#endif

#endif // KRITAUI_EXPORT_INSTANCE_H
