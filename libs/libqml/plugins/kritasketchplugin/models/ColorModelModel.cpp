/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
}

ColorModelModel::~ColorModelModel()
{
    delete d;
}

QHash<int, QByteArray> ColorModelModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(TextRole, "text");

    return roleNames;
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
