/*
 * Copyright (c) 2019 boud <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KISSTORAGEMODEL_H
#define KISSTORAGEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QScopedPointer>

#include "KisResourceStorage.h"
#include "kritaresources_export.h"

class KRITARESOURCES_EXPORT KisStorageModel : public QAbstractTableModel
{
public:

    enum Columns {
        Id = 0,
        StorageType,
        Location,
        TimeStamp,
        PreInstalled,
        Active,
        Thumbnail
    };

    static KisStorageModel * instance();

    KisStorageModel(QObject *parent = 0);
    ~KisStorageModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    KisResourceStorageSP storageForId(const QModelIndex &index) const;

private:

    KisStorageModel(const KisStorageModel&);
    KisStorageModel operator=(const KisStorageModel&);

    bool prepareQuery();

    struct Private;
    QScopedPointer<Private> d;


};

#endif // KISSTORAGEMODEL_H
