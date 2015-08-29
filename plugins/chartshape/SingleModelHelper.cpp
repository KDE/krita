/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>

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

// Own
#include "SingleModelHelper.h"

// Qt
#include <QAbstractItemModel>

// KoChart
#include "TableSource.h"
#include "ChartProxyModel.h"
#include "CellRegion.h"

using namespace KoChart;

SingleModelHelper::SingleModelHelper(Table *table, ChartProxyModel *proxyModel)
    : m_table(table)
    , m_proxyModel(proxyModel)
{
    Q_ASSERT(table);
    Q_ASSERT(proxyModel);

    QAbstractItemModel *model = table->model();
    connect(model, SIGNAL(modelReset()),
            this,  SLOT(slotModelStructureChanged()));
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this,  SLOT(slotModelStructureChanged()));
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this,  SLOT(slotModelStructureChanged()));
    connect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this,  SLOT(slotModelStructureChanged()));
    connect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            this,  SLOT(slotModelStructureChanged()));

    // Initialize the proxy with this model
    slotModelStructureChanged();
}

void SingleModelHelper::slotModelStructureChanged()
{
    QAbstractItemModel *model = m_table->model();
    const int columnCount = model->columnCount();
    const int rowCount = model->rowCount();
    CellRegion region(m_table);
    if (columnCount >= 1 && rowCount >= 1) {
        QPoint topLeft(1, 1);
        QPoint bottomRight(columnCount, rowCount);
        region.add(QRect(topLeft, bottomRight));
    }
    m_proxyModel->reset(region);
}
