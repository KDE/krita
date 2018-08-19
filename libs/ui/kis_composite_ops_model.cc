/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_composite_ops_model.h"

#include <KoCompositeOp.h>
#include <KoCompositeOpRegistry.h>
#include <kis_icon.h>

#include "kis_debug.h"
#include "kis_config.h"

KoID KisCompositeOpListModel::favoriteCategory() {
    static KoID category("favorites", ki18n("Favorites"));
    return category;
}

void KisCompositeOpListModel::initialize()
{
    QMap<KoID, KoID> ops = KoCompositeOpRegistry::instance().getCompositeOps();
    QMapIterator<KoID, KoID> it(ops);

    while (it.hasNext()) {
        KoID op = *it.next();
        KoID category = it.key();
        BaseKoIDCategorizedListModel::DataItem *item = categoriesMapper()->addEntry(category.name(), op);
        item->setCheckable(true);
    }

    BaseKoIDCategorizedListModel::DataItem *item = categoriesMapper()->addCategory(favoriteCategory().name());
    item->setExpanded(true);

    readFavoriteCompositeOpsFromConfig();
}

KisCompositeOpListModel* KisCompositeOpListModel::sharedInstance()
{
    static KisCompositeOpListModel *model = 0;

    if (!model) {
        model = new KisCompositeOpListModel();
        model->initialize();
    }

    return model;
}

void KisCompositeOpListModel::validate(const KoColorSpace *cs)
{
    for (int i = 0, size = categoriesMapper()->rowCount(); i < size; i++) {
        DataItem *item = categoriesMapper()->itemFromRow(i);

        if (!item->isCategory()) {
            bool value = KoCompositeOpRegistry::instance().colorSpaceHasCompositeOp(cs, *item->data());
            item->setEnabled(value);
        }
    }
}

bool KisCompositeOpListModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid()) return false;

    bool result = BaseKoIDCategorizedListModel::setData(idx, value, role);

    DataItem *item = categoriesMapper()->itemFromRow(idx.row());
    Q_ASSERT(item);


    if(role == Qt::CheckStateRole) {
        if (item->isChecked()) {
            addFavoriteEntry(*item->data());
        } else {
            removeFavoriteEntry(*item->data());
        }

        writeFavoriteCompositeOpsToConfig();
    }

    return result;
}

QVariant KisCompositeOpListModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return QVariant();

    if(role == Qt::DecorationRole) {
        DataItem *item = categoriesMapper()->itemFromRow(idx.row());
        Q_ASSERT(item);

        if (!item->isCategory() && !item->isEnabled()) {
            return KisIconUtils::loadIcon("dialog-warning");
        }
    }

    return BaseKoIDCategorizedListModel::data(idx, role);
}

void KisCompositeOpListModel::addFavoriteEntry(const KoID &entry)
{
    DataItem *item = categoriesMapper()->addEntry(favoriteCategory().name(), entry);
    item->setCheckable(false);
}

void KisCompositeOpListModel::removeFavoriteEntry(const KoID &entry)
{
    categoriesMapper()->removeEntry(favoriteCategory().name(), entry);
}

void KisCompositeOpListModel::readFavoriteCompositeOpsFromConfig()
{
    KisConfig config(true);
    Q_FOREACH (const QString &op, config.favoriteCompositeOps()) {
        KoID entry = KoCompositeOpRegistry::instance().getKoID(op);

        DataItem *item = categoriesMapper()->fetchOneEntry(entry);
        if (item) {
            item->setChecked(true);
        }

        addFavoriteEntry(entry);
    }
}

void KisCompositeOpListModel::writeFavoriteCompositeOpsToConfig() const
{
    QStringList list;
    QVector<DataItem*> filteredItems =
        categoriesMapper()->itemsForCategory(favoriteCategory().name());

    Q_FOREACH (DataItem *item, filteredItems) {
        list.append(item->data()->id());
    }

    KisConfig config(false);
    config.setFavoriteCompositeOps(list);
}
