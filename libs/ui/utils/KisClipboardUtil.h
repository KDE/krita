/*
 *  SPDX-FileCopyrightText: 2019 Dmitrii Utkin <loentar@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CLIPBOARD_UTIL_H
#define KIS_CLIPBOARD_UTIL_H

#include <kritaui_export.h>
#include <kis_types.h>

class KisView;
class QMimeData;
class QPoint;

namespace KisClipboardUtil {
    /**
     * Show popup on Kismage when clipboard contents are urls
     * False if clipboard contents are not urls
     * @return void
     */
    KRITAUI_EXPORT void clipboardHasUrlsAction(KisView *kisview, const QMimeData *data, QPoint eventPos);
    };

#endif //KIS_CLIPBOARD_UTIL_H
