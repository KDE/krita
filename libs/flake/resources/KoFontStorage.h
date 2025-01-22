/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFONTSTORAGE_H
#define KOFONTSTORAGE_H

#include <KisResourceStorage.h>
#include <kritaflake_export.h>
#include <KisStoragePlugin.h>

class KRITAFLAKE_EXPORT KoFontStorage: public KisStoragePlugin
{
public:
    KoFontStorage(const QString &location = "fontregistery");
    virtual ~KoFontStorage();

    KisResourceStorage::ResourceItem resourceItem(const QString &url) override;
    KoResourceSP resource(const QString &url) override;

    bool supportsVersioning() const override;
    QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) override;
    QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) override;

    bool isValid() const override;

    bool loadVersionedResource(KoResourceSP resource) override;
};

#endif // KOFONTSTORAGE_H
