/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef _KIS_FILTERS_MODEL_H_
#define _KIS_FILTERS_MODEL_H_

#include <QAbstractItemModel>

#include <kis_types.h>

class KisFilter;

/**
 *
 */
class KisFiltersModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    /**
     * @brief KisFiltersModel create a new filters model
     * @param showAll if true, show all filters, if false, do not show filters that don't work in adjustment layers
     * @param thumb the thumbnail image that is filtered
     */
    KisFiltersModel(bool showAll, KisPaintDeviceSP thumb);
    ~KisFiltersModel() override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QModelIndex indexForFilter(const QString& id);
    const KisFilter* indexToFilter(const QModelIndex& idx);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
private:
    struct Private;
    Private* const d;
};

#endif
