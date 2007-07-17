/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_bookmarked_configurations_model.h"

#include <QList>

#include <KoID.h>

#include <kis_bookmarked_configuration_manager.h>

struct KisBookmarkedConfigurationsModel::Private
{
    KisBookmarkedConfigurationManager* bookmarkManager;
    QList<QString> configsKey;
};


KisBookmarkedConfigurationsModel::KisBookmarkedConfigurationsModel(KisBookmarkedConfigurationManager* bm) : d(new Private)
{
    d->bookmarkManager = bm;
    d->configsKey = d->bookmarkManager->configurations();
    qSort( d->configsKey );
}

KisBookmarkedConfigurationsModel::~KisBookmarkedConfigurationsModel()
{
    delete d;
}

int KisBookmarkedConfigurationsModel::rowCount(const QModelIndex &parent ) const
{
    return 2 + d->configsKey.size();
}

QVariant KisBookmarkedConfigurationsModel::data(const QModelIndex &index, int role) const
{
    if(not index.isValid())
    {
        return QVariant();
    }
    if(role == Qt::DisplayRole)
    {
        switch(index.row())
        {
            case 0:
                return KisBookmarkedConfigurationManager::ConfigDefault.name();
            case 1:
                return KisBookmarkedConfigurationManager::ConfigLastUsed.name();
            default:
                return d->configsKey[ index.row() - 2 ];
        }
    }
    return QVariant();
}

KisSerializableConfiguration* KisBookmarkedConfigurationsModel::configuration(const QModelIndex &index) const
{
    return 0;
}
