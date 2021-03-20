/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "IconImageProvider.h"
#include <QDebug>
#include "kis_icon_utils.h"

IconImageProvider::IconImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}


QImage IconImageProvider::requestImage(const QString &id, QSize */*size*/, const QSize &requestedSize)
{
    QIcon icon = KisIconUtils::loadIcon(id);
    QSize properSize = QSize(10, 10);
    if (requestedSize.isValid()) {
        properSize = requestedSize;
    }
    if (!icon.isNull()) {
        return icon.pixmap(properSize).toImage();
    }
    return QImage(properSize, QImage::Format_ARGB32);
}
