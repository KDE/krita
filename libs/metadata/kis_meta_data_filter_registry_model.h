/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
