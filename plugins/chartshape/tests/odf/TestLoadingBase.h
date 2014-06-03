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

#ifndef KCHART_TESTLOADING_BASE
#define KCHART_TESTLOADING_BASE

// Qt
#include <QObject>
#include <QString>

// KDE
#include <qtest_kde.h>

// KChart
#include "CellRegion.h"

class KoShape;

namespace KChart {

class ChartShape;
class Table;
class TableSource;
class Axis;

/**
 * Base class for every ODF-loading related unit test.
 *
 * The philosophy is basically to do as many tests as possible by
 * using a helper method from this base class. Lines like these:

 * testLegendElements(QStringList() << "Row 1" << "Row 2" << "Row 3");
 *
 * are much more readable than using flat code by using copy&paste and doing
 * the same gets and checks over again in multiple unit tests.
 *
 */
class TestLoadingBase : public QObject
{
    Q_OBJECT

public:
    TestLoadingBase();

protected slots:
    virtual void initTestCase();

protected:
    // Helper methods to be used by test functions

    // 0) Generics
    void testElementIsVisible(KoShape *element, bool shouldBeVisible);

    // 1) Legend
    void testLegendElements(QStringList labels);

    // 2) Data Sets
    void testDataSetCellRegions(int dataSetNr,
                                CellRegion yDataRegion,
                                CellRegion labelDataRegion    = CellRegion(),
                                CellRegion categoryDataRegion = CellRegion(),
                                CellRegion xDataRegion        = CellRegion(),
                                CellRegion customDataRegion   = CellRegion());

    // 3) Internal Table
    void testHasOnlyInternalTable();
    void testInternalTableSize(int rowCount, int colCount);

    // 4) Title, Subtitle and Footer
    void testTitleText(const QString &text);
    void testSubTitleText(const QString &text);
    void testFooterText(const QString &text);

    // 5) Axes
    void testAxisTitle(Axis *axis, const QString &text);

    Table* internalTable();
    TableSource* tableSource();

    ChartShape *m_chart;
};

} // namespace KChart

namespace QTest {
    template<>
    char *toString(const KChart::CellRegion &region);
}

#endif // KCHART_TESTLOADING_BASE
