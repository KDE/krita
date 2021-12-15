/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisResourceQueryMapper.h"

#include <QString>
#include <QVariant>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QSqlError>

#include "KisResourceModel.h"
#include "KisResourceLocator.h"
#include "KisResourceModelProvider.h"
#include "KisTag.h"



QImage KisResourceQueryMapper::getThumbnailFromQuery(const QSqlQuery &query, bool useResourcePrefix)
{
    QString storageLocation = query.value("location").toString();
    QString resourceType = query.value("resource_type").toString();
    QString filename = query.value(useResourcePrefix ? "resource_filename" : "filename").toString();

    QImage img = KisResourceLocator::instance()->thumbnailCached(storageLocation, resourceType, filename);
    if (!img.isNull()) {
        return img;
    } else {
        QByteArray ba = query.value(useResourcePrefix ? "resource_thumbnail" : "thumbnail").toByteArray();
        QBuffer buf(&ba);
        buf.open(QBuffer::ReadOnly);
        img.load(&buf, "PNG");
        KisResourceLocator::instance()->cacheThumbnail(storageLocation, resourceType, filename, img);
        return img;
    }
}

QVariant KisResourceQueryMapper::variantFromResourceQuery(const QSqlQuery &query, int column, int role, bool useResourcePrefix)
{
    const QString resourceType = query.value("resource_type").toString();

    switch(role) {
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
            return query.value(useResourcePrefix ? "resource_status" : "status");
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
            return query.value("storage_active");
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
        return query.value(useResourcePrefix ? "resource_status" : "status");
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
        return query.value("storage_active");
    }
    default:
        ;
    }

    return QVariant();
}

