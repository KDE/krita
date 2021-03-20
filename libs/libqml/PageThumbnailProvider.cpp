/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 KO GmbH. Contact : Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "PageThumbnailProvider.h"

#include <QHash>

class PageThumbnailProvider::Private
{
public:
    Private() { }
    ~Private() { }

    QHash<QString, QImage> thumbnails;
};

PageThumbnailProvider::PageThumbnailProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , d(new Private)
{

}

PageThumbnailProvider::~PageThumbnailProvider()
{
    delete(d);
}

QImage PageThumbnailProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);
    if (d->thumbnails.contains(id))
        return d->thumbnails[id];
    return QImage();
}

void PageThumbnailProvider::addThumbnail(QString id, QImage thumb)
{
    d->thumbnails[id] = thumb;
}

bool PageThumbnailProvider::hasThumbnail(QString id)
{
    return d->thumbnails.contains(id);
}
