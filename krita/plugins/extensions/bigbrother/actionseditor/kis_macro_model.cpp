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

int KisMacroModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return m_macro->actions().count();
}

QVariant KisMacroModel::data(const QModelIndex & index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return m_macro->actions()[index.row()]->name();
        }
    }
    return QVariant();
}

bool KisMacroModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid()) {
        if (role == Qt::EditRole) {
            m_macro->actions()[index.row()]->setName(value.toString());
            return true;
        }
    }
    return false;
}

Qt::ItemFlags KisMacroModel::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

bool KisMacroModel::removeRows(int row, int count, const QModelIndex & parent)
{
    beginRemoveRows(parent, row, row + count);

    QList<KisRecordedAction*> actions;
    for (int i = row; i < row + count; ++i) {
        actions.push_back(m_macro->actions()[i]);
    }
    m_macro->removeActions(actions);

    endRemoveRows();
    return true;
}

void KisMacroModel::duplicateAction(const QModelIndex& index)
{
    if (index.isValid()) {
        KisRecordedAction* action = m_macro->actions()[index.row()];
        beginInsertRows(QModelIndex(), index.row(), index.row());
        m_macro->addAction(*action, action);
        endInsertRows();
    }
}

void KisMacroModel::raise(const QModelIndex& index)
{
    if (index.isValid()) {
        KisRecordedAction* action = m_macro->actions()[index.row()];
        KisRecordedAction* before = m_macro->actions()[index.row() - 1];
        m_macro->moveAction(action, before);
        emit(dataChanged(createIndex(index.row() - 1, 0), index));
    }
}

void KisMacroModel::lower(const QModelIndex& index)
{
    if (index.isValid()) {
        KisRecordedAction* before = m_macro->actions()[index.row()];
        KisRecordedAction* action = m_macro->actions()[index.row() + 1];
        m_macro->moveAction(action, before);
        emit(dataChanged(index, createIndex(index.row() + 1, 0)));
    }
}
