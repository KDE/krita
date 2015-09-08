/* This file is part of the KDE project

   @@COPYRIGHT@@

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
#include "TestLoading.h"

// Qt
#include <QStandardItemModel>

// KoChart
#include "ChartShape.h"
#include "TableSource.h"
#include "Legend.h"

using namespace KoChart;

TestLoading::TestLoading()
    : TestLoadingBase()
{
}

void TestLoading::initTestCase()
{
    // Fake sheet data from embedding document
    m_sheet.setRowCount(6);
    m_sheet.setColumnCount(8);
    // Categories
    m_sheet.setData(m_sheet.index(3, 4), "Pass");
    m_sheet.setData(m_sheet.index(3, 5), "Fail");
    m_sheet.setData(m_sheet.index(3, 6), "NA");
    // Series label
    m_sheet.setData(m_sheet.index(4, 3), "Week");
    QVERIFY(tableSource());
    tableSource()->add("Sheet1", &m_sheet);
    // No actual data needed

    // Tell the chart it's embedded
    m_chart->setUsesInternalModelOnly(false);

    TestLoadingBase::initTestCase();
}

void TestLoading::testInternalTable()
{
    QVERIFY(internalTable());
}

void TestLoading::testDataSets()
{
    TableSource *source = tableSource();
    QVERIFY(source);
                              // y data
    testDataSetCellRegions(0, CellRegion(source, "Sheet1.E5:G5"),
                              // series label
                              CellRegion(source, "Sheet1.D5"),
                              // categories (specified in x-axis)
                              CellRegion(source, "Sheet1.E4:G4"));
}

void TestLoading::testLegend()
{
    testElementIsVisible(m_chart->legend(), true);
    testLegendElements(QStringList() << "Week");
}

QTEST_MAIN(TestLoading)
