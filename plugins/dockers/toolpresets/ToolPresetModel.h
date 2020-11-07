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
#ifndef TOOLPRESETMODEL_H
#define TOOLPRESETMODEL_H

#include "kis_categorized_list_model.h"

#include <QStandardPaths>

#include <KoIcon.h>

#include <KoToolFactoryBase.h>
#include <KoToolRegistry.h>

static QIcon toolIcon(const QString &toolId)
{
    KoToolFactoryBase *factory = KoToolRegistry::instance()->value(toolId);
    if (factory) {
        return koIcon(factory->iconName().toLatin1());
    }
    else {
        return QIcon();
    }
}

static QString createConfigFileName(QString toolId)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/toolpresets/" + toolId.replace('/', '_') + ".toolpreset";
}


struct ToolPresetInfo {
    ToolPresetInfo() = default;

    ToolPresetInfo(const QString &_toolId, const QString &_presetName)
        : toolId(_toolId)
        , presetName(_presetName)
    {}

   ToolPresetInfo(const ToolPresetInfo &) = default;

    QString toolId;
    QString presetName;
};

bool operator==(const ToolPresetInfo& a, const ToolPresetInfo& b);

struct ToolPresetInfoToQStringConverter {
    QString operator() (const ToolPresetInfo &info) {
        return info.presetName;
    }
};

typedef KisCategorizedListModel<ToolPresetInfo, ToolPresetInfoToQStringConverter> BaseOptionCategorizedListModel;

class ToolPresetModel : public BaseOptionCategorizedListModel
{
public:
    ToolPresetModel(QObject *parent = 0);

    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& idx, const QVariant& value, int role=Qt::EditRole) override;

    ToolPresetInfo *toolPresetInfo(int row) const;

private:

    friend class ToolPresetFilterProxyModel;

    static ToolPresetInfo favoriteCategory();

    void addFavoriteEntry(const ToolPresetInfo &entry);
    void removeFavoriteEntry(const ToolPresetInfo &entry);
};

class ToolPresetFilterProxyModel : public KisSortedCategorizedListModel<ToolPresetModel>
{
public:
    ToolPresetFilterProxyModel(QObject *parent = 0)
        : KisSortedCategorizedListModel<ToolPresetModel>(parent)
    {
        m_model = new ToolPresetModel(0);
        initializeModel(m_model);
    }

    ~ToolPresetFilterProxyModel()
    {
        delete m_model;
    }

protected:

    void setFilter(const QString &toolId);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
        return lessThanPriority(left, right, ToolPresetModel::favoriteCategory().presetName);
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:

    ToolPresetModel *m_model;
    QString m_toolIdFilter;

};


#endif // TOOLPRESETMODEL_H
