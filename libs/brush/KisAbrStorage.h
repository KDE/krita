/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISABRSTORAGE_H
#define KISABRSTORAGE_H

#include <KisStoragePlugin.h>

#include <kritabrush_export.h>
#include <kis_abr_brush_collection.h>

class BRUSH_EXPORT KisAbrStorage : public KisStoragePlugin
{
public:
    KisAbrStorage(const QString &location);
    virtual ~KisAbrStorage();

    KisResourceStorage::ResourceItem resourceItem(const QString &url) override;

    KoResourceSP resource(const QString &url) override;
    bool loadVersionedResource(KoResourceSP resource) override;
    bool supportsVersioning() const override;
    QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) override;
    QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) override;
    QImage thumbnail() const override;
    KisAbrBrushCollectionSP m_brushCollection;
};

#endif // KISABRSTORAGE_H
