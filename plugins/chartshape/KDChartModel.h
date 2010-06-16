/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
   Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
     Contact: Suresh Chande suresh.chande@nokia.com

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

#ifndef KCHART_KDCHARTMODEL_H
#define KCHART_KDCHARTMODEL_H


// Qt
#include <QAbstractItemModel>

// KChart
#include "ChartShape.h"


namespace KChart {

/**
 * Takes a list of DataSet's and compiles them into a
 * QAbstractItemModel for use with KDChart.
 *
 * Data sets in this model are aligned column-wise. Each column
 * occupies dimension() columns. For example, for an X/Y chart,
 * the data of this model would be structured as follows:
 *
 *             Brush 0       Brush 1
 *             Pen 0         Pen 1
 *             Label 0       Label 1
 * -----------|------|------|------|------|
 * Category 1 | x0,0 | y0,0 | x1,0 | x1,0 |
 * -----------|------|------|------|------|
 * Category 2 | x0,1 | y0,1 | x1,1 | x1,1 |
 * -----------|------|------|------|------|
 * Category 3 | x0,2 | y0,2 | x1,2 | x1,2 |
 * -----------|------|------|------|------|
 *
 */

class CHARTSHAPELIB_EXPORT KDChartModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    KDChartModel( QObject *parent = 0 );
    ~KDChartModel();

    enum DataRole {
        XDataRole,
        YDataRole,
        ZDataRole,
        LabelDataRole,
        CategoryDataRole,
        CustomDataRole,
        BrushDataRole,
        PenDataRole,
        PieAttributesRole
    };
    
    /**
     * Specifies in what direction a data set 'points'. More specifically,
     * if the data direction is Qt::Vertical, a data set occupies one
     * column (in case only one data dimension is being used).
     */
    void setDataDirection( Qt::Orientation direction );
    /**
     * See \a setDataDirection
     */
    Qt::Orientation dataDirection() const;
    /**
     * Returns the opposite of dataDirection().
     */
    Qt::Orientation categoryDirection() const;

public slots:
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    void slotColumnsInserted( const QModelIndex& parent, int start, int end );
    
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    
    void setDataDimensions( int dataDimensions );
    int dataDimensions() const;

    void addDataSet( DataSet *dataSet, bool silent = false );
    void removeDataSet( DataSet *dataSet, bool silent = false );
    QList<DataSet*> dataSets() const;

    void dataSetChanged( DataSet *dataSet );
    void dataSetChanged( DataSet *dataSet, DataRole role, int first, int last = -1 );
    void dataSetSizeChanged( DataSet *dataSet, int newSize );
    
    void emitReset();

private:
    class Private;
    Private *const d;
};

} // namespace KChart

#endif // KCHART_KDCHARTMODEL_H
