/* This file is part of the KDE project

   Copyright 2008, 2010 Johannes Simon <johannes.simon@gmail.com>

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
#include "TestProxyModel.h"

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

TestProxyModel::TestProxyModel()
    : m_source()
    , m_proxyModel( &m_source )
    , m_sourceModel()
    , m_table( 0 )
{
}

void TestProxyModel::initTestCase()
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

    QRect selection( QPoint( 1, 1 ),
                     QSize( m_sourceModel.columnCount(), m_sourceModel.rowCount() ) );

    m_proxyModel.reset( CellRegion( m_table, selection ) );
}

void TestProxyModel::testWithoutLabels()
{
    QList<DataSet*> dataSets;

    // Horizontal data direction
    m_proxyModel.setDataDirection( Qt::Horizontal );
    m_proxyModel.setFirstColumnIsLabel( false );
    m_proxyModel.setFirstRowIsLabel( false );

    dataSets = m_proxyModel.dataSets();

    QCOMPARE( dataSets.size(), 4 );
    QCOMPARE( dataSets[0]->size(), 5 );
    QCOMPARE( dataSets[1]->size(), 5 );
    QCOMPARE( dataSets[2]->size(), 5 );
    QCOMPARE( dataSets[3]->size(), 5 );

    QCOMPARE( dataSets[0]->yDataRegion(), CellRegion( m_table, QRect( 1, 1, 5, 1 ) ) );
    QCOMPARE( dataSets[0]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[1]->yDataRegion(), CellRegion( m_table, QRect( 1, 2, 5, 1 ) ) );
    QCOMPARE( dataSets[1]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[2]->yDataRegion(), CellRegion( m_table, QRect( 1, 3, 5, 1 ) ) );
    QCOMPARE( dataSets[2]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[3]->yDataRegion(), CellRegion( m_table, QRect( 1, 4, 5, 1 ) ) );
    QCOMPARE( dataSets[3]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->categoryDataRegion(), CellRegion() );


    // Vertical data direction
    m_proxyModel.setDataDirection( Qt::Vertical );

    dataSets = m_proxyModel.dataSets();

    QCOMPARE( dataSets.size(), 5 );
    QCOMPARE( dataSets[0]->size(), 4 );
    QCOMPARE( dataSets[1]->size(), 4 );
    QCOMPARE( dataSets[2]->size(), 4 );
    QCOMPARE( dataSets[3]->size(), 4 );
    QCOMPARE( dataSets[4]->size(), 4 );

    QCOMPARE( dataSets[0]->yDataRegion(), CellRegion( m_table, QRect( 1, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[0]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[1]->yDataRegion(), CellRegion( m_table, QRect( 2, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[1]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[2]->yDataRegion(), CellRegion( m_table, QRect( 3, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[2]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[3]->yDataRegion(), CellRegion( m_table, QRect( 4, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[3]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[4]->yDataRegion(), CellRegion( m_table, QRect( 5, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[4]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[4]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[4]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[4]->categoryDataRegion(), CellRegion() );
}

void TestProxyModel::testFirstRowAsLabel()
{
    QList<DataSet*> dataSets;

    // Horizontal data direction
    m_proxyModel.setDataDirection( Qt::Horizontal );

    m_proxyModel.setFirstColumnIsLabel( false );

    // With first row as category data
    m_proxyModel.setFirstRowIsLabel( true );
    dataSets = m_proxyModel.dataSets();

    QCOMPARE( dataSets.size(), 3 );
    QCOMPARE( dataSets[0]->size(), 5 );
    QCOMPARE( dataSets[1]->size(), 5 );
    QCOMPARE( dataSets[2]->size(), 5 );

    QCOMPARE( dataSets[0]->yDataRegion(), CellRegion( m_table, QRect( 1, 2, 5, 1 ) ) );
    QCOMPARE( dataSets[0]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 1, 5, 1 ) ) );

    QCOMPARE( dataSets[1]->yDataRegion(), CellRegion( m_table, QRect( 1, 3, 5, 1 ) ) );
    QCOMPARE( dataSets[1]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 1, 5, 1 ) ) );

    QCOMPARE( dataSets[2]->yDataRegion(), CellRegion( m_table, QRect( 1, 4, 5, 1 ) ) );
    QCOMPARE( dataSets[2]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 1, 5, 1 ) ) );


    // Vertical data direction
    m_proxyModel.setDataDirection( Qt::Vertical );

    dataSets = m_proxyModel.dataSets();

    QCOMPARE( dataSets.size(), 5 );
    QCOMPARE( dataSets[0]->size(), 3 );
    QCOMPARE( dataSets[1]->size(), 3 );
    QCOMPARE( dataSets[2]->size(), 3 );

    QCOMPARE( dataSets[0]->yDataRegion(), CellRegion( m_table, QRect( 1, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[0]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->labelDataRegion(), CellRegion( m_table, QRect( 1, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[1]->yDataRegion(), CellRegion( m_table, QRect( 2, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[1]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->labelDataRegion(), CellRegion( m_table, QRect( 2, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[1]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[2]->yDataRegion(), CellRegion( m_table, QRect( 3, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[2]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->labelDataRegion(), CellRegion( m_table, QRect( 3, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[2]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[3]->yDataRegion(), CellRegion( m_table, QRect( 4, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[3]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->labelDataRegion(), CellRegion( m_table, QRect( 4, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[3]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[4]->yDataRegion(), CellRegion( m_table, QRect( 5, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[4]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[4]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[4]->labelDataRegion(), CellRegion( m_table, QRect( 5, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[4]->categoryDataRegion(), CellRegion() );
}

void TestProxyModel::testFirstColumnAsLabel()
{
    QList<DataSet*> dataSets;

    // Horizontal data direction
    m_proxyModel.setDataDirection( Qt::Horizontal );

    m_proxyModel.setFirstRowIsLabel( false );

    // With first column as label data
    m_proxyModel.setFirstColumnIsLabel( true );
    dataSets = m_proxyModel.dataSets();

    QCOMPARE( dataSets.size(), 4 );
    QCOMPARE( dataSets[0]->size(), 4 );
    QCOMPARE( dataSets[1]->size(), 4 );
    QCOMPARE( dataSets[2]->size(), 4 );
    QCOMPARE( dataSets[3]->size(), 4 );

    QCOMPARE( dataSets[0]->yDataRegion(), CellRegion( m_table, QRect( 2, 1, 4, 1 ) ) );
    QCOMPARE( dataSets[0]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->labelDataRegion(), CellRegion( m_table, QRect( 1, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[1]->yDataRegion(), CellRegion( m_table, QRect( 2, 2, 4, 1 ) ) );
    QCOMPARE( dataSets[1]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->labelDataRegion(), CellRegion( m_table, QRect( 1, 2, 1, 1 ) ) );
    QCOMPARE( dataSets[1]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[2]->yDataRegion(), CellRegion( m_table, QRect( 2, 3, 4, 1 ) ) );
    QCOMPARE( dataSets[2]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->labelDataRegion(), CellRegion( m_table, QRect( 1, 3, 1, 1 ) ) );
    QCOMPARE( dataSets[2]->categoryDataRegion(), CellRegion() );

    QCOMPARE( dataSets[3]->yDataRegion(), CellRegion( m_table, QRect( 2, 4, 4, 1 ) ) );
    QCOMPARE( dataSets[3]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->labelDataRegion(), CellRegion( m_table, QRect( 1, 4, 1, 1 ) ) );
    QCOMPARE( dataSets[3]->categoryDataRegion(), CellRegion() );


    // Vertical data direction
    m_proxyModel.setDataDirection( Qt::Vertical );

    dataSets = m_proxyModel.dataSets();

    QCOMPARE( dataSets.size(), 4 );
    QCOMPARE( dataSets[0]->size(), 4 );
    QCOMPARE( dataSets[1]->size(), 4 );
    QCOMPARE( dataSets[2]->size(), 4 );
    QCOMPARE( dataSets[3]->size(), 4 );

    QCOMPARE( dataSets[0]->yDataRegion(), CellRegion( m_table, QRect( 2, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[0]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 1, 1, 4 ) ) );

    QCOMPARE( dataSets[1]->yDataRegion(), CellRegion( m_table, QRect( 3, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[1]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 1, 1, 4 ) ) );

    QCOMPARE( dataSets[2]->yDataRegion(), CellRegion( m_table, QRect( 4, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[2]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 1, 1, 4 ) ) );

    QCOMPARE( dataSets[3]->yDataRegion(), CellRegion( m_table, QRect( 5, 1, 1, 4 ) ) );
    QCOMPARE( dataSets[3]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->labelDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 1, 1, 4 ) ) );
}

void TestProxyModel::testFirstRowAndColumnAsLabels()
{
    QList<DataSet*> dataSets;

    // Horizontal data direction
    m_proxyModel.setDataDirection( Qt::Horizontal );

    // With first row as category data
    m_proxyModel.setFirstRowIsLabel( true );
    // ...and first column as label data
    m_proxyModel.setFirstColumnIsLabel( true );

    dataSets = m_proxyModel.dataSets();

    QCOMPARE( dataSets.size(), 3 );
    QCOMPARE( dataSets[0]->size(), 4 );
    QCOMPARE( dataSets[1]->size(), 4 );
    QCOMPARE( dataSets[2]->size(), 4 );

    QCOMPARE( dataSets[0]->yDataRegion(), CellRegion( m_table, QRect( 2, 2, 4, 1 ) ) );
    QCOMPARE( dataSets[0]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->labelDataRegion(), CellRegion( m_table, QRect( 1, 2, 1, 1 ) ) );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion( m_table, QRect( 2, 1, 4, 1 ) ) );

    QCOMPARE( dataSets[1]->yDataRegion(), CellRegion( m_table, QRect( 2, 3, 4, 1 ) ) );
    QCOMPARE( dataSets[1]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->labelDataRegion(), CellRegion( m_table, QRect( 1, 3, 1, 1 ) ) );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion( m_table, QRect( 2, 1, 4, 1 ) ) );

    QCOMPARE( dataSets[2]->yDataRegion(), CellRegion( m_table, QRect( 2, 4, 4, 1 ) ) );
    QCOMPARE( dataSets[2]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->labelDataRegion(), CellRegion( m_table, QRect( 1, 4, 1, 1 ) ) );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion( m_table, QRect( 2, 1, 4, 1 ) ) );


    // Vertical data direction
    m_proxyModel.setDataDirection( Qt::Vertical );

    dataSets = m_proxyModel.dataSets();

    QCOMPARE( dataSets.size(), 4 );
    QCOMPARE( dataSets[0]->size(), 3 );
    QCOMPARE( dataSets[1]->size(), 3 );
    QCOMPARE( dataSets[2]->size(), 3 );
    QCOMPARE( dataSets[3]->size(), 3 );

    QCOMPARE( dataSets[0]->yDataRegion(), CellRegion( m_table, QRect( 2, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[0]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[0]->labelDataRegion(), CellRegion( m_table, QRect( 2, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[0]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 2, 1, 3 ) ) );

    QCOMPARE( dataSets[1]->yDataRegion(), CellRegion( m_table, QRect( 3, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[1]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[1]->labelDataRegion(), CellRegion( m_table, QRect( 3, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[1]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 2, 1, 3 ) ) );

    QCOMPARE( dataSets[2]->yDataRegion(), CellRegion( m_table, QRect( 4, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[2]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[2]->labelDataRegion(), CellRegion( m_table, QRect( 4, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[2]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 2, 1, 3 ) ) );

    QCOMPARE( dataSets[3]->yDataRegion(), CellRegion( m_table, QRect( 5, 2, 1, 3 ) ) );
    QCOMPARE( dataSets[3]->xDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->customDataRegion(), CellRegion() );
    QCOMPARE( dataSets[3]->labelDataRegion(), CellRegion( m_table, QRect( 5, 1, 1, 1 ) ) );
    QCOMPARE( dataSets[3]->categoryDataRegion(), CellRegion( m_table, QRect( 1, 2, 1, 3 ) ) );
}

QTEST_MAIN( TestProxyModel )
