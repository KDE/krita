/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_paint_ops_model.h"
#include <kis_paintop_factory.h>

#include <kcategorizedsortfilterproxymodel.h>
#include "kis_debug.h"
#include <kis_paintop_registry.h>
#include "kis_factory2.h"
#include <kstandarddirs.h>

static const QString DEFAULT_PAINTOP = "paintbrush";

KisPaintOpsModel::KisPaintOpsModel(const QList<KisPaintOpFactory*>& list)
{
    QString fileName;
    foreach(KisPaintOpFactory* op, list) {
        fileName = KisFactory2::componentData().dirs()->findResource("kis_images", op->pixmap());
        QPixmap pixmap(fileName);
        if (pixmap.isNull()){
            pixmap = QPixmap(22,22);
            pixmap.fill();
        }
        m_list.push_back(PaintOpInfo(op->id(), op->name(), op->category(), pixmap));
    }

    foreach(const PaintOpInfo & info, m_list){
            m_opsInOrder << info.id;
    }
    qSort(m_opsInOrder);
}

KisPaintOpsModel::~KisPaintOpsModel()
{
}

int KisPaintOpsModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_list.count();
}

QVariant KisPaintOpsModel::data(const QModelIndex & index, int role) const
{
    if (index.isValid()) {
        switch (role) {
        case Qt::DisplayRole: {
            return m_list[index.row()].name;
        }
        case Qt::DecorationRole:{
            return m_list[index.row()].icon;
        }

        case PaintOpSortRole: {
            int idx = m_opsInOrder.indexOf(m_list[index.row()].id);
            if (idx == -1) return m_opsInOrder.count();
            return idx;
        }
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
        case KCategorizedSortFilterProxyModel::CategorySortRole:
            return m_list[index.row()].category;
        }
    }
    return QVariant();
}

const QString& KisPaintOpsModel::itemAt(const QModelIndex & index) const
{
    if (!index.isValid()) return DEFAULT_PAINTOP;
    return m_list[index.row()].id;
}

QModelIndex KisPaintOpsModel::indexOf(const KisPaintOpFactory* op) const
{
    if (!op) return QModelIndex();

    return indexOf(op->id());
}

QModelIndex KisPaintOpsModel::indexOf(const QString& id) const
{
    int index = 0;
    foreach(const PaintOpInfo&  op2, m_list) {
        if (id == op2.id)
            break;
        ++index;
    }
    
    if (index < m_list.count()) {
        return createIndex(index, 0);
    } else {
        return QModelIndex();
    }

}
