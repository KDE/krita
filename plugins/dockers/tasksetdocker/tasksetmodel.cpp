/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "tasksetmodel.h"

#include <QAction>
#include <klocalizedstring.h>
#include <kis_icon.h>

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
                    return KisIconUtils::loadIcon("tools-wizard");
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
    beginResetModel();
    endResetModel();
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
    beginResetModel();
    endResetModel();
}

