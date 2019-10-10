/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "compositionmodel.h"

#include <kis_icon.h>

#include <QAction>
#include <klocalizedstring.h>

CompositionModel::CompositionModel(QObject* parent): QAbstractTableModel(parent)
{
}

CompositionModel::~CompositionModel()
{
}

QVariant CompositionModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid()) {

        switch (role) {
            case Qt::DisplayRole:
            {
                return m_compositions.at(index.row())->name();
            }
            case Qt::DecorationRole:
            {
                return KisIconUtils::loadIcon("tools-wizard");
            }
            case Qt::CheckStateRole: {
                return m_compositions.at(index.row())->isExportEnabled() ? Qt::Checked : Qt::Unchecked;
            }
        }
    }
    return QVariant();
}

bool CompositionModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if (index.isValid()) {
        if (role == Qt::CheckStateRole) {
            Q_ASSERT(index.row() < rowCount());
            Q_ASSERT(index.column() < columnCount());
            if (index.column() == 0) {
                bool exportEnabled = value.toInt() == Qt::Checked;
                KisLayerCompositionSP layerComposition = m_compositions.at(index.row());
                if (layerComposition) {
                    layerComposition->setExportEnabled(exportEnabled);
                }
            }
        }
        return true;
    }
    return false;
}


QVariant CompositionModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    return i18n("Composition");
}


int CompositionModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_compositions.count();
}

int CompositionModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 2;
}

Qt::ItemFlags CompositionModel::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return flags;
}

KisLayerCompositionSP CompositionModel::compositionFromIndex(const QModelIndex& index)
{
    if(index.isValid()) {
        return m_compositions.at(index.row());
    }
    return KisLayerCompositionSP();
}

void CompositionModel::setCompositions(QList< KisLayerCompositionSP > compositions)
{
    m_compositions = compositions;
    beginResetModel();
    endResetModel();
}

