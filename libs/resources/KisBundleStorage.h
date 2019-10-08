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

#ifndef KISBUNDLESTORAGE_H
#define KISBUNDLESTORAGE_H

#include <KisStoragePlugin.h>
#include "kritaresources_export.h"

class KRITARESOURCES_EXPORT KisBundleStorage : public KisStoragePlugin
{
public:
    KisBundleStorage(const QString &location);
    virtual ~KisBundleStorage();

    KisResourceStorage::ResourceItem resourceItem(const QString &url) override;
    KoResourceSP resource(const QString &url) override;
    QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) override;
    QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) override;
    QImage thumbnail() const override;
    QStringList metaDataKeys() const override;
    QString metaData(const QString &key) const override;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // KISBUNDLESTORAGE_H
