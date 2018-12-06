/*
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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
#include "TableModel.h"

#include <QApplication>

TableDelegate::TableDelegate(QObject *parent)
    : QSqlRelationalDelegate(parent)
{}

void TableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem viewItemOption(option);
    // Only do this if we are accessing the column with boolean variables.
    if (m_booleanColumns.contains(index.column())) {
        // This basically changes the rectangle in which the check box is drawn.
        const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        QRect newRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                            QSize(option.decorationSize.width() +
                                                  5,option.decorationSize.height()),
                                            QRect(option.rect.x() + textMargin, option.rect.y(),
                                                  option.rect.width() -
                                                  (2 * textMargin), option.rect.height()));
        viewItemOption.rect = newRect;
    }
    // Draw the check box using the new rectangle.
    QSqlRelationalDelegate::paint(painter, viewItemOption, index);
}

QSize TableDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSqlRelationalDelegate::sizeHint(option, index);
}

bool TableDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (m_editable) {
        return QSqlRelationalDelegate::editorEvent(event, model, option, index);
    }
    return false;
}

QWidget *TableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (m_editable) {
        return QSqlRelationalDelegate::createEditor(parent, option, index);
    }
    return 0;
}

void TableDelegate::addDateTimeColumn(int column)
{
    m_dateTimeColumns << column;
}

void TableDelegate::addBooleanColumn(int column)
{
    m_booleanColumns << column;
}

void TableDelegate::setEditable(bool editable)
{
    m_editable = editable;
}


TableModel::TableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
{

}

TableModel::~TableModel()
{

}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    QVariant d = QSqlRelationalTableModel::data(index, Qt::DisplayRole);
    if (role == Qt::DisplayRole) {
        if (m_dateTimeColumns.contains(index.column())) {
            d = QVariant::fromValue<QString>(QDateTime::fromSecsSinceEpoch(d.toInt()).toString());
        }
        if (m_booleanColumns.contains(index.column())) {
            return QVariant();
        }
    }
    else if (role == Qt::CheckStateRole) {
        if (m_booleanColumns.contains(index.column())) {
            if (d.toInt() == 0) {
                return Qt::Unchecked;
            }
            else {
                return Qt::Checked;
            }
        }
        return QVariant();
    }
    return d;

}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QSqlRelationalTableModel::flags((index));
    if (m_booleanColumns.contains(index.column())) {
        f |= Qt::ItemIsUserCheckable;
    }
    return f;

}

void TableModel::addDateTimeColumn(int column)
{
    m_dateTimeColumns << column;
}

void TableModel::addBooleanColumn(int column)
{
    m_booleanColumns << column;
}
