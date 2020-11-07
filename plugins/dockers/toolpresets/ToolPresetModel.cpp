/*
 *  Copyright (c) 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "ToolPresetModel.h"

#include <QDebug>
#include <QApplication>
#include <QStyle>
#include <QStyleOptionButton>

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

ToolPresetModel::ToolPresetModel(QObject *parent)
    : BaseOptionCategorizedListModel(parent)
{
    // Categories for each tool
    Q_FOREACH(const KoToolFactoryBase *toolFactory, KoToolRegistry::instance()->values()) {
        ToolPresetInfo info(toolFactory->id(), toolFactory->toolTip());

        // All the tool presets for this id
        KConfig cfg(createConfigFileName(info.toolId), KConfig::SimpleConfig);
        Q_FOREACH(const QString &group, cfg.groupList()) {
            auto item = categoriesMapper()->addEntry(info.toolId, ToolPresetInfo(info.toolId, group));
            item->setCheckable(true);
        }
    }

    // The favorites category
    auto item = categoriesMapper()->addCategory(favoriteCategory().presetName);
    item->setExpanded(true);

    // The favorites themselves
    KConfigGroup grp(KSharedConfig::openConfig(), "");
    QStringList favorites = grp.readEntry("favorite_tool_presets", QString()).split(',');
    Q_FOREACH(const QString &favorite, favorites) {
        if (!favorite.contains(":")) {
            continue;
        }
        QString toolId = favorite.split(':').first();
        QString toolName = favorite.split(':').last();

        ToolPresetInfo entry(toolId, toolName);

        auto item = categoriesMapper()->fetchOneEntry(entry);
        if (item) {
            item->setChecked(true);
        }
        addFavoriteEntry(entry);
    }
}

QVariant ToolPresetModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return QVariant();

    if (role == Qt::DecorationRole) {
        DataItem *item = categoriesMapper()->itemFromRow(idx.row());
        Q_ASSERT(item);

        if (!item->isCategory()) {
            QStyle *style = QApplication::style();
            QStyleOptionButton so;
            QSize size = style->sizeFromContents(QStyle::CT_CheckBox, &so, QSize(), 0);
            ToolPresetInfo *toolInfo = item->data();
            return toolIcon(item->data()->toolId).pixmap(size);
        }
    }

    return BaseOptionCategorizedListModel::data(idx, role);
}

bool ToolPresetModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid()) return false;

    bool result = BaseOptionCategorizedListModel::setData(idx, value, role);

    DataItem *item = categoriesMapper()->itemFromRow(idx.row());

    if (role == Qt::CheckStateRole) {
        if (item->isChecked()) {
            addFavoriteEntry(*item->data());
        } else {
            removeFavoriteEntry(*item->data());
        }

        // Save all favorites
        QStringList favoritesList;
        QVector<DataItem*> filteredItems =
                categoriesMapper()->itemsForCategory(favoriteCategory().presetName);

        Q_FOREACH (DataItem *entry, filteredItems) {
            ToolPresetInfo *info = entry->data();
            favoritesList.append(info->toolId + ':' + info->presetName);
        }

        KConfigGroup grp(KSharedConfig::openConfig(), "");
        grp.writeEntry("favorite_tool_presets", favoritesList.join(','));

    }

    return result;
}

ToolPresetInfo *ToolPresetModel::toolPresetInfo(int row) const
{
    DataItem *item = categoriesMapper()->itemFromRow(row);
    Q_ASSERT(item);
    return item->data();
}

ToolPresetInfo ToolPresetModel:: favoriteCategory()
{
    static ToolPresetInfo category("favorites", ki18n("Favorites").toString());
    return category;
}

void ToolPresetModel::addFavoriteEntry(const ToolPresetInfo &entry)
{
    DataItem *item  = categoriesMapper()->addEntry(favoriteCategory().presetName, entry);
    item->setCheckable(false);
}

void ToolPresetModel::removeFavoriteEntry(const ToolPresetInfo &entry)
{
    categoriesMapper()->removeEntry(favoriteCategory().toolId, entry);
}

bool operator==(const ToolPresetInfo &a, const ToolPresetInfo &b)
{
    return ((a.toolId == b.toolId) && (a.presetName == b.presetName));
}

ToolPresetInfo *ToolPresetFilterProxyModel::toolPresetInfo(int row) const
{
    QModelIndex idx = index(row, 0);
    QModelIndex sourceIdx = mapToSource(idx);

    ToolPresetModel *presetModel = dynamic_cast<ToolPresetModel*>(sourceModel());

    if (!presetModel) return 0;

    return presetModel->toolPresetInfo(sourceIdx.row());
}

void ToolPresetFilterProxyModel::setFilter(const QString &toolId)
{
    m_toolIdFilter = toolId;
    invalidateFilter();
}

bool ToolPresetFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_toolIdFilter.isEmpty()) return true;

    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) return false;

    ToolPresetModel *presetModel = dynamic_cast<ToolPresetModel*>(sourceModel());
    if (!presetModel) return false;

    ToolPresetInfo *info = presetModel->toolPresetInfo(idx.row());

    return (info->toolId == m_toolIdFilter);
}



