/*
 * Copyright (c) 2014 Lukáš Tvrdý <lukast.dev@gmail.com
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


#include <kis_gmic_filter_proxy_model.h>
#include "kis_gmic_filter_model.h"
#include <kis_debug.h>
#include <QQueue>

KisGmicFilterProxyModel::KisGmicFilterProxyModel(QObject* parent): QSortFilterProxyModel(parent)
{

}

bool KisGmicFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    QQueue<QModelIndex> q;
    q.enqueue(index);

    while (!q.isEmpty())
    {
        QModelIndex item = q.dequeue();
        if ( item.data(Qt::DisplayRole).toString().contains( filterRegExp() ) )
        {
            return true;
        }
        else
        {
            int rowCount = sourceModel()->rowCount(item);
            for (int row = 0; row < rowCount; row++)
            {
                q.enqueue( item.child(row, 0) );
            }
        }
    }

    return false;
}
