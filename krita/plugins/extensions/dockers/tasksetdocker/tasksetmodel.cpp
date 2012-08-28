/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "tasksetmodel.h"

#include <QAction>
#include <KLocale>
#include <KoIcon.h>

TasksetModel::TasksetModel(QObject* parent): QAbstractTableModel(parent)
{
}

TasksetModel::~TasksetModel()
{
}

QVariant TasksetModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid()) {

        switch (role) {
            case Qt::DisplayRole:
            {
                return m_actions.at(index.row())->iconText();
            }
            case Qt::DecorationRole:
            {
                const QIcon icon = m_actions.at(index.row())->icon();
                if (icon.isNull()) {
                    return koIcon("tools-wizard");
                }
                return icon;
            }
        }
    }
    return QVariant();
}

QVariant TasksetModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    return i18n("Task");
}


int TasksetModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_actions.count();
}

int TasksetModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

Qt::ItemFlags TasksetModel::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags flags = /*Qt::ItemIsSelectable |*/ Qt::ItemIsEnabled;
    return flags;
}

void TasksetModel::addAction(QAction* action)
{
    m_actions.append(action);
    reset();
}

QVector< QAction* > TasksetModel::actions()
{
    return m_actions;
}

QAction* TasksetModel::actionFromIndex(const QModelIndex& index)
{
    if(index.isValid()) {
        return m_actions.at(index.row());
    }
    return 0;
}

void TasksetModel::clear()
{
    m_actions.clear();
    reset();
}

#include "tasksetmodel.moc"
