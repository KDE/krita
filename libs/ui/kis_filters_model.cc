/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filters_model.h"

#include <QPixmap>

#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <kis_selection.h>

struct KisFiltersModel::Private {
    struct Node {

        virtual ~Node() {}

        QString name;
        QString displayRole() {
            return name;
        }
        virtual int childrenCount() = 0;

    };
    struct Filter : public Node {

        ~Filter() override {}

        QString id;
        QPixmap icon;
        KisFilterSP filter;
        int childrenCount() override {
            return 0;
        }
    };
    struct Category : public Node {

        ~Category() override {}

        QString id;
        QList<Filter> filters;
        int childrenCount() override {
            return filters.count();
        }
    };

    QHash<QString, Category> categories;
    QList<QString> categoriesKeys;
    KisPaintDeviceSP thumb;
};

KisFiltersModel::KisFiltersModel(bool showAll, KisPaintDeviceSP thumb)
    : d(new Private)
{
    d->thumb = thumb;
    QStringList keys = KisFilterRegistry::instance()->keys();
    keys.sort();

    Q_FOREACH (const QString &filterName, keys) {
        KisFilterSP filter = KisFilterRegistry::instance()->get(filterName);
        if (!showAll && !filter->supportsAdjustmentLayers()) {
            continue;
        }
        Q_ASSERT(filter);
        if (!d->categories.contains(filter->menuCategory().id())) {
            Private::Category cat;
            cat.id = filter->menuCategory().id();
            cat.name = filter->menuCategory().name();
            d->categories[ cat.id ] = cat;
            d->categoriesKeys.append(cat.id);
        }
        Private::Filter filt;
        filt.id = filter->id();
        filt.name = filter->name();
        filt.filter = filter;
        d->categories[filter->menuCategory().id()].filters.append(filt);
    }
    std::sort(d->categoriesKeys.begin(), d->categoriesKeys.end());

}

KisFiltersModel::~KisFiltersModel()
{
    delete d;
}

int KisFiltersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        Private::Node* node = static_cast<Private::Node*>(parent.internalPointer());
        return node->childrenCount();
    } else {
        return d->categoriesKeys.count();
    }
}

int KisFiltersModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex KisFiltersModel::indexForFilter(const QString& id)
{
    for (int i = 0; i < d->categoriesKeys.size(); i++) {
        KisFiltersModel::Private::Category& category = d->categories[ d->categoriesKeys[ i ] ];
        for (int j = 0; j < category.filters.size(); j++) {
            KisFiltersModel::Private::Filter& filter = category.filters[j];
            if (filter.id == id) {
                return index(j, i, index(i , 0, QModelIndex()));
            }
        }
    }
    return QModelIndex();
}

const KisFilter* KisFiltersModel::indexToFilter(const QModelIndex& idx)
{
    Private::Node* node = static_cast<Private::Node*>(idx.internalPointer());
    Private::Filter* filter = dynamic_cast<Private::Filter*>(node);
    if (filter) {
        return filter->filter;
    }
    return 0;
}

QModelIndex KisFiltersModel::index(int row, int column, const QModelIndex &parent) const
{
//     dbgKrita << parent.isValid() << row << endl;
    if (parent.isValid()) {
        Private::Category* category = static_cast<Private::Category*>(parent.internalPointer());
        return createIndex(row, column, &category->filters[row]);
    } else {
        return createIndex(row, column, &d->categories[ d->categoriesKeys[row] ]);
    }
}

QModelIndex KisFiltersModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    Private::Node* node = static_cast<Private::Node*>(child.internalPointer());
    Private::Filter* filter = dynamic_cast<Private::Filter*>(node);
    if (filter) {
        QString catId = filter->filter->menuCategory().id();
        return createIndex(d->categoriesKeys.indexOf(catId) , 0, &d->categories[ catId ]);
    }
    return QModelIndex(); // categories don't have parents
}

QVariant KisFiltersModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            Private::Node* node = static_cast<Private::Node*>(index.internalPointer());
            return QVariant(node->displayRole());
        }
    }
    return QVariant();
}

Qt::ItemFlags KisFiltersModel::flags(const QModelIndex & index) const
{
    if (!index.isValid()) return QFlags<Qt::ItemFlag>();

    Private::Node* node = static_cast<Private::Node*>(index.internalPointer());
    Private::Filter* filter = dynamic_cast<Private::Filter*>(node);
    if (filter) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    } else {
        return Qt::ItemIsEnabled;
    }
}


