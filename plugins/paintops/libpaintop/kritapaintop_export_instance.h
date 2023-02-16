/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Ref: GPL-2.0-or-later
 */

#ifndef KRITAPAINTOP_EXPORT_INSTANCE_H
#define KRITAPAINTOP_EXPORT_INSTANCE_H

#include "kritapaintop_export.h"

/* See https://reviews.llvm.org/D61118 */

#if defined(__MINGW64__) || defined(__MINGW32__)
#define PAINTOP_EXPORT_TEMPLATE PAINTOP_EXPORT
#define PAINTOP_EXPORT_INSTANCE
#elif defined(_MSC_VER)
#define PAINTOP_EXPORT_TEMPLATE
#define PAINTOP_EXPORT_INSTANCE PAINTOP_EXPORT
#else
#define PAINTOP_EXPORT_TEMPLATE PAINTOP_EXPORT
#define PAINTOP_EXPORT_INSTANCE PAINTOP_EXPORT
#endif


#endif // KRITAPAINTOP_EXPORT_INSTANCE_H
