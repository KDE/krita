/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
    if (icon.isNull()) {
        QImage img = QImage(requestedSize, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        return img;
    }
    return icon.pixmap(requestedSize).toImage();
}
