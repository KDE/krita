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

#include "ColorProfileModel.h"
#include <libs/pigment/KoColorSpaceRegistry.h>
#include <libs/pigment/KoColorProfile.h>

class ColorProfileModel::Private
{
public:
    Private(ColorProfileModel* qq) : q(qq), defaultProfile(-1) { }

    void updateProfiles();

    ColorProfileModel* q;

    QString colorModelId;
    QString colorDepthId;
    QString colorSpaceId;
    int defaultProfile;
    QList<const KoColorProfile*> colorProfiles;
};

ColorProfileModel::ColorProfileModel(QObject* parent)
    : QAbstractListModel(parent), d(new Private(this))
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(TextRole, "text");
    setRoleNames(roleNames);
}

ColorProfileModel::~ColorProfileModel()
{
    delete d;
}

int ColorProfileModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->colorProfiles.count();
}

QVariant ColorProfileModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || index.row() < 0 || index.row() >= d->colorProfiles.count())
        return QVariant();

    if(role == TextRole) {
        return d->colorProfiles.at(index.row())->name();
    }

    return QVariant();
}

QString ColorProfileModel::colorModelId() const
{
    return d->colorModelId;
}

void ColorProfileModel::setColorModelId(const QString& id)
{
    if(id != d->colorModelId) {
        d->colorModelId = id;
        d->updateProfiles();
        emit colorModelIdChanged();
    }
}

QString ColorProfileModel::colorDepthId() const
{
    return d->colorDepthId;
}

void ColorProfileModel::setColorDepthId(const QString& id)
{
    if(id != d->colorDepthId) {
        d->colorDepthId = id;
        d->updateProfiles();
        emit colorDepthIdChanged();
    }
}

int ColorProfileModel::defaultProfile() const
{
    return d->defaultProfile;
}

QString ColorProfileModel::id(int index)
{
    return d->colorProfiles.at(index)->name();
}

void ColorProfileModel::Private::updateProfiles()
{
    if(colorDepthId.isEmpty() || colorModelId.isEmpty())
        return;

    q->beginResetModel();

    colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(colorModelId, colorDepthId);
    colorProfiles = KoColorSpaceRegistry::instance()->profilesFor(colorSpaceId);

    QString profile = KoColorSpaceRegistry::instance()->colorSpaceFactory(colorSpaceId)->defaultProfile();
    for(int i = 0; i < colorProfiles.count(); ++i) {
        if(colorProfiles.at(i)->name() == profile) {
            defaultProfile = i;
            break;
        }
    }

    q->endResetModel();

    emit q->defaultProfileChanged();
}
