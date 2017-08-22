/* This file is part of the KDE project
 *
 * Copyright (c) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */



#ifndef CALLIGRAMOBILE_FILESYSTEMMODEL_H
#define CALLIGRAMOBILE_FILESYSTEMMODEL_H

#include <QQmlParserStatus>

#include <QAbstractListModel>

class FileSystemModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString parentFolder READ parentFolder)
    Q_PROPERTY(QString filter READ filter WRITE setFilter)

public:
    enum FileRoles {
        FileNameRole = Qt::UserRole,
        FilePathRole,
        FileIconRole,
        FileTypeRole,
        FileDateRole,
    };

    explicit FileSystemModel(QObject* parent = 0);
    virtual ~FileSystemModel();

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    virtual void classBegin();
    virtual void componentComplete();

    virtual QString path();
    virtual void setPath(const QString& path);

    virtual QString parentFolder();

    virtual QString filter();
    virtual void setFilter(const QString& filter);

Q_SIGNALS:
    void pathChanged();

private:
    class Private;
    Private * const d;
};

#endif // CALLIGRAMOBILE_FILESYSTEMMODEL_H
