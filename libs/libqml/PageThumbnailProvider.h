/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 KO GmbH. Contact : Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
