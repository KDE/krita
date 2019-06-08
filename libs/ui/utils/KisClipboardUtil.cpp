/*
 *  Copyright (c) 2019 Dmitrii Utkin <loentar@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisClipboardUtil.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QImage>
#include <QList>
#include <QSet>
#include <QPair>

namespace KisClipboardUtil {

QImage getImageFromClipboard()
{
    static const QList<QPair<QSet<QString>, QString>> supportedFormats = {
            {{"image/png"}, "PNG"},
            {{"image/tiff"}, "TIFF"},
            {{"image/bmp", "image/x-bmp", "image/x-MS-bmp", "image/x-win-bitmap"}, "BMP"},
            {{"image/jpeg"}, "JPG"}
    };

    QClipboard *clipboard = QApplication::clipboard();

    QImage image = clipboard->image();
    if (!image.isNull()) {
        return image;
    }

    const QSet<QString> &clipboardFormats = clipboard->mimeData()->formats().toSet();

    Q_FOREACH (const auto &supportedFormat, supportedFormats) {
        const auto& intersection = supportedFormat.first & clipboardFormats;
        if (intersection.isEmpty()) {
            continue;
        }
        const QString& format = *intersection.constBegin();

        const QByteArray &imageData = clipboard->mimeData()->data(format);
        if (imageData.isEmpty()) {
            continue;
        }

        if (image.loadFromData(imageData, supportedFormat.second.toLatin1())) {
            break;
        }
    }

    return image;
}

}