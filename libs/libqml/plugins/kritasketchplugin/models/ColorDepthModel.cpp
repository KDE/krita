/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
}

ColorDepthModel::~ColorDepthModel()
{
    delete d;
}

QHash<int, QByteArray> ColorDepthModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(TextRole, "text");
    return roleNames;
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
