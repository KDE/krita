/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_clipboard_test.h"

#include <QTest>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device.h"
#include "kis_clipboard.h"

#include "testutil.h"


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


QTEST_MAIN(KisClipboardTest)
