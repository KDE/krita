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

#ifndef PRESETMODEL_H
#define PRESETMODEL_H

#include <QAbstractListModel>

class PresetModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged)
    Q_PROPERTY(QString currentPreset READ currentPreset WRITE setCurrentPreset NOTIFY currentPresetChanged)
public:
    enum PresetRoles {
        ImageRole = Qt::UserRole + 1,
        TextRole,
        NameRole
    };

    explicit PresetModel(QObject *parent = 0);
    virtual ~PresetModel();

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QObject* view() const;
    void setView(QObject* newView);

    QString currentPreset() const;
    void setCurrentPreset(QString presetName);

    Q_INVOKABLE int nameToIndex(QString presetName) const;

Q_SIGNALS:
    void viewChanged();
    void currentPresetChanged();

public Q_SLOTS:
    void activatePreset(int index);
    void resourceChanged(int key, const QVariant& v);

private:
    class Private;
    Private* d;
};

#endif // PRESETMODEL_H
