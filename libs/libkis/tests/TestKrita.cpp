/* SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestKrita.h"
#include <QTest>

#include <KritaVersionWrapper.h>
#include <Krita.h>
#include <Window.h>
#include <Document.h>

#include <sdk/tests/testui.h>

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


KISTEST_MAIN(TestKrita)

