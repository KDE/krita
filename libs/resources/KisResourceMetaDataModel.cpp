/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisResourceMetaDataModel.h"

#include <QtSql>

struct KisResourceMetaDataModel::Private
{
    QSqlQuery query;
    bool isQueryPrepared = false;
};

KisResourceMetaDataModel::KisResourceMetaDataModel(const QString &tableName)
    : m_d(new Private)
{
    m_d->query.setForwardOnly(true);

    m_d->isQueryPrepared =
        m_d->query.prepare(
            "SELECT value\n"
            "FROM   metadata\n"
            "WHERE  foreign_id = :id\n"
            "AND    table_name = :table\n"
            "AND    key = :key\n"
            "LIMIT 1");

    if (!m_d->isQueryPrepared) {
        qWarning() << "Could not prepare metadata query" << m_d->query.lastError();
        return;
    } else {
        m_d->query.bindValue(":table", tableName);
    }
}

KisResourceMetaDataModel::~KisResourceMetaDataModel()
{
}

QVariant KisResourceMetaDataModel::metaDataValue(int resourceId, const QString &key)
{
    QVariant resultValue;
    
    if (!m_d->isQueryPrepared) return resultValue;

    m_d->query.bindValue(":id", resourceId);
    m_d->query.bindValue(":key", key);

    if (!m_d->query.exec()) {
        qWarning() << "Could not execute metadata query" << m_d->query.lastError();
        return resultValue;
    }

    if (m_d->query.size() > 1) {
        qWarning() << "Found duplicated entries for metadata for resource" << resourceId << "and key" << key;
    }

    if (m_d->query.first()) {
        QByteArray ba = m_d->query.value(0).toByteArray();
        if (!ba.isEmpty()) {
            QDataStream ds(QByteArray::fromBase64(ba));
            QVariant value;
            ds >> value;
            resultValue = value;
        }
    }

    return resultValue;
}
