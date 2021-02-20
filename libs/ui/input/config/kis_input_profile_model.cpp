/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_profile_model.h"

#include "input/kis_input_profile_manager.h"

KisInputProfileModel::KisInputProfileModel(QObject *parent)
    : QStringListModel(parent)
{
    setStringList(KisInputProfileManager::instance()->profileNames());
    connect(KisInputProfileManager::instance(), SIGNAL(profilesChanged()), SLOT(profileNamesChanged()));
}

KisInputProfileModel::~KisInputProfileModel()
{

}

void KisInputProfileModel::profileNamesChanged()
{
    setStringList(KisInputProfileManager::instance()->profileNames());
}

bool KisInputProfileModel::setData(const QModelIndex &index, const QVariant &value, int /*role*/)
{
    QString oldName = profileName(index);
    return KisInputProfileManager::instance()->renameProfile(oldName, value.toString());
}

QString KisInputProfileModel::profileName(const QModelIndex &index)
{
    return data(index, Qt::DisplayRole).toString();
}

QModelIndex KisInputProfileModel::find(const QString &name)
{
    for (int i = 0; i < rowCount(); ++i) {
        QModelIndex ind = index(i, 0);

        if (profileName(ind) == name) {
            return ind;
        }
    }

    return QModelIndex();
}

