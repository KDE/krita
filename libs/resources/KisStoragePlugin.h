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

#ifndef KISSTORAGEPLUGIN_H
#define KISSTORAGEPLUGIN_H

#include <QScopedPointer>
#include <QString>

#include <KisResourceStorage.h>
#include "kritaresources_export.h"

/**
 * The KisStoragePlugin class is the base class
 * for storage plugins. A storage plugin is used by
 * KisResourceStorage to locate resources and tags in
 * a kind of storage, like a folder, a bundle or an adobe
 * resource library.
 */
class KRITARESOURCES_EXPORT KisStoragePlugin
{
public:
    KisStoragePlugin(const QString &location);
    virtual ~KisStoragePlugin();

    virtual KisResourceStorage::ResourceItem resourceItem(const QString &url) = 0;
    virtual KoResourceSP resource(const QString &url) = 0;
    virtual QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) = 0;
    virtual QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) = 0;
    virtual bool addTag(const QString &resourceType, KisTagSP tag) {Q_UNUSED(resourceType); Q_UNUSED(tag); return false;}
    virtual bool addResource(const QString &resourceType, KoResourceSP resource) {Q_UNUSED(resourceType); Q_UNUSED(resource); return false;}

    virtual QImage thumbnail() const { return QImage(); }

    virtual QStringList metaDataKeys() const { return QStringList(); }
    virtual QString metaData(const QString &key) const { Q_UNUSED(key); return QString(); }

protected:
    friend class TestBundleStorage;
    QString location() const;
private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // KISSTORAGEPLUGIN_H
