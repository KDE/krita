/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
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

#include "SearchModel.h"

#include <QDebug>
#include <QPushButton>
#include <QFrame>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QAction>
#include <QVariant>

#include <kis_global.h>
#include <kactioncollection.h>

SearchModel::SearchModel(KActionCollection *actionCollection, QObject *parent)
    : QAbstractListModel(parent)
    , m_actionCollection(actionCollection)
{
}

// reimp from QAbstractListModel
int SearchModel::rowCount(const QModelIndex &/*parent*/) const
{
    qDebug() << "Count" << m_actionCollection->count();
    return m_actionCollection->count();
}

int SearchModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant SearchModel::data(const QModelIndex &index, int role) const
{
    qDebug() << index.isValid() << index;

    QAction *action = m_actionCollection->action(index.row());

    qDebug() << action;

//    switch (role) {
//    case Qt::DisplayRole:
        return action->text();
//    case Qt::WhatsThisRole:
//        return action->whatsThis();
//    case Qt::UserRole + 1:
//        return QStringList() << action->text() << action->property("tags").toStringList();
//    default:
//        return action->objectName();
//    };
}
