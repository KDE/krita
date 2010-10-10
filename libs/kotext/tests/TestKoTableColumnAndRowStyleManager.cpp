/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2009-2010 Casper Boemann <casper.boemann@kogmbh.com>
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
#include "../KoTableColumnAndRowStyleManager.h"

void TestKoTableColumnAndRowStyleManager::testManager()
{
    KoTableColumnAndRowStyleManager manager;

    KoTableColumnStyle *style1 = new KoTableColumnStyle();
    KoTableColumnStyle *style2 = new KoTableColumnStyle();

    manager.setColumnStyle(0, *style1);
    manager.setColumnStyle(2, *style2);

    QVERIFY(manager.columnStyle(0) == *style1);
    QVERIFY(manager.columnStyle(2) == *style2);
}

QTEST_MAIN(TestKoTableColumnAndRowStyleManager)
#include <TestKoTableColumnAndRowStyleManager.moc>
