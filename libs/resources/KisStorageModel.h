/*
 * SPDX-FileCopyrightText: 2019 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSTORAGEMODEL_H
#define KISSTORAGEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QScopedPointer>

#include "KisResourceStorage.h"
#include "kritaresources_export.h"

/**
 * KisStorageModel provides a model of all registered storages, like
 * the folder storages, the bundle storages or the memory storages. Note
 * that inactive storages are also part of this model.
 */
class KRITARESOURCES_EXPORT KisStorageModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum Columns {
        Id = 0,
        StorageType,
        Location,
        TimeStamp,
        PreInstalled,
        Active,
        Thumbnail,
        DisplayName,
        MetaData
    };

    KisStorageModel(QObject *parent = 0);
    ~KisStorageModel() override;

    static KisStorageModel * instance();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    /// Enable and disable the storage represented by index
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    KisResourceStorageSP storageForIndex(const QModelIndex &index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

Q_SIGNALS:

    void storageEnabled(const QString &storage);
    void storageDisabled(const QString &storage);

private Q_SLOTS:

    /// Called whenever a storage is added
    void addStorage(const QString &location);

    /// This is called when a storage really is deleted both from database and anywhere else
    void removeStorage(const QString &location);

private:

    KisStorageModel(const KisStorageModel&);
    KisStorageModel operator=(const KisStorageModel&);

    struct Private;
    QScopedPointer<Private> d;


};

#endif // KISSTORAGEMODEL_H
