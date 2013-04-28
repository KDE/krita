/*
 * Copyright (C) 2012 Boudewijn Rempt <boud@kde.org>
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

#include "kis_flipbook_test.h"

#include <qtest_kde.h>

#include "kis_flipbook.h"


void KisFlipbookTest::testRoundtrip()
{
    KisFlipbook flipbook;
    flipbook.setName("test");
    flipbook.addItem("item1");
    flipbook.addItem("item2");

    flipbook.save("flipbooktest.flipbook");

    KisFlipbook flipbook2;
    flipbook2.load("flipbooktest.flipbook");
    QVERIFY(flipbook2.name() == flipbook.name());
    QVERIFY(flipbook2.rowCount() == flipbook.rowCount());
}

QTEST_KDEMAIN(KisFlipbookTest, GUI)

#include "kis_flipbook_test.moc"

