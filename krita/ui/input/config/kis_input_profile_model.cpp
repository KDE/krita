/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

