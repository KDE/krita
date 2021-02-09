/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "commandmodel.h"

#include <KLocalizedString>
#include <QAction>
#include <QDebug>

CommandModel::CommandModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void CommandModel::refresh(QVector<QPair<QString, QAction *>> actionList)
{
    QVector<Item> temp;
    temp.reserve(actionList.size());
    for (auto action : actionList) {
        temp.push_back({action.first, action.second, 0});
    }

    beginResetModel();
    m_rows = std::move(temp);
    endResetModel();
}

QVariant CommandModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    };

    auto entry = m_rows[index.row()];
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
        if (col == 0) {
            return QString(entry.component + QStringLiteral(": ") + KLocalizedString::removeAcceleratorMarker(entry.action->text()));
        } else {
            return entry.action->shortcut().toString();
        }
    case Qt::DecorationRole:
        if (col == 0) {
            return entry.action->icon();
        }
        break;
    case Qt::TextAlignmentRole:
        if (col == 0) {
            return Qt::AlignLeft;
        } else {
            return Qt::AlignRight;
        }
    case Qt::UserRole: {
        QVariant v;
        v.setValue(entry.action);
        return v;
    }
    case Role::Score:
        return entry.score;
    }

    return {};
}
