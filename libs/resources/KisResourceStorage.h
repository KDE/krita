/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISRESOURCESTORAGE_H
#define KISRESOURCESTORAGE_H

#include <QSharedPointer>
#include <QScopedPointer>
#include <QString>
#include <QDateTime>


#include <KoResource.h>

#include <kritaresources_export.h>

/**
 * The KisResourceStorage class is the base class for
 * places where resources can be stored. Examples are
 * folders, bundles or Adobe resource libraries like
 * ABR files.
 */
class KRITARESOURCES_EXPORT KisResourceStorage
{
public:

    struct ResourceItem {
        QString url;
        QDateTime lastModified;
    };

    enum class StorageType : int {
        Unknown = 0,
        Folder = 1,
        Bundle = 2,
        AdobeBrushLibrary = 3,
        AdobeStyleLibrary = 4
    };

    KisResourceStorage(const QString &location);
    ~KisResourceStorage();

    QString name() const;
    QString location() const;
    bool valid() const;
    StorageType type() const;
    QDateTime timestamp() const;

    KoResourceSP resource(const QString &url);
    QVector<KoResourceSP> resources(const QString &resourceType);

private:
    class Private;
    QScopedPointer<Private> d;
};


typedef QSharedPointer<KisResourceStorage> KisResourceStorageSP;

#endif // KISRESOURCESTORAGE_H
