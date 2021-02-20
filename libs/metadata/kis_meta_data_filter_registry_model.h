/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_META_DATA_FILTER_REGISTRY_MODEL_H_
#define _KIS_META_DATA_FILTER_REGISTRY_MODEL_H_

#include "kis_meta_data_filter_registry.h"

#include <KoGenericRegistryModel.h>

class QStringList;

namespace KisMetaData
{

/**
 * Use this model to display a list of filters (KisMetaData::Filter) that can be
 * enabled or disabled.
 */
class KRITAMETADATA_EXPORT FilterRegistryModel : public KoGenericRegistryModel<const Filter*>
{
public:
    FilterRegistryModel();
    ~FilterRegistryModel() override;
public:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    /// @return a list of filters that are enabled
    QList<const Filter*> enabledFilters() const;
    /// enable the filters in the given list; others will be disabled.
    virtual void setEnabledFilters(const QStringList &enabledFilters);
private:
    struct Private;
    Private* const d;
};

}

#endif
