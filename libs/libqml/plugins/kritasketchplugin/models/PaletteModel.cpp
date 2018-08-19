/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "PaletteModel.h"

#include <resources/KoColorSet.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceServerProvider.h>
#include <KisResourceServerProvider.h>

class PaletteModel::Private {
public:
    Private(QObject* q)
        : currentSet(0)
    {
        KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
        serverAdaptor = new KoResourceServerAdapter<KoColorSet>(rServer, q);
        serverAdaptor->connectToResourceServer();
    }
    KoResourceServerAdapter<KoColorSet>* serverAdaptor;
    KoColorSet* currentSet;
};

PaletteModel::PaletteModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
}

PaletteModel::~PaletteModel()
{
    delete d;
}

QHash<int, QByteArray> PaletteModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ImageRole] = "image";
    roles[TextRole] = "text";

    return roles;
}

int PaletteModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return d->serverAdaptor->resources().count();
}

QVariant PaletteModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (index.isValid())
    {
        switch(role)
        {
        case ImageRole:
            result = "../images/help-about.png";
            break;
        case TextRole:
            result = d->serverAdaptor->resources().at(index.row())->name();
            break;
        default:
            break;
        }
    }
    return result;
}

QVariant PaletteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    QVariant result;
    if (section == 0)
    {
        switch(role)
        {
        case ImageRole:
            result = QString("Thumbnail");
            break;
        case TextRole:
            result = QString("Name");
            break;
        default:
            break;
        }
    }
    return result;
}

void PaletteModel::itemActivated(int index)
{
    QList<KoResource*> resources = d->serverAdaptor->resources();
    if (index >= 0 && index < resources.count())
    {
        d->currentSet = dynamic_cast<KoColorSet*>(resources.at(index));
        emit colorSetChanged();
    }
}

QObject* PaletteModel::colorSet() const
{
    return d->currentSet;
}

