/*  This file is part of the Calligra project.
    Copyright 2015 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kritacreator.h"

// Calligra
#include <KoStore.h>

// Qt
#include <QPainter>


extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new KritaCreator;
    }
}

KritaCreator::KritaCreator()
{
}

KritaCreator::~KritaCreator()
{
}

bool KritaCreator::create(const QString &path, int width, int height, QImage &image)
{
    // for now just rely on the rendered data inside the file,
    // do not load Krita code for rendering ourselves, as that currently (2.9)
    // means loading all plugins, resources etc.
    KoStore *store = KoStore::createStore(path, KoStore::Read);

    if (!store) {
        return false;
    }

    QImage thumbnail;
    bool biggerSizeNeeded = true;

    // first check if normal thumbnail is good enough
    if (// ORA thumbnail?
        store->open(QLatin1String("Thumbnails/thumbnail.png")) ||
         // KRA thumbnail?
        store->open(QLatin1String("preview.png"))) {

        const QByteArray thumbnailData = store->read(store->size());
        store->close();

        if (thumbnail.loadFromData(thumbnailData)) {
            biggerSizeNeeded = (thumbnail.width() < width || thumbnail.height() < height);
        }
    }

    // for bigger sizes see if full rendered image is available, both ORA & KRA
    if (biggerSizeNeeded && store->open(QLatin1String("mergedimage.png"))) {
        const QByteArray mergedImageData = store->read(store->size());

        QImage mergedImage;
        if (mergedImage.loadFromData(mergedImageData)) {
            thumbnail = mergedImage.scaled(width, height,
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    const bool createdThumbnail = !thumbnail.isNull();
    if (createdThumbnail) {
        // transparent areas put a white background behind the thumbnail
        // (or better checkerboard?)
        image = QImage(thumbnail.size(), QImage::Format_RGB32);
        image.fill(QColor(Qt::white).rgb());
        QPainter p(&image);
        p.drawImage(QPoint(0, 0), thumbnail);
    }

    delete store;

    return createdThumbnail;
}

ThumbCreator::Flags KritaCreator::flags() const
{
    return DrawFrame;
}
