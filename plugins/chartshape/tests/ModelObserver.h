/* This file is part of the KDE project

   Copyright 2009 Johannes Simon <johannes.simon@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCHART_MODELOBSERVER_H
#define KCHART_MODELOBSERVER_H

#include <QObject>
#include <QModelIndex>

class QAbstractItemModel;

struct HeaderDataChange {
    Qt::Orientation orientation;
    int first;
    int last;
    bool valid;
};

struct DataChange {
    QModelIndex topLeft;
    QModelIndex bottomRight;
    bool valid;
};

class ModelObserver : public QObject
{
    Q_OBJECT

public:
    ModelObserver(QAbstractItemModel *source);

private Q_SLOTS:
    void slotRowsInserted(const QModelIndex & parent, int start, int end);
    void slotColumnsInserted(const QModelIndex & parent, int start, int end);
    void slotRowsRemoved(const QModelIndex & parent, int start, int end);
    void slotColumnsRemoved(const QModelIndex & parent, int start, int end);
    void slotHeaderDataChanged(Qt::Orientation, int first, int last);
    void slotDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void slotModelReset();

public:
    QAbstractItemModel *m_source;
    int m_numRows;
    int m_numCols;
    HeaderDataChange m_lastHeaderDataChange;
    DataChange m_lastDataChange;
};

#endif // KCHART_MODELOBSERVER_H
