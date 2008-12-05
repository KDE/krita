/* This file is part of the KDE project
 * Copyright (C) 2008 Alexia Allanic
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

#ifndef KOENDLINESHAPEMODEL_H_
#define KOENDLINESHAPEMODEL_H_

#include <QAbstractItemModel>
#include <QList>
#include <QString>
#include <QIcon>

/**
 * Struct containing the information stored in KoEndLineShape item
 */
struct KoEndLineShape
{
    KoEndLineShape()
    {
    };

    QString id;
    QString name;
    QIcon icon;
};

class KoEndLineShapeModel : public QAbstractListModel
{
public:
    KoEndLineShapeModel( QObject * parent = 0 );
    ~KoEndLineShapeModel() {}
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void setShapeTemplateList(const QList<KoEndLineShape>& newlist);
private:
    QList<KoEndLineShape> m_shapeTemplateList;
}; 

#endif