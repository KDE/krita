/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef KISINPUTPROFILEMODEL_H
#define KISINPUTPROFILEMODEL_H

#include <QStringListModel>

/**
 * \brief A model providing a list of profiles available.
 */
class KisInputProfileModel : public QStringListModel
{
    Q_OBJECT
public:
    KisInputProfileModel(QObject *parent = 0);
    ~KisInputProfileModel() override;

    bool setData(const QModelIndex &index, const QVariant &value, int = Qt::EditRole) override;

    QString profileName(const QModelIndex &index);
    QModelIndex find(const QString &name);

private Q_SLOTS:
    void profileNamesChanged();
};

#endif // KISINPUTPROFILEMODEL_H
