/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KIS_META_DATA_MODEL_H
#define KIS_META_DATA_MODEL_H
#include <QAbstractTableModel>

namespace KisMetaData
{
class Store;
}

class KisMetaDataModel : public QAbstractTableModel
{
public:
    KisMetaDataModel(KisMetaData::Store* store);
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    KisMetaData::Store* m_store;
};
#endif
