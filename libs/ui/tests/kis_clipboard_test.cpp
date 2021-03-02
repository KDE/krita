/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_clipboard_test.h"

#include <simpletest.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device.h"
#include "kis_clipboard.h"

#include <testutil.h>


void KisClipboardTest::testRoundTrip()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisPaintDeviceSP newDev;
    QPoint errorPoint;

    QRect fillRect(10,10,20,20);
    KoColor pixel(Qt::red, cs);
    dev->fill(fillRect.x(),fillRect.y(),
              fillRect.width(), fillRect.height(), pixel.data());

    QCOMPARE(dev->exactBounds(), fillRect);
    KisClipboard::instance()->setClip(dev, QPoint());
    newDev = KisClipboard::instance()->clip(QRect(), false);
    QCOMPARE(newDev->exactBounds().size(), fillRect.size());
    newDev->setX(dev->x());
    newDev->setY(dev->y());
    QVERIFY(TestUtil::comparePaintDevices(errorPoint, dev, newDev));

    QPoint offset(100,100);
    dev->setX(offset.x());
    dev->setY(offset.y());

    QCOMPARE(dev->exactBounds(), fillRect.translated(offset));
    KisClipboard::instance()->setClip(dev, QPoint());
    newDev = KisClipboard::instance()->clip(QRect(), true);
    QCOMPARE(newDev->exactBounds().size(), fillRect.translated(offset).size());
    newDev->setX(dev->x());
    newDev->setY(dev->y());
    QVERIFY(TestUtil::comparePaintDevices(errorPoint, dev, newDev));
}


SIMPLE_TEST_MAIN(KisClipboardTest)
