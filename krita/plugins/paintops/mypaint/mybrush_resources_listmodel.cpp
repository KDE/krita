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
#include "mybrush_resources_listmodel.h"

#include <QtGui>

#include <kis_paintop_registry.h>
#include "mypaint_paintop_factory.h"
#include "mypaint_brush_resource.h"

MyBrushResourcesListModel::MyBrushResourcesListModel(QListView* parent)
    : QAbstractListModel(parent)
{
    m_factory = dynamic_cast<MyPaintFactory*>(KisPaintOpRegistry::instance()->get("mypaintbrush"));
}

QVariant MyBrushResourcesListModel::data( const QModelIndex & index, int role) const
{
    if (role == Qt::DisplayRole) {
        QFileInfo info(m_factory->brushes().at(index.row())->filename());
        return info.baseName();
    } else if (role == Qt::DecorationRole) {
        return QPixmap::fromImage(m_factory->brushes().at(index.row())->img());
    } else if (role == Qt::UserRole) {
        return m_factory->brushes().at(index.row())->filename();
    } else if (role == Qt::SizeHintRole) {
        return QSize(130, 130);
    } else if (role == Qt::ToolTipRole) {
        QFileInfo info(m_factory->brushes().at(index.row())->filename());
        return QString("Brush %1: %2").arg(index.row()).arg(info.baseName());
    } else {
        return QVariant();
    }
}

bool MyBrushResourcesListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::UserRole) {
        emit dataChanged(index, index);
        return true;
    }
    return false;

}

int MyBrushResourcesListModel::rowCount(const QModelIndex& parent) const
{
    return m_factory->brushes().size();
}

Qt::ItemFlags MyBrushResourcesListModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    } else {
        return defaultFlags;
    }
}

MyPaintBrushResource* MyBrushResourcesListModel::brush(const QString& baseFileName) const
{
    return m_factory->brush(baseFileName);
}
