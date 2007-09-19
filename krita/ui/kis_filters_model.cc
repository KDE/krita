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

#include <kis_filter.h>
#include <kis_filter_registry.h>


struct KisFiltersModel::Private {
    struct Node {
        QString name;
        QString displayRole() { return name; }
        virtual int childrenCount() = 0;
    };
    struct Filter : public Node {
        QString id;
        QPixmap icon;
        KisFilterSP filter;
        virtual int childrenCount() { return 0; }
    };
    struct Category : public Node {
        QString id;
        QList<Filter> filters;
        virtual int childrenCount() { return filters.count(); }
    };
    
    QHash<QString, Category> categories;
    QList<QString> categoriesKeys;
};

KisFiltersModel::KisFiltersModel() : d(new Private)
{
    KisFilterRegistry* registry = KisFilterRegistry::instance();
    foreach(QString key, registry->keys())
    {
        KisFilterSP filter = registry->get(key);
        Q_ASSERT(filter);
        if(not d->categories.contains( filter->menuCategory().id() ) )
        {
            Private::Category cat;
            cat.id = filter->menuCategory().id();
            cat.name = filter->menuCategory().name();
            d->categories[ cat.id ] = cat;
            d->categoriesKeys.append( cat.id );
        }
        Private::Filter filt;
        filt.id = filter->id();
        filt.name = filter->name();
        filt.filter = filter;
        d->categories[ filter->menuCategory().id() ].filters.append(filt);
    }
    qSort(d->categoriesKeys);
}

int KisFiltersModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        Private::Node* node = static_cast<Private::Node*>(parent.internalPointer());
        return node->childrenCount();
    } else {
        return d->categoriesKeys.count();
    }
}
int KisFiltersModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex KisFiltersModel::index(int row, int column, const QModelIndex &parent) const
{
//     kDebug() << parent.isValid() << row << endl;
    if(parent.isValid())
    {
        Private::Category* category = static_cast<Private::Category*>(parent.internalPointer());
        return createIndex(row, column, &category->filters[row]);
    } else
    {
        return createIndex(row, column, &d->categories[ d->categoriesKeys[row] ]);
    }
}

QModelIndex KisFiltersModel::parent(const QModelIndex &child) const
{
    if(not child.isValid())
        return QModelIndex();
    Private::Node* node = static_cast<Private::Node*>(child.internalPointer());
    Private::Filter* filter = dynamic_cast<Private::Filter*>(node);
    if(filter)
    {
        QString catId = filter->filter->menuCategory().id();
        return createIndex( d->categoriesKeys.indexOf(catId) , 0, &d->categories[ catId ]);
    }
    return QModelIndex(); // categories don't have parents
}

QVariant KisFiltersModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid())
    {
        if(role == Qt::DisplayRole)
        {
            Private::Node* node = static_cast<Private::Node*>(index.internalPointer());
            return QVariant(node->displayRole());
        }
    }
    return QVariant();
}
