/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include "kis_paint_ops_model.h"

#include "kis_debug.h"
#include <brushengine/kis_paintop_registry.h>
#include <brushengine/kis_paintop_factory.h>
#include <KoResourcePaths.h>
#include <QIcon>



KisPaintOpListModel::KisPaintOpListModel(QObject *parent)
    : BasePaintOpCategorizedListModel(parent)
{
}

QVariant KisPaintOpListModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return QVariant();

    DataItem *item = categoriesMapper()->itemFromRow(idx.row());
    Q_ASSERT(item);

    if(role == Qt::DecorationRole) {
        if (!item->isCategory()) {
            return item->data()->icon;
        }
    } else if (role == SortRole) {
        return item->isCategory() ? item->name() :
            QString("%1%2%3")
            .arg(item->parentCategory()->name())
            .arg(item->data()->priority, 4)
            .arg(item->name());
    }

    return BasePaintOpCategorizedListModel::data(idx, role);
}

void KisPaintOpListModel::fill(const QList<KisPaintOpFactory*>& list)
{
    Q_FOREACH (KisPaintOpFactory *factory, list) {

        categoriesMapper()->addEntry(factory->category(),
                                     KisPaintOpInfo(factory->id(),
                                                    factory->name(),
                                                    factory->category(),
                                                    factory->icon(),
                                                    factory->priority()));
    }
    categoriesMapper()->expandAllCategories();
}
