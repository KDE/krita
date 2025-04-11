/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISRESOURCEMETADATAMODEL_H
#define KISRESOURCEMETADATAMODEL_H

#include <QObject>
#include <QScopedPointer>

#include "kritaresources_export.h"

/**
 * \class KisResourceMetaDataModel is a simple class used for fetching
 * specific metadata for a resource. Internally it stores a prepared query
 * that can fetch particular metadata keys in an efficiant way.
 *
 * In contrast to \ref KisAbstractResourceModel::MetaData role in
 * the resource model this class allows fetching **individual** keys
 * from the resource's metadata. That is usually more efficient in
 * some cases.
 *
 * The rule of thumb: if you want to fetch **all** metadata for resource,
 * use \ref KisAbstractResourceModel::MetaData, if you want an individual
 * key, use \ref KisResourceMetaDataModel.
 */
class KRITARESOURCES_EXPORT KisResourceMetaDataModel
{
public:
    KisResourceMetaDataModel(const QString &tableName);
    ~KisResourceMetaDataModel();

    QVariant metaDataValue(int resourceId, const QString &key);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISRESOURCEMETADATAMODEL_H
