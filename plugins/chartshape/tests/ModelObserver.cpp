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

#include "ModelObserver.h"
#include <QAbstractItemModel>
#include <QDebug>

ModelObserver::ModelObserver( QAbstractItemModel *source )
    : QObject()
{
    m_source = source;
    m_numRows = 0;
    m_numCols = 0;
}

void ModelObserver::slotRowsInserted( const QModelIndex & parent, int start, int end )
{
    Q_ASSERT( start <= end );
    Q_ASSERT( end <= m_numRows );

    m_numRows += end - start + 1;

    qDebug() << "m_numRows: " << m_numRows;
}

void ModelObserver::slotColumnsInserted( const QModelIndex & parent, int start, int end )
{
    Q_ASSERT( start <= end );
    Q_ASSERT( end <= m_numCols );

    m_numCols += end - start + 1;

    qDebug() << "m_numCols: " << m_numCols;
}

void ModelObserver::slotRowsRemoved( const QModelIndex & parent, int start, int end )
{
    Q_ASSERT( start <= end );
    Q_ASSERT( end <= m_numRows );

    m_numRows -= end - start + 1;

    qDebug() << "m_numRows: " << m_numRows;
}

void ModelObserver::slotColumnsRemoved( const QModelIndex & parent, int start, int end )
{
    Q_ASSERT( start <= end );
    Q_ASSERT( end <= m_numCols );

    m_numCols -= end - start + 1;

    qDebug() << "m_numCols: " << m_numCols;
}

void ModelObserver::slotModelReset()
{
    qDebug() << "TestModel was reset";
    m_numRows = m_source->rowCount();
    m_numCols = m_source->columnCount();
}
