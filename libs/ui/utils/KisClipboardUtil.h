/*
 *  SPDX-FileCopyrightText: 2019 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CLIPBOARD_UTIL_H
#define KIS_CLIPBOARD_UTIL_H

#include <kritaui_export.h>

class QImage;

namespace KisClipboardUtil {

    /**
     * load an image from clipboard handling different supported formats
     * @return image
     */
    KRITAUI_EXPORT QImage getImageFromClipboard();

}


#endif //KIS_CLIPBOARD_UTIL_H
