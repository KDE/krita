/* Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>

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
#include "TestKrita.h"
#include <QTest>

#include <KritaVersionWrapper.h>
#include <Krita.h>
#include <Window.h>
#include <Document.h>

void TestKrita::initTestCase()
{
    Krita::instance();
}

void TestKrita::testKrita()
{
    Krita *krita = Krita::instance();
    QVERIFY2(krita, "Could not create krita instance.");
    QCOMPARE(krita->batchmode(), false);
    krita->setBatchmode(true);
    QCOMPARE(krita->batchmode(), true);

    QVERIFY(krita->filters().size() > 0);
    QVERIFY(krita->filter(krita->filters().first()) != 0);

    //QVERIFY(krita->generators().size() > 0);
    //QVERIFY(krita->generator(krita->generators().first()) != 0);

    QStringList profiles = krita->profiles("RGBA", "U8");
    QVERIFY(profiles.size() != 0);
    Document *doc = krita->createDocument(100, 100, "test", "RGBA", "U8", profiles.first());
    QVERIFY(doc);
    QCOMPARE(krita->documents().size(), 1);


}

void TestKrita::cleanupTestCase()
{
    if (m_win) {
        m_win->close();
    }
    QTest::qWait(1000);
}


QTEST_MAIN(TestKrita)

