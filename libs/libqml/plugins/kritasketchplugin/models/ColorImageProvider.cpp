/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ColorImageProvider.h"

ColorImageProvider::ColorImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QPixmap ColorImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    int width = 100;
    int height = 50;

    if ( size )
        *size = QSize(width, height);
    QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
                   requestedSize.height() > 0 ? requestedSize.height() : height);
    if (QColor::isValidColor(id))
    {
        pixmap.fill(QColor(id).rgba());
    }
    else
    {
        QList<QString> elements = id.split(",");
        if (elements.count() == 4)
            pixmap.fill(QColor::fromRgbF(elements.at(0).toFloat(), elements.at(1).toFloat(), elements.at(2).toFloat(), elements.at(3).toFloat()));
        if (elements.count() == 3)
            pixmap.fill(QColor::fromRgbF(elements.at(0).toFloat(), elements.at(1).toFloat(), elements.at(2).toFloat()));
    }
    return pixmap;
}
