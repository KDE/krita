/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef MUSIC_PARTSLISTMODEL_H
#define MUSIC_PARTSLISTMODEL_H

#include <QAbstractListModel>

namespace MusicCore {
    class Sheet;
    class Part;
}

class PartsListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PartsListModel(MusicCore::Sheet *sheet);

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
private slots:
    void partAdded(int index, MusicCore::Part* part);
    void partRemoved(int index, MusicCore::Part* part);
private:
    MusicCore::Sheet* m_sheet;
};

#endif // MUSIC_PARTSLISTMODEL_H
