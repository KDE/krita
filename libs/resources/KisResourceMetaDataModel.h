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
