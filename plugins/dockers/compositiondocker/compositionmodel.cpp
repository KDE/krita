/*
 *  SPDX-FileCopyrightText: 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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

