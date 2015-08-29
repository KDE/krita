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

#ifndef KCHART_TESTTABLESOURCE_H
#define KCHART_TESTTABLESOURCE_H

// Qt
#include <QObject>
#include <QStandardItemModel>

// KoChart
#include "TableSource.h"

using namespace KoChart;

class TestTableSource : public QObject
{
    Q_OBJECT

public:
    TestTableSource();

private Q_SLOTS:
    void init();
    void testAdding();
    void testRenaming();
    void testRemoval();

    /**
     * Tests for sheet access model functionality
     */
    void testAdding_SAM();
    void testRenaming_SAM();
    void testRemoval_SAM();

private:
    TableSource m_source;
    QStandardItemModel m_table1, m_table2, m_table3, m_table4;
    QStandardItemModel m_sheetAccessModel;
};

#endif // KCHART_TESTTABLESOURCE_H
