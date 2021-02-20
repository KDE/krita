/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addDateTimeColumn(int column);
    void addBooleanColumn(int column);

private:

    QVector<int> m_booleanColumns;
    QVector<int> m_dateTimeColumns;

};

#endif // TABLEMODEL_H
