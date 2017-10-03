/* This file is part of the KDE project
 * Copyright (C) 2012 KO GmbH. Contact: Boudewijn Rempt <boud@kogmbh.com>
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
#ifndef KRITA_SKETCH_PAGETHUMBNAILPROVIDER_H
#define KRITA_SKETCH_PAGETHUMBNAILPROVIDER_H

#include <QQuickImageProvider>
#include <QObject>

class PageThumbnailProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    PageThumbnailProvider();
    virtual ~PageThumbnailProvider();

    virtual QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);

    void addThumbnail(QString id, QImage thumb);
    bool hasThumbnail(QString id);

private:
    class Private;
    Private* d;
};

#endif // CMPAGETHUMBNAILPROVIDER_H
