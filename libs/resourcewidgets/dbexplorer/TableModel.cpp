/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TableModel.h"

#include <QApplication>

TableDelegate::TableDelegate(QObject *parent)
    : QSqlRelationalDelegate(parent)
{}

QRect getNewRect(const QStyleOptionViewItem &option)
{
    // get the rectangle in the middle of the field
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    QRect newRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                        QSize(option.decorationSize.width() +
                                              5,option.decorationSize.height()),
                                        QRect(option.rect.x() + textMargin, option.rect.y(),
                                              option.rect.width() -
                                              (2 * textMargin), option.rect.height()));
    return newRect;
}

void TableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem viewItemOption(option);
    // Only do this if we are accessing the column with boolean variables.
    if (m_booleanColumns.contains(index.column())) {
        // This basically changes the rectangle in which the check box is drawn.
        viewItemOption.rect = getNewRect(option);
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
        if (m_booleanColumns.contains(index.column())) {
            QStyleOptionViewItem optionCheckable = option;
            optionCheckable.rect = getNewRect(option);
            optionCheckable.features |= QStyleOptionViewItem::HasCheckIndicator;
            return QSqlRelationalDelegate::editorEvent(event, model, optionCheckable, index);
        } else {
            return QSqlRelationalDelegate::editorEvent(event, model, option, index);
        }
    }
    return false;
}

QWidget *TableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (m_editable) {
        if (m_booleanColumns.contains(index.column())) {
            QStyleOptionViewItem optionCheckable = option;
            optionCheckable.features |= QStyleOptionViewItem::HasCheckIndicator;
            optionCheckable.rect = getNewRect(option);
            return QSqlRelationalDelegate::createEditor(parent, optionCheckable, index);
        } else {
            return QSqlRelationalDelegate::createEditor(parent, option, index);
        }
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
    this->setEditStrategy(QSqlTableModel::OnFieldChange);
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

bool TableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (m_booleanColumns.contains(index.column()) && role == Qt::CheckStateRole) {
        // Writing the data when the check box is set to checked.
        if (value == Qt::Checked) {
            // Let's write the new value
            return QSqlTableModel::setData(index, 1, Qt::EditRole);
            // Writing the data when the check box is set to unchecked
        } else if (value == Qt::Unchecked) {
            // Let's write the new value
            return QSqlTableModel::setData(index, 0, Qt::EditRole);
        }
    }

    bool response = QSqlTableModel::setData(index, value, role);
    return response;
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
