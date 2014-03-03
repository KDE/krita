/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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

#include "KoResourceTableModel.h"
#include <QPixmap>
#include <QColor>

MyTableModel::MyTableModel(QObject *parent)
     :QAbstractTableModel(parent)
 {

 }

 int MyTableModel::rowCount(const QModelIndex & /*parent*/) const
 {
    return 2;
 }

 int MyTableModel::columnCount(const QModelIndex & /*parent*/) const
 {
     return 3;
 }

 QVariant MyTableModel::data(const QModelIndex &index, int role) const
 {
     if (role == Qt::DecorationRole && index.column() == 0) {
         QPixmap pixmap(20,20);
         QColor black(0,0,0);
         pixmap.fill(black);
         return pixmap;
     }

     if (role == Qt::DisplayRole)
     {
         switch(index.column())
         {
         case 0:
             return QString("Resource Example %1").arg(index.row()+1);
         case 1:
             return QString("Boudewijn Rempt");
         case 2:
             return QString("None");
         }
     }
     return QVariant();
 }

 QVariant MyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
      if (role == Qt::DisplayRole)
      {
          if (orientation == Qt::Horizontal) {
              switch (section)
              {
              case 0:
                  return QString("Name");
              case 1:
                  return QString("Author");
              case 2:
                  return QString("Tags");
              }
          }
      }
      return QVariant();
  }

