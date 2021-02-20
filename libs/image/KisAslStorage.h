/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISASLSTORAGE_H
#define KISASLSTORAGE_H

#include <kritaimage_export.h>

#include <KisStoragePlugin.h>
#include <kis_asl_layer_style_serializer.h>

class KRITAIMAGE_EXPORT KisAslStorage : public KisStoragePlugin
{
public:
    KisAslStorage(const QString &location);
    virtual ~KisAslStorage();

    KisResourceStorage::ResourceItem resourceItem(const QString &url) override;
    KoResourceSP resource(const QString &url) override;
    bool loadVersionedResource(KoResourceSP resource) override;
    bool supportsVersioning() const override;
    QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) override;
    QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) override;

    bool addResource(const QString &resourceType, KoResourceSP resource) override;

    QSharedPointer<KisAslLayerStyleSerializer> m_aslSerializer;
};

#endif // KISASLSTORAGE_H
