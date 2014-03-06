/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoResourceTableModel.h"
#include <QPixmap>
#include <QColor>
#include "KoResource.h"
#include "KoResourceServerAdapter.h"
#include <klocale.h>


KoResourceTableModel::KoResourceTableModel(KoAbstractResourceServerAdapter *resourceAdapter,QObject *parent)
    :KoResourceModel(resourceAdapter,parent)
{

}

int KoResourceTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return resourcesCount();
}

int KoResourceTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant KoResourceTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DecorationRole && index.column() == 1) {
        KoResource * resource = static_cast<KoResource*>(index.internalPointer());
        if( ! resource )
            return QVariant();

        return QVariant( resource->image().scaledToWidth(50));

        /*QPixmap pixmap(20,20);
        QColor black(0,0,0);
        pixmap.fill(black);
        return pixmap;*/
    }
    else if (role == Qt::DisplayRole)
    {
        KoResource * resource = static_cast<KoResource*>(index.internalPointer());
        if (!resource) {
            return QVariant();
        }
        else {
            switch (index.column()) {
            case 1:
                return i18n( resource->name().toUtf8().data());
            case 2:
                if (assignedTagsList(resource).count()) {
                    QString taglist = assignedTagsList(resource).join("] , [");
                    return QString(" - %1: [%2]").arg(i18n("Tags"), taglist);
                }
                else {
                    return QVariant();
                }
            default:
                return QVariant();
            }
        }
    }
    else if (role == Qt::CheckStateRole && index.column()==0) {
        return Qt::Unchecked;
    }

    return QVariant();
}

QVariant KoResourceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString();
            case 1:
                return QString("Name");
            case 2:
                return QString("Tags");
            }
        }
    }
    return QVariant();
}

QModelIndex KoResourceTableModel::index ( int row, int column, const QModelIndex & ) const
{
    const QList<KoResource*> resources = currentlyVisibleResources();
    if ( row >= resources.count() || row < 0) {
        return QModelIndex();
    }
    else {
        return createIndex( row, column, resources[row] );
    }

}




