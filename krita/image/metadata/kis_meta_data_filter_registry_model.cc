/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_filter_registry_model.h"

using namespace KisMetaData;

struct FilterRegistryModel::Private {
    
};

FilterRegistryModel::FilterRegistryModel()
    : KoGenericRegistryModel<Filter*>( FilterRegistry::instance() ), d(new Private)
{
    
}

FilterRegistryModel::~FilterRegistryModel()
{
    delete d;
}

QVariant FilterRegistryModel::data(const QModelIndex &index, int role ) const
{
    if(index.isValid())
    {
        if( role == Qt::CheckStateRole)
        {
            return get(index)->defaultEnabled();
        }
    }
    return KoGenericRegistryModel<Filter*>::data(index, role);
}

Qt::ItemFlags FilterRegistryModel::flags( const QModelIndex & ) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}
