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

#include "KisBundleStorage.h"

#include "KisResourceStorage.h"

class BundleTagIterator : public KisResourceStorage::TagIterator
{
public:

    BundleTagIterator(const QString &location, const QString &resourceType)
        : m_location(location)
        , m_resourceType(resourceType)
    {}

    bool hasNext() const override {return false; }
    void next() const override {}

    QString url() const override { return QString(); }
    QString name() const override { return QString(); }
    QString comment() const override {return QString(); }


private:

    QString m_location;
    QString m_resourceType;
};

class BundleIterator : public KisResourceStorage::ResourceIterator
{
public:
    bool hasNext() const override {return false; }
    void next() const override {}

    QString url() const override { return QString(); }
    QString type() const override { return QString(); }
    QDateTime lastModified() const override { return QDateTime(); }
    /// This only loads the resource when called
    QByteArray md5sum() const override { return QByteArray(); }
    /// This only loads the resource when called
    KoResourceSP resource() const override { return 0; }
};

KisBundleStorage::KisBundleStorage(const QString &location)
    : KisStoragePlugin(location)
{
}

KisBundleStorage::~KisBundleStorage()
{
}

KisResourceStorage::ResourceItem KisBundleStorage::resourceItem(const QString &url)
{
    return KisResourceStorage::ResourceItem();
}

KoResourceSP KisBundleStorage::resource(const QString &url)
{
    return 0;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisBundleStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new BundleIterator);
}

QSharedPointer<KisResourceStorage::TagIterator> KisBundleStorage::tags(const QString &resourceType)
{
return QSharedPointer<KisResourceStorage::TagIterator>(new BundleTagIterator(location(), resourceType));
}

