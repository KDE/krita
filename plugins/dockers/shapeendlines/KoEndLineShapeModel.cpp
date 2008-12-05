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

#include "KoEndLineShapeModel.h"

KoEndLineShapeModel::KoEndLineShapeModel( QObject * parent )
    : QAbstractListModel( parent )
{

}

int KoEndLineShapeModel::rowCount( const QModelIndex &/*parent*/ ) const
{
  
}

QVariant KoEndLineShapeModel::data( const QModelIndex &index, int role ) const
{
       if (!index.isValid() || index.row() > m_shapeTemplateList.count ())
        return QVariant();

    switch(role)
    {
        //case Qt::ToolTipRole:
          //  return m_shapeTemplateList[index.row()].toolTip;

        case Qt::DecorationRole:
            return m_shapeTemplateList[index.row()].icon;

        case Qt::UserRole:
            return m_shapeTemplateList[index.row()].id;

      //  case Qt::UserRole + 1:
        //    return m_shapeTemplateList[index.row()].properties;

        case Qt::DisplayRole:
            return m_shapeTemplateList[index.row()].name;

        default:
            return QVariant();
    }

    return QVariant();
}
void KoEndLineShapeModel::setShapeTemplateList(const QList<KoEndLineShape>& newlist)
{
    m_shapeTemplateList = newlist;
    reset();
}