/*
 * This file is part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

        virtual ~Filter() {}

        QString id;
        QPixmap icon;
        KisFilterSP filter;
        virtual int childrenCount() {
            return 0;
        }
    };
    struct Category : public Node {

        virtual ~Category() {}

        QString id;
        QList<Filter> filters;
        virtual int childrenCount() {
            return filters.count();
        }
    };

    QHash<QString, Category> categories;
    QList<QString> categoriesKeys;
    KisPaintDeviceSP thumb;
    QHash<const KisFilter*, QImage> previewCache;
};

KisFiltersModel::KisFiltersModel(KisPaintDeviceSP thumb) : d(new Private)
{
    d->thumb = thumb;
    QList<KisFilterSP> filters = KisFilterRegistry::instance()->values();
    foreach(const KisFilterSP filter, filters) {
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
        d->categories[ filter->menuCategory().id()].filters.append(filt);
    }
    qSort(d->categoriesKeys);
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
        if (role == Qt::DecorationRole) {
#if 1
            Private::Node* node = static_cast<Private::Node*>(index.internalPointer());
            Private::Filter* filter = dynamic_cast<Private::Filter*>(node);
            if (filter) {
                if (!d->previewCache.contains(filter->filter)) {
//                     KisPaintDeviceSP target = new KisPaintDevice( d->thumb->colorSpace() );
                    KisPaintDeviceSP target = new KisPaintDevice(*d->thumb);
                    KisConstProcessingInformation cpi(d->thumb, QPoint(0, 0));
                    KisProcessingInformation cp(target, QPoint(0, 0));
                    filter->filter->process(cpi, cp, QSize(100, 100), filter->filter->defaultConfiguration(d->thumb));
                    d->previewCache[ filter->filter ] = target->convertToQImage(0);
                }
                return d->previewCache[ filter->filter ];
            } else {
#endif
                return QVariant();
#if 1
            }
#endif
        } else if (role == Qt::DisplayRole) {
            Private::Node* node = static_cast<Private::Node*>(index.internalPointer());
            return QVariant(node->displayRole());
        }
    }
    return QVariant();
}

Qt::ItemFlags KisFiltersModel::flags(const QModelIndex & index) const
{
    if (!index.isValid()) return 0;

    Private::Node* node = static_cast<Private::Node*>(index.internalPointer());
    Private::Filter* filter = dynamic_cast<Private::Filter*>(node);
    if (filter) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    } else {
        return Qt::ItemIsEnabled;
    }
}
