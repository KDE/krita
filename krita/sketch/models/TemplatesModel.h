/*
 * This file is part of the KDE project
 * Copyright (C) 2014 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TEMPLATESMODEL_H
#define TEMPLATESMODEL_H

#include <QAbstractListModel>

class TemplatesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum TemplateRoles {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        FileRole,
        IconRole,
        GroupName,
        GroupFolded
    };
    explicit TemplatesModel(QObject* parent = 0);
    virtual ~TemplatesModel();
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int rowCount(const QModelIndex& parent) const;
    Q_INVOKABLE QString groupNameOf(int index) const;
    Q_INVOKABLE void toggleGroup(const QString& name);
    Q_SLOT void populate();

private:
    struct ItemData;
    class Private;
    Private* d;
};

#endif // TEMPLATESMODEL_H
