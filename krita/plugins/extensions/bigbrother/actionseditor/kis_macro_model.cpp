/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_macro_model.h"
#include <recorder/kis_macro.h>
#include <recorder/kis_recorded_action.h>

KisMacroModel::KisMacroModel(KisMacro* _macro) : m_macro(_macro)
{
}

KisMacroModel::~KisMacroModel()
{
}

int KisMacroModel::rowCount( const QModelIndex & parent ) const
{
    Q_UNUSED(parent);
    return m_macro->actions().count();
}

QVariant KisMacroModel::data( const QModelIndex & index, int role ) const
{
    if(index.isValid()) {
        if( role == Qt::DisplayRole || role == Qt::EditRole )
        {
            return m_macro->actions()[index.row()]->name();
        }
    }
    return QVariant();
}

bool KisMacroModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
    if(index.isValid()) {
        if( role == Qt::EditRole )
        {
            m_macro->actions()[index.row()]->setName(value.toString());
            return true;
        }
    }
    return false;
}

Qt::ItemFlags KisMacroModel::flags( const QModelIndex & index ) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}
