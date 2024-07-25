/*
 *  SPDX-FileCopyrightText: 2020 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisResourceQueryMapper.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QFont>
#include <QImage>
#include <QSqlError>
#include <QString>
#include <QVariant>

#include "KisResourceLocator.h"
#include "KisResourceModel.h"
#include "KisResourceModelProvider.h"
#include "KisResourceThumbnailCache.h"
#include "KisTag.h"
#include "kis_assert.h"

QImage KisResourceQueryMapper::getThumbnailFromQuery(const QSqlQuery &query, bool useResourcePrefix)
{
    const QString storageLocation =
        KisResourceLocator::instance()->makeStorageLocationAbsolute(query.value("location").toString());
    const QString resourceType = query.value("resource_type").toString();
    const QString filename = query.value(useResourcePrefix ? "resource_filename" : "filename").toString();

    // NOTE: Only use the private methods of KisResourceThumbnailCache here to prevent any chances of
    // recursion.
    QImage img =
        KisResourceThumbnailCache::instance()->originalImage(storageLocation, resourceType, filename);
    if (!img.isNull()) {
        return img;
    } else {
        const int resourceId = query.value(useResourcePrefix ? "resource_id" : "id").toInt();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resourceId >= 0, img);

        bool result = false;
        QSqlQuery thumbQuery;
        result = thumbQuery.prepare("SELECT thumbnail FROM resources WHERE resources.id = :resource_id");
        if (!result) {
            qWarning() << "Failed to prepare query for thumbnail of" << resourceId << thumbQuery.lastError();
            return img;
        }

        thumbQuery.bindValue(":resource_id", resourceId);

        result = thumbQuery.exec();

        if (!result) {
            qWarning() << "Failed to execute query for thumbnail of" << resourceId << thumbQuery.lastError();
            return img;
        }

        if (!thumbQuery.next()) {
            qWarning() << "Failed to find thumbnail of" << resourceId;
            return img;
        }

        QByteArray ba = thumbQuery.value("thumbnail").toByteArray();
        QBuffer buf(&ba);
        buf.open(QBuffer::ReadOnly);
        img.load(&buf, "PNG");

        KisResourceThumbnailCache::instance()->insert(storageLocation, resourceType, filename, img);
        return img;
    }
}

QVariant KisResourceQueryMapper::variantFromResourceQuery(const QSqlQuery &query, int column, int role, bool useResourcePrefix)
{
    const QString resourceType = query.value("resource_type").toString();

    switch(role) {
    case Qt::FontRole:
        return QFont();
    case Qt::DisplayRole:
    {
        switch(column) {
        case KisAbstractResourceModel::Id:
            return query.value(useResourcePrefix ? "resource_id" : "id");
        case KisAbstractResourceModel::StorageId:
            return query.value("storage_id");
        case KisAbstractResourceModel::Name:
            return query.value(useResourcePrefix ? "resource_name" : "name");
        case KisAbstractResourceModel::Filename:
            return query.value(useResourcePrefix ? "resource_filename" : "filename");
        case KisAbstractResourceModel::Tooltip:
            return query.value(useResourcePrefix ? "resource_tooltip" : "tooltip");
        case KisAbstractResourceModel::Thumbnail:
        {
            return QVariant::fromValue<QImage>(getThumbnailFromQuery(query, useResourcePrefix));
        }
        case KisAbstractResourceModel::Status:
            return query.value(useResourcePrefix ? "resource_active" : "status");
        case KisAbstractResourceModel::Location:
            return query.value("location");
        case KisAbstractResourceModel::ResourceType:
            return query.value("resource_type");
        case KisAbstractResourceModel::Dirty:
        {
            QString storageLocation = query.value("location").toString();
            QString filename = query.value(useResourcePrefix ? "resource_filename" : "filename").toString();

            // An uncached resource has not been loaded, so it cannot be dirty
            if (!KisResourceLocator::instance()->resourceCached(storageLocation, resourceType, filename)) {
                return false;
            }
            else {
                // Now we have to check the resource, but that's cheap since it's been loaded in any case
                KoResourceSP resource = KisResourceLocator::instance()->resourceForId(query.value(useResourcePrefix ? "resource_id" : "id").toInt());
                return resource->isDirty();
            }
        }
        case KisAbstractResourceModel::ResourceActive:
            return query.value("resource_active");
        case KisAbstractResourceModel::StorageActive:
            return query.value(useResourcePrefix ? "resource_storage_active" : "storage_active");
        default:
            ;
        };
        Q_FALLTHROUGH();
    }
    case Qt::DecorationRole:
    {
        if (column == KisAbstractResourceModel::Thumbnail) {
            return QVariant::fromValue<QImage>(getThumbnailFromQuery(query, useResourcePrefix));
        }
        return QVariant();
    }
    case Qt::CheckStateRole: {
        switch (column) {
        case KisAbstractResourceModel::Status:
            if (query.value(useResourcePrefix ? "resource_active" : "status").toInt() == 0) {
                return Qt::Unchecked;
            } else {
                return Qt::Checked;
            }
        case KisAbstractResourceModel::Dirty: {
            const QString storageLocation = query.value("location").toString();
            const QString filename = query.value(useResourcePrefix ? "resource_filename" : "filename").toString();

            // An uncached resource has not been loaded, so it cannot be dirty
            if (!KisResourceLocator::instance()->resourceCached(storageLocation, resourceType, filename)) {
                return Qt::Unchecked;
            } else {
                // Now we have to check the resource, but that's cheap since it's been loaded in any case
                KoResourceSP resource = KisResourceLocator::instance()->resourceForId(
                    query.value(useResourcePrefix ? "resource_id" : "id").toInt());
                return resource->isDirty() ? Qt::Checked : Qt::Unchecked;
            }
        }
        case KisAbstractResourceModel::ResourceActive:
            if (query.value("resource_active").toInt() == 0) {
                return Qt::Unchecked;
            } else {
                return Qt::Checked;
            }
        case KisAbstractResourceModel::StorageActive:
            if (query.value(useResourcePrefix ? "resource_storage_active" : "storage_active").toInt() == 0) {
                return Qt::Unchecked;
            } else {
                return Qt::Checked;
            }
        default:
            return {};
        };
    }
    case Qt::StatusTipRole:
        return QVariant();
    case Qt::ToolTipRole:
        Q_FALLTHROUGH();
    case Qt::WhatsThisRole:
        return query.value("tooltip");
    case Qt::UserRole + KisAbstractResourceModel::Id:
        return query.value(useResourcePrefix ? "resource_id" : "id");
    case Qt::UserRole + KisAbstractResourceModel::StorageId:
        return query.value("storage_id");
    case Qt::UserRole + KisAbstractResourceModel::Name:
        return query.value(useResourcePrefix ? "resource_name" : "name");
    case Qt::UserRole + KisAbstractResourceModel::Filename:
        return query.value(useResourcePrefix ? "resource_filename" : "filename");
    case Qt::UserRole + KisAbstractResourceModel::Tooltip:
        return query.value(useResourcePrefix ? "resource_tooltip" : "tooltip");
    case Qt::UserRole + KisAbstractResourceModel::MD5:
        return query.value(useResourcePrefix ? "resource_md5sum" : "md5sum");
    case Qt::UserRole + KisAbstractResourceModel::Thumbnail:
    {
        return QVariant::fromValue<QImage>(getThumbnailFromQuery(query, useResourcePrefix));
    }
    case Qt::UserRole + KisAbstractResourceModel::Status:
        return query.value(useResourcePrefix ? "resource_active" : "status");
    case Qt::UserRole + KisAbstractResourceModel::Location:
        return query.value("location");
    case Qt::UserRole + KisAbstractResourceModel::ResourceType:
        return query.value("resource_type");
    case Qt::UserRole + KisAbstractResourceModel::Tags:
    {
        KisAllResourcesModel *resourceModel = KisResourceModelProvider::resourceModel(resourceType);
        QStringList tagNames;
        Q_FOREACH(const KisTagSP tag, resourceModel->tagsForResource(query.value(useResourcePrefix ? "resource_id" : "id").toInt())) {
            tagNames << tag->name();
        }
        return tagNames;
    }
    case Qt::UserRole + KisAbstractResourceModel::Dirty:
    {
        QString storageLocation = query.value("location").toString();
        QString filename = query.value(useResourcePrefix ? "resource_filename" : "filename").toString();

        // An uncached resource has not been loaded, so it cannot be dirty
        if (!KisResourceLocator::instance()->resourceCached(storageLocation, resourceType, filename)) {
            return false;
        }
        else {
            // Now we have to check the resource, but that's cheap since it's been loaded in any case
            KoResourceSP resource = KisResourceLocator::instance()->resourceForId(query.value(useResourcePrefix ? "resource_id" : "id").toInt());
            return resource->isDirty();
        }
    }
    case Qt::UserRole + KisAbstractResourceModel::MetaData:
    {
        QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForResource(query.value(useResourcePrefix ? "resource_id" : "id").toInt());
        return r;
    }
    case Qt::UserRole + KisAbstractResourceModel::ResourceActive:
    {
        return query.value("resource_active");
    }
    case Qt::UserRole + KisAbstractResourceModel::StorageActive:
    {
        return query.value(useResourcePrefix ? "resource_storage_active" : "storage_active");
    }
    default:
        ;
    }

    return QVariant();
}

