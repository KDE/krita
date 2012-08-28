/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "compositionmodel.h"

#include <KoIcon.h>

#include <QAction>
#include <KLocale>

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
                    return koIcon("tools-wizard");
            }
        }
    }
    return QVariant();
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
    return 1;
}

Qt::ItemFlags CompositionModel::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags flags = /*Qt::ItemIsSelectable |*/ Qt::ItemIsEnabled;
    return flags;
}

KisLayerComposition* CompositionModel::compositionFromIndex(const QModelIndex& index)
{
    if(index.isValid()) {
        return m_compositions.at(index.row());
    }
    return 0;
}

void CompositionModel::setCompositions(QList< KisLayerComposition* > compositions)
{
    m_compositions = compositions;
    reset();
}

#include "compositionmodel.moc"
