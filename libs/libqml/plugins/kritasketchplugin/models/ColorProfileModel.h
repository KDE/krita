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

#ifndef COLORPROFILEMODEL_H
#define COLORPROFILEMODEL_H

#include <QAbstractListModel>

class ColorProfileModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString colorModelId READ colorModelId WRITE setColorModelId NOTIFY colorModelIdChanged)
    Q_PROPERTY(QString colorDepthId READ colorDepthId WRITE setColorDepthId NOTIFY colorDepthIdChanged)
    Q_PROPERTY(int defaultProfile READ defaultProfile NOTIFY defaultProfileChanged)

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
    };
    
    explicit ColorProfileModel(QObject* parent = 0);
    virtual ~ColorProfileModel();

    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int rowCount(const QModelIndex& parent) const;

    QString colorModelId() const;
    QString colorDepthId() const;

    int defaultProfile() const;

    Q_INVOKABLE QString id(int index);

public Q_SLOTS:
    void setColorModelId(const QString& id);
    void setColorDepthId(const QString& id);

Q_SIGNALS:
    void colorModelIdChanged();
    void colorDepthIdChanged();
    void defaultProfileChanged();

private:
    class Private;
    Private * const d;
};

#endif // COLORDEPTHMODEL_H
