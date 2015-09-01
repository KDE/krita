/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2009-2010 C. Boemann <cbo@kogmbh.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TestKoTableColumnAndRowStyleManager.h"

#include "styles/KoTableColumnStyle.h"
#include "styles/KoTableRowStyle.h"
#include "../KoTableColumnAndRowStyleManager.h"

#include <QTest>

void TestKoTableColumnAndRowStyleManager::testColumns()
{
    KoTableColumnAndRowStyleManager manager;

    KoTableColumnStyle *style1 = new KoTableColumnStyle();
    KoTableColumnStyle *style2 = new KoTableColumnStyle();
    KoTableColumnStyle *style3 = new KoTableColumnStyle();

    manager.setColumnStyle(0, *style1);
    manager.setColumnStyle(2, *style2);

    QVERIFY(manager.columnStyle(0) == *style1);
    //column 1 is a default inited style
    QVERIFY(manager.columnStyle(2) == *style2);

    manager.insertColumns(1, 2, *style3);

    QVERIFY(manager.columnStyle(0) == *style1);
    QVERIFY(manager.columnStyle(1) == *style3);
    QVERIFY(manager.columnStyle(2) == *style3);
    //column 3 is a default inited style
    QVERIFY(manager.columnStyle(4) == *style2);

    manager.removeColumns(2, 2);

    QVERIFY(manager.columnStyle(0) == *style1);
    QVERIFY(manager.columnStyle(1) == *style3);
    QVERIFY(manager.columnStyle(2) == *style2);
}

void TestKoTableColumnAndRowStyleManager::testRows()
{
    KoTableColumnAndRowStyleManager manager;

    KoTableRowStyle *style1 = new KoTableRowStyle();
    KoTableRowStyle *style2 = new KoTableRowStyle();
    KoTableRowStyle *style3 = new KoTableRowStyle();

    manager.setRowStyle(0, *style1);
    manager.setRowStyle(2, *style2);

    QVERIFY(manager.rowStyle(0) == *style1);
    QVERIFY(manager.rowStyle(2) == *style2);

    manager.insertRows(1, 2, *style3);

    QVERIFY(manager.rowStyle(0) == *style1);
    QVERIFY(manager.rowStyle(1) == *style3);
    QVERIFY(manager.rowStyle(2) == *style3);
    //row 3 is a default inited style
    QVERIFY(manager.rowStyle(4) == *style2);

    manager.removeRows(2, 2);

    QVERIFY(manager.rowStyle(0) == *style1);
    QVERIFY(manager.rowStyle(1) == *style3);
    QVERIFY(manager.rowStyle(2) == *style2);
}

QTEST_MAIN(TestKoTableColumnAndRowStyleManager)
