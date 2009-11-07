/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef MYBRUSH_RESOURCES_LISTMODEL_H
#define MYBRUSH_RESOURCES_LISTMODEL_H

#include <QAbstractListModel>

class QListView;
class MyPaintFactory;
class MyPaintBrushResource;

class MyBrushResourcesListModel : public QAbstractListModel {

    Q_OBJECT

public:

    MyBrushResourcesListModel(QListView* parent);
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::UserRole);
    Qt::ItemFlags flags(const QModelIndex& index) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    MyPaintBrushResource* brush(const QString& baseFileName) const;

private:

    MyPaintFactory* m_factory;
};
#endif // MYBRUSH_RESOURCES_LISTMODEL_H
