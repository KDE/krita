/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOIMAGEDATA_P_H
#define KOIMAGEDATA_P_H

#include <QSharedData>

#include <QByteArray>
#include <QImage>
#include <QPixmap>
#include <QSize>

#include "KoImageData.h"

class KoImageCollection;


class KoImageDataPrivate : public QSharedData
{
public:
    KoImageDataPrivate();
    ~KoImageDataPrivate();

    bool saveToFile(QIODevice & device);

    KoImageCollection *collection;
    KoImageData::ImageQuality quality; // TODO remove
    KoImageData::ErrorCode errorCode;
    QSizeF imageSize;
    QPixmap pixmap;
    QImage image; // this member holds the data in case the image is embedded.
    QByteArray key; // key to identify the picture
    QByteArray rawData; // the raw data of the picture either from the file or store
    QString suffix; // the suffix of the picture e.g. png
};

#endif /* KOIMAGEDATA_P_H */
