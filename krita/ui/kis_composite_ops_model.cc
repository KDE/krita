/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_composite_ops_model.h"
#include <KoCompositeOp.h>

#include <kcategorizedsortfilterproxymodel.h>
#include "kis_debug.h"

static QStringList opsInOrder;

KisCompositeOpsModel::KisCompositeOpsModel(const QList<KoCompositeOp*>& list)
{
    foreach(KoCompositeOp* op, list) {
        if (op->userVisible()) {
            m_list.push_back(CompositeOpInfo(op->id(), op->description(), op->category()));
        }
    }
    if (opsInOrder.isEmpty()) {
        opsInOrder <<
        COMPOSITE_OVER <<
        COMPOSITE_ERASE <<
        COMPOSITE_COPY <<
        COMPOSITE_ALPHA_DARKEN <<
        COMPOSITE_IN <<
        COMPOSITE_OUT <<
        COMPOSITE_ATOP <<
        COMPOSITE_XOR <<
        COMPOSITE_PLUS <<
        COMPOSITE_MINUS <<
        COMPOSITE_ADD <<
        COMPOSITE_SUBTRACT <<
        COMPOSITE_DIFF <<
        COMPOSITE_MULT <<
        COMPOSITE_DIVIDE <<
        COMPOSITE_DODGE <<
        COMPOSITE_BURN <<
        COMPOSITE_BUMPMAP <<
        COMPOSITE_CLEAR <<
        COMPOSITE_DISSOLVE <<
        COMPOSITE_DISPLACE <<
        COMPOSITE_NO <<
        COMPOSITE_DARKEN <<
        COMPOSITE_LIGHTEN <<
        COMPOSITE_HUE <<
        COMPOSITE_SATURATION <<
        COMPOSITE_VALUE <<
        COMPOSITE_COLOR <<
        COMPOSITE_COLORIZE <<
        COMPOSITE_LUMINIZE <<
        COMPOSITE_SCREEN <<
        COMPOSITE_OVERLAY <<
        COMPOSITE_UNDEF <<
        COMPOSITE_COPY_RED <<
        COMPOSITE_COPY_GREEN <<
        COMPOSITE_COPY_BLUE <<
        COMPOSITE_COPY_OPACITY;
    }
}

KisCompositeOpsModel::~KisCompositeOpsModel()
{
}

int KisCompositeOpsModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_list.count();
}

QVariant KisCompositeOpsModel::data(const QModelIndex & index, int role) const
{
    if (index.isValid()) {
        switch (role) {
        case Qt::DisplayRole: {
            return m_list[index.row()].description;
        }
        case CompositeOpSortRole: {
            int idx = opsInOrder.indexOf(m_list[index.row()].id);
            if (idx == -1) return opsInOrder.count();
            return idx;
        }
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
        case KCategorizedSortFilterProxyModel::CategorySortRole:
            return m_list[index.row()].category;
        }
    }
    return QVariant();
}

const QString& KisCompositeOpsModel::itemAt(const QModelIndex & index) const
{
    if (!index.isValid()) return COMPOSITE_OVER;
    return m_list[index.row()].id;
}

QModelIndex KisCompositeOpsModel::indexOf(const KoCompositeOp* op) const
{
    if (!op) return QModelIndex();

    return indexOf(op->id());
}

QModelIndex KisCompositeOpsModel::indexOf(const QString& id) const
{
    int index = 0;
    foreach(const CompositeOpInfo&  op2, m_list) {
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
