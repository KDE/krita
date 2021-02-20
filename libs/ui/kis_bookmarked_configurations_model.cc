/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_bookmarked_configurations_model.h"
#include <QList>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoID.h>

#include <kis_bookmarked_configuration_manager.h>

struct KisBookmarkedConfigurationsModel::Private {
    KisBookmarkedConfigurationManager* bookmarkManager;
    QList<QString> configsKey;
};


KisBookmarkedConfigurationsModel::KisBookmarkedConfigurationsModel(KisBookmarkedConfigurationManager* bm) : d(new Private)
{
    d->bookmarkManager = bm;
    d->configsKey = d->bookmarkManager->configurations();
    std::sort(d->configsKey.begin(), d->configsKey.end());
}

KisBookmarkedConfigurationsModel::~KisBookmarkedConfigurationsModel()
{
    delete d;
}

KisBookmarkedConfigurationManager* KisBookmarkedConfigurationsModel::bookmarkedConfigurationManager()
{
    return d->bookmarkManager;
}

int KisBookmarkedConfigurationsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2 + d->configsKey.size();
}

QVariant KisBookmarkedConfigurationsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.row()) {
        case 0:
            return i18n("Default");
        case 1:
            return i18n("Last Used");
        default:
            return d->configsKey[ index.row() - 2 ];
        }
    }
    return QVariant();
}

bool KisBookmarkedConfigurationsModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole && index.row() >= 2) {
        QString name = value.toString();
        int idx = index.row() - 2;
        KisSerializableConfigurationSP config = d->bookmarkManager->load(d->configsKey[idx]);
        d->bookmarkManager->remove(d->configsKey[idx]);
        d->bookmarkManager->save(name, config);
        d->configsKey[idx] = name;

        emit(dataChanged(index, index));
        return true;
    }
    return false;
}

KisSerializableConfigurationSP KisBookmarkedConfigurationsModel::configuration(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;
    switch (index.row()) {
    case 0:
        dbgKrita << "loading default" << endl;
        return d->bookmarkManager->load(KisBookmarkedConfigurationManager::ConfigDefault);
        break;
    case 1:
        return d->bookmarkManager->load(KisBookmarkedConfigurationManager::ConfigLastUsed);
        break;
    default:
        return d->bookmarkManager->load(d->configsKey[ index.row() - 2 ]);
    }
}

QModelIndex KisBookmarkedConfigurationsModel::indexFor(const QString& name) const
{
    int idx = d->configsKey.indexOf(name);
    if (idx == -1) return QModelIndex();
    return createIndex(idx + 2, 0);
}

bool KisBookmarkedConfigurationsModel::isIndexDeletable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 2) return false;
    return true;
}

void KisBookmarkedConfigurationsModel::newConfiguration(KLocalizedString baseName, const KisSerializableConfigurationSP config)
{
    saveConfiguration(d->bookmarkManager->uniqueName(baseName), config);
}

void KisBookmarkedConfigurationsModel::saveConfiguration(const QString & name, const KisSerializableConfigurationSP config)
{
    d->bookmarkManager->save(name, config);
    if (!d->configsKey.contains(name)) {
        beginInsertRows(QModelIndex(), d->configsKey.count() + 2, d->configsKey.count() + 2);
        d->configsKey << name;
        endInsertRows();
    }
}

void KisBookmarkedConfigurationsModel::deleteIndex(const QModelIndex &index)
{
    if (!index.isValid() || index.row() < 2) return ;
    int idx = index.row() - 2;
    d->bookmarkManager->remove(d->configsKey[idx]);
    beginRemoveRows(QModelIndex(), idx + 2, idx + 2);
    d->configsKey.removeAt(idx);
    endRemoveRows();
}

Qt::ItemFlags KisBookmarkedConfigurationsModel::flags(const QModelIndex & index) const
{
    if (!index.isValid()) return 0;
    switch (index.row()) {
    case 0:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    case 1:
        if (d->bookmarkManager->exists(KisBookmarkedConfigurationManager::ConfigLastUsed)) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        } else {
            return 0;
        }
    default:
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }
}
