/*
 * SPDX-FileCopyrightText: 2023 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISRESOURCETHUMBNAILCACHE_H_
#define __KISRESOURCETHUMBNAILCACHE_H_

#include <QImage>
#include <QScopedPointer>

#include "kritaresources_export.h"

class QModelIndex;

class KRITARESOURCES_EXPORT KisResourceThumbnailCache
{
public:
    KisResourceThumbnailCache();
    ~KisResourceThumbnailCache();

    static KisResourceThumbnailCache *instance();

    QImage getImage(const QModelIndex &index,
                    const QSize size = QSize(-1, -1),
                    Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                    Qt::TransformationMode transformMode = Qt::FastTransformation);

private:
    friend class KisResourceQueryMapper;
    friend class KisResourceLocator;
    friend class KisStorageModel;

    /*
     * Check if we have the original image in the cache.
     */
    QImage originalImage(const QString &storageLocation, const QString &resourceType, const QString &filename) const;
    void insert(const QString &storageLocation,
                const QString &resourceType,
                const QString &filename,
                const QImage &image);
    void insert(const QPair<QString, QString> &key, const QImage &image);

    void remove(const QString &storageLocation, const QString &resourceType, const QString &filename);
    void remove(const QPair<QString, QString> &key);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // __KISRESOURCETHUMBNAILCACHE_H_
