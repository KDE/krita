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
#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QObject>
#include <QtSql>

class TableDelegate : public QSqlRelationalDelegate
{
    Q_OBJECT
public:

    TableDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void addDateTimeColumn(int column);
    void addBooleanColumn(int column);
    void setEditable(bool editable);
private:

    bool m_editable{false};
    QVector<int> m_booleanColumns;
    QVector<int> m_dateTimeColumns;
};

/**
 * @brief The TableModel class handles boolean
 * and datatime columns in a custom way.
 */
class TableModel : public QSqlRelationalTableModel
{
    Q_OBJECT
public:

    TableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
    ~TableModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addDateTimeColumn(int column);
    void addBooleanColumn(int column);

private:

    QVector<int> m_booleanColumns;
    QVector<int> m_dateTimeColumns;

};

#endif // TABLEMODEL_H
