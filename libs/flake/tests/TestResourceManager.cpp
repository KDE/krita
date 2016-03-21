/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

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

#include "TestResourceManager.h"

#include "KoCanvasResourceManager.h"
#include "KoResourceManager_p.h"
#include "KoPathShape.h"
#include "KoUnit.h"
#include <QSignalSpy>
#include <QTest>

void TestResourceManager::koShapeResource()
{
    KoPathShape * shape = new KoPathShape();
    int key = 9001;

    KoCanvasResourceManager rp( 0 );
    rp.setResource( key, shape );
    QVERIFY( shape == rp.koShapeResource( key ) );
}

void TestResourceManager::testUnitChanged()
{
    KoCanvasResourceManager rm(0);
    QSignalSpy spy(&rm, SIGNAL(canvasResourceChanged(int, const QVariant &)));

    KoUnit a;
    rm.setResource(KoCanvasResourceManager::Unit, a);
    QCOMPARE(spy.count(), 1);

    KoUnit b(KoUnit::Millimeter);
    rm.setResource(KoCanvasResourceManager::Unit, b);
    QCOMPARE(spy.count(), 2);
}

#include "kis_global.h"

struct DerivedResource : public KoDerivedResourceConverter
{
    DerivedResource(int key, int sourceKey) : KoDerivedResourceConverter(key, sourceKey) {}

    QVariant fromSource(const QVariant &value) {
        return value.toInt() + 10;
    }

    QVariant toSource(const QVariant &value, const QVariant &sourceValue) {
        Q_UNUSED(sourceValue);
        return value.toInt() - 10;
    }
};

void TestResourceManager::testConverters()
{
    KoResourceManager m;

    const int key1 = 1;
    const int key2 = 2;
    const int derivedKey = 3;

    m.setResource(key1, 1);
    m.setResource(key2, 2);

    QCOMPARE(m.resource(key1).toInt(), 1);
    QCOMPARE(m.resource(key2).toInt(), 2);
    QVERIFY(!m.hasResource(derivedKey));

    m.addDerivedResourceConverter(toQShared(new DerivedResource(derivedKey, key2)));

    QCOMPARE(m.resource(derivedKey).toInt(), 12);
    QVERIFY(m.hasResource(derivedKey));

    m.setResource(derivedKey, 15);

    QCOMPARE(m.resource(key2).toInt(), 5);
    QCOMPARE(m.resource(derivedKey).toInt(), 15);

    QVERIFY(m.hasResource(derivedKey));
    m.clearResource(derivedKey);
    QVERIFY(m.hasResource(derivedKey));
    m.clearResource(key2);
    QVERIFY(!m.hasResource(derivedKey));
}

void TestResourceManager::testDerivedChanged()
{
    // const int key1 = 1;
    const int key2 = 2;
    const int derivedKey = 3;
    const int otherDerivedKey = 4;

    KoCanvasResourceManager m(0);
    m.addDerivedResourceConverter(toQShared(new DerivedResource(derivedKey, key2)));
    m.addDerivedResourceConverter(toQShared(new DerivedResource(otherDerivedKey, key2)));

    m.setResource(derivedKey, 15);

    QCOMPARE(m.resource(key2).toInt(), 5);
    QCOMPARE(m.resource(derivedKey).toInt(), 15);

    QSignalSpy spy(&m, SIGNAL(canvasResourceChanged(int, const QVariant &)));

    m.setResource(derivedKey, 16);

    QCOMPARE(spy.count(), 1);
    QList<QVariant> args;

    args = spy[0];
    QCOMPARE(args[0].toInt(), derivedKey);
    QCOMPARE(args[1].toInt(), 16);

    m.setResource(key2, 7);
    QCOMPARE(spy.count(), 4);

    args = spy[1];
    QCOMPARE(args[0].toInt(), key2);
    QCOMPARE(args[1].toInt(), 7);

    args = spy[2];
    QCOMPARE(args[0].toInt(), otherDerivedKey);
    QCOMPARE(args[1].toInt(), 17);

    args = spy[3];
    QCOMPARE(args[0].toInt(), derivedKey);
    QCOMPARE(args[1].toInt(), 17);
}

QTEST_MAIN(TestResourceManager)
