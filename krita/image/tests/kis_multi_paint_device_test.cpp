/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#include "kis_multi_paint_device_test.h"
#include "kis_multi_paint_device.h"

#include <qtest_kde.h>

#include <KoColorSpaceRegistry.h>


void KisMultiPaintDeviceTest::initTestCase()
{
    cs = KoColorSpaceRegistry::instance()->rgb8();

    red = new quint8[cs->pixelSize()];
    green = new quint8[cs->pixelSize()];
    blue = new quint8[cs->pixelSize()];

    cs->fromQColor(Qt::red, red);
    cs->fromQColor(Qt::green, green);
    cs->fromQColor(Qt::blue, blue);
}

void KisMultiPaintDeviceTest::cleanupTestCase()
{
    delete red;
    delete green;
    delete blue;
}

void KisMultiPaintDeviceTest::testContextCreation()
{
    KisMultiPaintDevice *dev = new KisMultiPaintDevice(cs);

    int id1 = dev->currentContext();
    dev->fill(0, 0, 512, 512, red);
    QImage thumb1a = dev->createThumbnail(50, 50);

    int id2 = dev->newContext();
    QImage thumb1b = dev->createThumbnail(50, 50);

    dev->switchContext(id2);
    QImage thumb2 = dev->createThumbnail(50, 50);

    QVERIFY(id1 != id2);
    QVERIFY(thumb1a == thumb1b);

    QVERIFY(thumb1b != thumb2);

    delete dev;
}

void KisMultiPaintDeviceTest::testContextSwitching()
{
    KisMultiPaintDevice *dev = new KisMultiPaintDevice(cs);

    int id1 = dev->currentContext();

    dev->fill(0, 0, 512, 512, red);
    QImage thumb1a = dev->createThumbnail(50, 50);

    int id2 = dev->newContext();
    dev->switchContext(id2);

    dev->fill(0, 256, 256, 512, green);
    QImage thumb2a = dev->createThumbnail(50, 50);

    dev->switchContext(id1);
    QImage thumb1b = dev->createThumbnail(50, 50);

    dev->switchContext(id2);
    QImage thumb2b = dev->createThumbnail(50, 50);

    QVERIFY(id1 != id2);
    QVERIFY(thumb1a != thumb2a);

    QVERIFY(thumb1a == thumb1b);
    QVERIFY(thumb2a == thumb2b);

    delete dev;
}

void KisMultiPaintDeviceTest::testDuplicateContextCreation()
{
    KisMultiPaintDevice *dev = new KisMultiPaintDevice(cs);

    int id1 = dev->currentContext();

    dev->fill(0, 0, 512, 512, red);
    QImage thumb1a = dev->createThumbnail(50, 50);

    int id2 = dev->newContext(id1);
    QImage thumb1b = dev->createThumbnail(50, 50);

    dev->fill(0, 256, 256, 512, green);
    QImage thumb1c = dev->createThumbnail(50, 50);

    dev->switchContext(id2);
    QImage thumb2a = dev->createThumbnail(50, 50);

    dev->fill(0, 256, 256, 512, blue);
    dev->switchContext(id1);
    QImage thumb1d = dev->createThumbnail(50, 50);

    QVERIFY(thumb1b == thumb1a);
    QVERIFY(thumb1c != thumb1b);
    QVERIFY(thumb2a != thumb1c);
    QVERIFY(thumb2a == thumb1a);
    QVERIFY(thumb1d == thumb1c);

    delete dev;
}

void KisMultiPaintDeviceTest::testContextDropping()
{
   KisMultiPaintDevice *dev = new KisMultiPaintDevice(cs);

    int id1 = dev->currentContext();
    int id2 = dev->newContext();
    int id3 = dev->newContext();
    int id4 = dev->newContext();

    dev->switchContext(id2);
    dev->dropContext(id2);

    QVERIFY(dev->currentContext() != id2);

    dev->dropContext(id1);
    dev->dropContext(id4);

    // This leave only context 3
    QCOMPARE(id3, dev->currentContext());

    dev->dropContext(id3);

    // Last remaining is not dropped
    QCOMPARE(id3, dev->currentContext());

    delete dev;
}

QTEST_KDEMAIN(KisMultiPaintDeviceTest, GUI)
#include "kis_multi_paint_device_test.moc"
