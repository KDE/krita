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

#ifndef COLORDEPTHMODEL_H
#define COLORDEPTHMODEL_H

#include <QAbstractListModel>

class ColorDepthModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString colorModelId READ colorModelId WRITE setColorModelId NOTIFY colorModelIdChanged)

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
    };

    explicit ColorDepthModel(QObject* parent = 0);
    virtual ~ColorDepthModel();

    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int rowCount(const QModelIndex& parent) const;

    QString colorModelId() const;

    Q_INVOKABLE QString id(int index);
    Q_INVOKABLE int indexOf(const QString& id);

public Q_SLOTS:
    void setColorModelId(const QString& id);

Q_SIGNALS:
    void colorModelIdChanged();

private:
    class Private;
    Private * const d;
};

#endif // COLORDEPTHMODEL_H
