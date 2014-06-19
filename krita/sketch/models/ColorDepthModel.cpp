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

#include "ColorDepthModel.h"
#include <libs/pigment/KoColorSpaceRegistry.h>

class ColorDepthModel::Private
{
public:
    Private() { }

    QString colorModelId;
    QList<KoID> colorDepths;
};

ColorDepthModel::ColorDepthModel(QObject* parent)
    : QAbstractListModel(parent), d(new Private)
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(TextRole, "text");
    setRoleNames(roleNames);
}

ColorDepthModel::~ColorDepthModel()
{
    delete d;
}

int ColorDepthModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->colorDepths.count();
}

QVariant ColorDepthModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || index.row() < 0 || index.row() >= d->colorDepths.count())
        return QVariant();

    if(role == TextRole) {
        return d->colorDepths.at(index.row()).name();
    }

    return QVariant();
}

QString ColorDepthModel::colorModelId() const
{
    return d->colorModelId;
}

void ColorDepthModel::setColorModelId(const QString& id)
{
    if(id != d->colorModelId) {
        d->colorModelId = id;
        if(d->colorDepths.count() > 0) {
            beginRemoveRows(QModelIndex(), 0, d->colorDepths.count() - 1);
            endRemoveRows();
        }
        d->colorDepths = KoColorSpaceRegistry::instance()->colorDepthList(d->colorModelId, KoColorSpaceRegistry::OnlyUserVisible);
        if(d->colorDepths.count() > 0) {
            beginInsertRows(QModelIndex(), 0, d->colorDepths.count() - 1);
            endInsertRows();
        }
        emit colorModelIdChanged();
    }
}

QString ColorDepthModel::id(int index)
{
    if(index < 0 || index >= d->colorDepths.count())
        return QString();

    return d->colorDepths.at(index).id();
}

int ColorDepthModel::indexOf(const QString& id)
{
    return d->colorDepths.indexOf(KoID(id));
}
