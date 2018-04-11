/* This file is part of the KDE project
 * Copyright (C) 2012 Boudewijn Rempt <boud@kogmbh.com>
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
#include "RecentImagesModel.h"

#include "RecentFileManager.h"

#include <QFile>
#include <QFileInfo>
#include <QDateTime>

class RecentImagesModel::Private {
public:

    RecentFileManager *recentFileManager;
};


RecentImagesModel::RecentImagesModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private())
{
    d->recentFileManager = 0;
}

RecentImagesModel::~RecentImagesModel()
{
    delete d;
}

QHash<int, QByteArray> RecentImagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ImageRole] = "image";
    roles[TextRole] = "text";
    roles[UrlRole] = "url";
    roles[NameRole] = "name";
    roles[DateRole] = "filedate";

    return roles;
}

int RecentImagesModel::rowCount(const QModelIndex &/*parent*/) const
{
    if (d->recentFileManager)
       return d->recentFileManager->size();
    else
        return 0;
}

QVariant RecentImagesModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (!d->recentFileManager) return result;
    if (index.isValid())
    {
        Q_ASSERT(index.row() < d->recentFileManager->size());

        QString key = d->recentFileManager->recentFileName(index.row());
        QString value = d->recentFileManager->recentFile(index.row());

        switch(role)
        {
        case ImageRole:
            result = QString("image://recentimage/%1").arg(value);
            break;
        case TextRole:
            result = QFileInfo(value).completeBaseName();
            break;
        case UrlRole:
            result = value;
            break;
        case NameRole:
            result = key;
            break;
        case DateRole:
        {
            QFile f(value);
            if (f.exists()) {
                QFileInfo fi(value);
                result = fi.lastModified().toString("dd-mm-yyyy (hh:mm)");
            }
            break;
        }
        default:
            result = "";
            break;
        }
    }
    return result;
}

QVariant RecentImagesModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case UrlRole:
        case NameRole:
        case DateRole:
        default:
            result = "";
            break;
        }
    }
    return result;
}

QObject *RecentImagesModel::recentFileManager() const
{
    return d->recentFileManager;
}

void RecentImagesModel::setRecentFileManager(QObject *recentFileManager)
{
    disconnect(d->recentFileManager);
    d->recentFileManager = qobject_cast<RecentFileManager*>(recentFileManager);
    connect(d->recentFileManager, SIGNAL(recentFilesListChanged()), SLOT(recentFilesListChanged()));
    emit recentFileManagerChanged();
}

void RecentImagesModel::recentFilesListChanged()
{
    beginResetModel();
    endResetModel();
}

void RecentImagesModel::addRecent(const QString &_url)
{
    if (d->recentFileManager)
        d->recentFileManager->addRecent(_url);
}

