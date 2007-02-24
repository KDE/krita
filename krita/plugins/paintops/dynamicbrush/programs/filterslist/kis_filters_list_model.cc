/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include "kis_filters_list_model.h"

#include "kis_dynamic_transformation.h"
#include "kis_darken_transformation.h"
#include "kis_size_transformation.h"

#include "kis_filters_list_dynamic_program.h"

KisFiltersListModel::KisFiltersListModel(KisFiltersListDynamicProgram* db, QObject *parent)
     : QAbstractListModel(parent), m_program(db), m_currentFilterType(0)
{
}

KisFiltersListModel::~KisFiltersListModel()
{
}

QVariant KisFiltersListModel::data(const QModelIndex &index, int role) const
{
     if (!index.isValid())
         return QVariant();

     if (role != Qt::DisplayRole and role != Qt::EditRole)
        return QVariant();

     return QVariant(  m_program->transfoAt( index.row() )->name() );
}

int KisFiltersListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_program->countTransformations();
}


Qt::ItemFlags KisFiltersListModel::flags(const QModelIndex &index) const
{
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void KisFiltersListModel::setCurrentFilterType(int filterType)
{
    m_currentFilterType = filterType;
}

void KisFiltersListModel::addNewFilter()
{
    kDebug() << "addNewFilter " << m_currentFilterType << endl;
    KisDynamicTransformation* transfo = 0;
    switch(m_currentFilterType)
    {
        case 0:
            transfo = new KisDarkenTransformation(0);
            break;
        case 1:
            transfo = new KisSizeTransformation(0, 0);
            break;
        default:
            kDebug() << "Unknown filter type" << endl;
            return;
    }
    beginInsertRows( createIndex(0, 0, 0), m_program->countTransformations(), m_program->countTransformations());
    m_program->appendTransformation( transfo );
    endInsertRows();
}

void KisFiltersListModel::deleteCurrentFilter()
{
    kDebug() << "Remove filter at " << m_currentTransformation << endl;
    if( m_currentTransformation == -1)
        return;
    beginRemoveRows( createIndex(0, 0, 0), m_currentTransformation, m_currentTransformation);
    m_program->removeTransformationAt( m_currentTransformation );
    endRemoveRows();
    m_currentTransformation = -1;
}

void KisFiltersListModel::setCurrentFilter(const QModelIndex& midx)
{
    kDebug() << "Set current filter " << midx.row() << endl;
    m_currentTransformation = midx.row();
}

#include "kis_filters_list_model.moc"
