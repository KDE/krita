/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PAINT_DEVICE_DEBUG_UTILS_H
#define __KIS_PAINT_DEVICE_DEBUG_UTILS_H


class QRect;
class QString;

#include <kis_types.h>
#include <kritaimage_export.h>

void KRITAIMAGE_EXPORT kis_debug_save_device_incremental(KisPaintDeviceSP device,
                                                         int i,
                                                         const QRect &rc,
                                                         const QString &suffix, const QString &prefix);

/**
 * Saves the paint device incrementally. Put this macro into a
 * function that is called several times and you'll have as many
 * separate dump files as the number of times the function was
 * called. That is very convenient for debugging canvas updates:
 * adding this macro will let you track the whole history of updates.
 *
 * The files are saved with pattern: \<counter\>_\<suffix\>.png
 */
#define KIS_DUMP_DEVICE_1(device, rc, suffix)                           \
    do {                                                                \
        static int i = -1; i++;                                         \
        kis_debug_save_device_incremental((device), i, (rc), (suffix), QString()); \
    } while(0)

/**
 * Saves the paint device incrementally. Put this macro into a
 * function that is called several times and you'll have as many
 * separate dump files as the number of times the function was
 * called. That is very convenient for debugging canvas updates:
 * adding this macro will let you track the whole history of updates.
 *
 * The \p prefix parameter makes it easy to sort out dumps from
 * different functions.
 *
 * The files are saved with pattern: \<prefix\>_\<counter\>_\<suffix\>.png
 */
#define KIS_DUMP_DEVICE_2(device, rc, suffix, prefix)                   \
    do {                                                                \
        static int i = -1; i++;                                         \
        kis_debug_save_device_incremental((device), i, (rc), (suffix), (prefix)); \
    } while(0)

#endif /* __KIS_PAINT_DEVICE_DEBUG_UTILS_H */
