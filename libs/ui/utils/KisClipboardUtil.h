/*
 *  SPDX-FileCopyrightText: 2019 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CLIPBOARD_UTIL_H
#define KIS_CLIPBOARD_UTIL_H

#include <kritaui_export.h>
#include <kis_types.h>

class QImage;
class QMimeData;

class KisView;

namespace KisClipboardUtil {

    /**
     * load an image from clipboard handling different supported formats
     * @return image
     */
    KRITAUI_EXPORT QImage getImageFromClipboard();

    bool clipboardHasUrls();

    /**
     * Show popup on Kismage when clipboard contents are urls
     * False if clipboard contents are not urls
     * @return void
     */
    KRITAUI_EXPORT void clipboardHasUrlsAction(KisView *kisview, const QMimeData *data, QPoint eventPos);

    KRITAUI_EXPORT KisPaintDeviceSP fetchImageByURL(const QUrl &url);

}


#endif //KIS_CLIPBOARD_UTIL_H
