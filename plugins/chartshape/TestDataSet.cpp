/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>

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
#include "TestDataSet.h"

// Qt
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QDebug>
#include <QString>
#include <QVariant>

// KChart
#include "DataSet.h"
#include "CellRegion.h"


namespace QTest {
    template<>
    char *toString( const CellRegion &region )
    {
        return qstrdup( region.toString().toAscii().data() );
    }
}

using namespace KChart;

TestDataSet::TestDataSet()
    : m_source()
    , m_proxyModel( &m_source )
    , m_sourceModel()
    , m_table( 0 )
{
}

void TestDataSet::initTestCase()
{
    m_table = m_source.add( "Table1", &m_sourceModel );

    m_sourceModel.setRowCount( 4 );
    m_sourceModel.setColumnCount( 5 );

    // Vertical header data
    m_sourceModel.setData( m_sourceModel.index( 1, 0 ), "Row 1" );
    m_sourceModel.setData( m_sourceModel.index( 2, 0 ), "Row 2" );
    m_sourceModel.setData( m_sourceModel.index( 3, 0 ), "Row 3" );

    // Horizontal header data
    m_sourceModel.setData( m_sourceModel.index( 0, 1 ), "Column 1" );
    m_sourceModel.setData( m_sourceModel.index( 0, 2 ), "Column 2" );
    m_sourceModel.setData( m_sourceModel.index( 0, 3 ), "Column 3" );
    m_sourceModel.setData( m_sourceModel.index( 0, 4 ), "Column 4" );

    // First row
    m_sourceModel.setData( m_sourceModel.index( 1, 1 ), 7.2 );
    m_sourceModel.setData( m_sourceModel.index( 1, 2 ), 1.8 );
    m_sourceModel.setData( m_sourceModel.index( 1, 3 ), 9.4 );
    m_sourceModel.setData( m_sourceModel.index( 1, 4 ), 1.5 );

    // Second row
    m_sourceModel.setData( m_sourceModel.index( 2, 1 ), 8.4 );
    m_sourceModel.setData( m_sourceModel.index( 2, 2 ), 2.9 );
    m_sourceModel.setData( m_sourceModel.index( 2, 3 ), 3.7 );
    m_sourceModel.setData( m_sourceModel.index( 2, 4 ), 5.5 );

    // Third row
    m_sourceModel.setData( m_sourceModel.index( 3, 1 ), 2.9 );
    m_sourceModel.setData( m_sourceModel.index( 3, 2 ), 5.3 );
    m_sourceModel.setData( m_sourceModel.index( 3, 3 ), 6.4 );
    m_sourceModel.setData( m_sourceModel.index( 3, 4 ), 2.1 );
}

void TestDataSet::testFooData()
{
    DataSet dataSet( 0, 0 );

    dataSet.setLabelDataRegion( CellRegion( m_table, QPoint( 1, 2 ) ) );
    dataSet.setCategoryDataRegion( CellRegion( m_table, QRect( 2, 1, 4, 1 ) ) );
    dataSet.setXDataRegion( CellRegion( m_table, QRect( 2, 2, 4, 1 ) ) );
    dataSet.setYDataRegion( CellRegion( m_table, QRect( 2, 3, 4, 1 ) ) );
    dataSet.setCustomDataRegion( CellRegion( m_table, QRect( 2, 4, 4, 1 ) ) );

    QCOMPARE( dataSet.size(), 4 );

    QCOMPARE( dataSet.labelData(), QVariant( "Row 1" ) );

    QCOMPARE( dataSet.categoryData( 0 ), QVariant( "Column 1" ) );
    QCOMPARE( dataSet.categoryData( 1 ), QVariant( "Column 2" ) );
    QCOMPARE( dataSet.categoryData( 2 ), QVariant( "Column 3" ) );
    QCOMPARE( dataSet.categoryData( 3 ), QVariant( "Column 4" ) );

    QCOMPARE( dataSet.xData( 0 ), QVariant( 7.2 ) );
    QCOMPARE( dataSet.xData( 1 ), QVariant( 1.8 ) );
    QCOMPARE( dataSet.xData( 2 ), QVariant( 9.4 ) );
    QCOMPARE( dataSet.xData( 3 ), QVariant( 1.5 ) );

    QCOMPARE( dataSet.yData( 0 ), QVariant( 8.4 ) );
    QCOMPARE( dataSet.yData( 1 ), QVariant( 2.9 ) );
    QCOMPARE( dataSet.yData( 2 ), QVariant( 3.7 ) );
    QCOMPARE( dataSet.yData( 3 ), QVariant( 5.5 ) );

    QCOMPARE( dataSet.customData( 0 ), QVariant( 2.9 ) );
    QCOMPARE( dataSet.customData( 1 ), QVariant( 5.3 ) );
    QCOMPARE( dataSet.customData( 2 ), QVariant( 6.4 ) );
    QCOMPARE( dataSet.customData( 3 ), QVariant( 2.1 ) );
}

QTEST_MAIN( TestDataSet )
