/* This file is part of the KDE project

   Copyright (C) 2010 Johannes Simon <johannes.simon@gmail.com>
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
 * Boston, MA 02110-1301, USA.
 */

#ifndef KCHART_TESTCELLREGION_H
#define KCHART_TESTCELLREGION_H

// Qt
#include <QObject>
#include <QStandardItemModel>

// KoChart
#include "CellRegion.h"
#include "TableSource.h"

using namespace KoChart;

class TestCellRegion : public QObject
{
    Q_OBJECT
public:
    TestCellRegion();

private Q_SLOTS:
    void init();
    void testToStringSingleTable();
    void testSkippedTableEntry();
    void testFromStringSingleTable();
    void testToStringWithSpecialCharactersSingleTable();
    void testFromStringWithSpecialCharactersSingleTable();
    void testTableNameChangeSingleTable();
    void testToStringMultipleTables();
    void testFromStringMultipleTables();
    void testToStringWithSpecialCharactersMultipleTables();
    void testFromStringWithSpecialCharactersMultipleTables();
    void testTableNameChangeMultipleTables();
    void testListOfRegions();
    void testListOfRegions2();

private:
    TableSource m_source;
    CellRegion m_region1, m_region2;
    QStandardItemModel m_model1, m_model2;
};

#endif // KCHART_TESTCELLREGION_H
