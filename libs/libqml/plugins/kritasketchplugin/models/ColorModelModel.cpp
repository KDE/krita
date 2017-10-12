/*
 * This file is part of the KDE project
 * Copyright (C) 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "ColorModelModel.h"
#include <libs/pigment/KoColorSpaceRegistry.h>

class ColorModelModel::Private
{
public:
    Private() { }

    QList<KoID> colorModels;
};

ColorModelModel::ColorModelModel(QObject* parent)
    : QAbstractListModel(parent), d(new Private)
{
    d->colorModels = KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::OnlyUserVisible);

    QHash<int, QByteArray> roleNames;
    roleNames.insert(TextRole, "text");
    setRoleNames(roleNames);
}

ColorModelModel::~ColorModelModel()
{
    delete d;
}

int ColorModelModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->colorModels.count();
}

QVariant ColorModelModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || index.row() < 0 || index.row() >= d->colorModels.count())
    {
        return QVariant();
    }

    if( role == TextRole )
        return d->colorModels.at(index.row()).name();

    return QVariant();
}

QString ColorModelModel::id(int index)
{
    if(index < 0 || index >= d->colorModels.count())
        return QString();

    return d->colorModels.at(index).id();
}

int ColorModelModel::indexOf(const QString& id)
{
    return d->colorModels.indexOf(KoID(id));
}
