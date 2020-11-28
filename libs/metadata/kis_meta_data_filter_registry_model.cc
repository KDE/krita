/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_filter_registry_model.h"
#include "kis_debug.h"
#include <QStringList>

using namespace KisMetaData;

struct Q_DECL_HIDDEN FilterRegistryModel::Private {
    QList<bool> enabled;
};

FilterRegistryModel::FilterRegistryModel()
        : KoGenericRegistryModel<const Filter*>(FilterRegistry::instance()), d(new Private)
{
    QList<QString> keys = FilterRegistry::instance()->keys();
    for (int i = 0; i < keys.size(); i++) {
        d->enabled.append(FilterRegistry::instance()->get(keys[i])->defaultEnabled());
    }
}

FilterRegistryModel::~FilterRegistryModel()
{
    delete d;
}

QVariant FilterRegistryModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::CheckStateRole) {
            if (d->enabled[index.row()]) return Qt::Checked;
            else return Qt::Unchecked;
        } else if (role == Qt::ToolTipRole) {
            return get(index)->description();
        }
    }
    return KoGenericRegistryModel<const Filter*>::data(index, role);
}

bool FilterRegistryModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid()) {
        if (role == Qt::CheckStateRole) {
            d->enabled[index.row()] = value.toBool();
        }
    }
    return KoGenericRegistryModel<const Filter*>::setData(index, value, role);
}

Qt::ItemFlags FilterRegistryModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

QList<const Filter*> FilterRegistryModel::enabledFilters() const
{
    QList<const Filter*> enabledFilters;
    QList<QString> keys = FilterRegistry::instance()->keys();
    for (int i = 0; i < keys.size(); i++) {
        if (d->enabled[i]) {
            enabledFilters.append(FilterRegistry::instance()->get(keys[i]));
        }
    }
    return enabledFilters;
}

void FilterRegistryModel::setEnabledFilters(const QStringList &enabledFilters)
{
    d->enabled.clear();
    QList<QString> keys = FilterRegistry::instance()->keys();
    Q_FOREACH (const QString &key, keys) {
        d->enabled.append(enabledFilters.contains(key));
    }

}
