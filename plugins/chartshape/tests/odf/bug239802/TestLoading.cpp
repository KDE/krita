/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>
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

// Own
#include "TestLoading.h"

// KoChart
#include "ChartShape.h"
#include "PlotArea.h"
#include "Legend.h"

using namespace KoChart;

TestLoading::TestLoading()
    : TestLoadingBase()
{
}

void TestLoading::testInternalTable()
{
    testHasOnlyInternalTable();
    testInternalTableSize(5, 4);
}
void TestLoading::testPlotArea()
{
    testElementIsVisible(m_chart->plotArea(), true);
}

void TestLoading::testLegend()
{
    testElementIsVisible(m_chart->legend(), true);
    testLegendElements(QStringList() << "Row 1" << "Row 2" << "Row 3" << "Row 4");
}

QTEST_MAIN(TestLoading)
