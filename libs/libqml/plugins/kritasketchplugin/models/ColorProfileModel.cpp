/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
}

ColorProfileModel::~ColorProfileModel()
{
    delete d;
}

QHash<int, QByteArray> ColorProfileModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(TextRole, "text");

    return roleNames;
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

    QString profile = KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId);
    for(int i = 0; i < colorProfiles.count(); ++i) {
        if(colorProfiles.at(i)->name() == profile) {
            defaultProfile = i;
            break;
        }
    }

    q->endResetModel();

    emit q->defaultProfileChanged();
}
