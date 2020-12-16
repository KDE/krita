/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISRESOURCETYPEMODEL_H
#define KISRESOURCETYPEMODEL_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QObject>

#include "kritaresources_export.h"

/**
 * KisResourceTypeModel provides a view on the various resource types
 * defined in the database. This should be the same list as available
 * from KisResourceLoaderRegistry. 
 */
class KRITARESOURCES_EXPORT KisResourceTypeModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum Columns {
        Id = 0,
        ResourceType,
        Name,
    };

    KisResourceTypeModel(QObject *parent = 0);
    ~KisResourceTypeModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:

    bool prepareQuery();

    struct Private;
    Private* const d;

};

#endif // KISRESOURCETYPEMODEL_H
