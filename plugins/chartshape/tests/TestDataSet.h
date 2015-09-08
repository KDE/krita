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

#ifndef KCHART_TESTDATASET_H
#define KCHART_TESTDATASET_H

// KoChart
#include "ChartProxyModel.h"
#include "ChartTableModel.h"
#include "TableSource.h"

using namespace KoChart;

class TestDataSet : public QObject
{
    Q_OBJECT

public:
    TestDataSet();
    
private Q_SLOTS:
    void initTestCase();

    // Tests DataSet::*Data() methods
    void testFooData();
    void testFooDataMultipleTables();
    
private:
    // m_source must be initialized before m_proxyModel
    TableSource m_source;
    ChartProxyModel m_proxyModel;
    ChartTableModel m_sourceModel1, m_sourceModel2;
    Table *m_table1, *m_table2;
};
    
#endif // KCHART_TESTDATASET_H
