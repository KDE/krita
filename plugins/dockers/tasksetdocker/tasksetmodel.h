/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
