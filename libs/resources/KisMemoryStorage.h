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

#ifndef KISMEMORYSTORAGE_H
#define KISMEMORYSTORAGE_H

#include <KisStoragePlugin.h>

#include <kritaresources_export.h>

/**
 * @brief The KisMemoryStorage class stores the temporary resources
 * that are not saved to disk or bundle. It is also used to stores
 * transient per-document resources, such as the document-local palette
 * list.
 */
class KRITARESOURCES_EXPORT KisMemoryStorage : public KisStoragePlugin
{
public:
    KisMemoryStorage(const QString &location = QString("memory"));
    virtual ~KisMemoryStorage();

    /// Copying the memory storage clones all contained resources and tags
    KisMemoryStorage(const KisMemoryStorage &rhs);

    /// This clones all contained resources and tags from rhs
    KisMemoryStorage &operator=(const KisMemoryStorage &rhs);

    bool addTag(const QString &resourceType, KisTagSP tag) override;
    bool addResource(const QString &resourceType, KoResourceSP resource) override;

    KisResourceStorage::ResourceItem resourceItem(const QString &url) override;
    KoResourceSP resource(const QString &url) override;
    QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) override;
    QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) override;

    void setMetaData(const QString &key, const QVariant &value) override;
    QStringList metaDataKeys() const override;
    QVariant metaData(const QString &key) const override;


private:
    class Private;
    QScopedPointer<Private> d;

};


#endif // KISMEMORYSTORAGE_H
