/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TASKSETMODEL_H
#define TASKSETMODEL_H

#include <QModelIndex>

#include <kis_types.h>

class QAction;
class TasksetModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TasksetModel(QObject* parent = 0);
    ~TasksetModel() override;
    
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void addAction(QAction* action);
    QAction* actionFromIndex(const QModelIndex& index);
    QVector<QAction*> actions();

public Q_SLOTS:
    void clear();
private:
    QVector<QAction*> m_actions;
};

#endif // TASKSETMODEL_H
