/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
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

#include "KisSnapshotModel.h"

#include <QMap>
#include <QList>

#include <KisDocument.h>

struct KisSnapshotModel::Private
{
    Private();

    QMap<KisCanvas2 *, QList<KisDocument *> > canvasDocMap;
    QPointer<KisCanvas2> curCanvas;
};

KisSnapshotModel::Private::Private()
{
}

KisSnapshotModel::KisSnapshotModel()
    : QAbstractListModel()
    , m_d(new Private)
{
}

KisSnapshotModel::~KisSnapshotModel()
{
}

int KisSnapshotModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant KisSnapshotModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

void KisSnapshotModel::setCanvas(QPointer<KisCanvas2> canvas)
{
    m_d->curCanvas = canvas;
}

