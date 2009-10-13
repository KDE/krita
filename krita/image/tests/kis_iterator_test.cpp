/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_iterator_test.h"
#include <QApplication>

#include <qtest_kde.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>

#include "kis_iterators_pixel.h"
#include "kis_random_accessor.h"
#include "kis_random_sub_accessor.h"


#include "kis_paint_device.h"

void KisIteratorTest::allCsApplicator(void (KisIteratorTest::* funcPtr)(const KoColorSpace*cs))
{
    QList<QString> csIds = KoColorSpaceRegistry::instance()->keys();
    int counter = 0;
    foreach(QString csId, csIds) {

        qDebug() << "Testing with" << csId;

        QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor(csId);
        if (profiles.size() > 0) {
            const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(csId, profiles[0]);
            if (cs)(this->*funcPtr)(cs);
        }

    }
}

void KisIteratorTest::writeBytes(const KoColorSpace * colorSpace)
{


    KisPaintDevice dev(colorSpace);

    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    // Check allocation on tile boundaries

    // Allocate memory for a 2 * 5 tiles grid
    quint8* bytes = colorSpace->allocPixelBuffer(64 * 64 * 10);
    memset(bytes, 128, 64 * 64 * 10 * colorSpace->pixelSize());

    // Covers 5 x 2 tiles
    dev.writeBytes(bytes, 0, 0, 5 * 64, 2 * 64);

    // Covers
    QVERIFY(dev.extent() == QRect(0, 0, 64 * 5, 64 * 2));
    QVERIFY(dev.exactBounds() == QRect(0, 0, 64 * 5, 64 * 2));

    dev.clear();
    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    dev.clear();
    // Covers three by three tiles
    dev.writeBytes(bytes, 10, 10, 130, 130);

    QVERIFY(dev.extent() == QRect(0, 0, 64 * 3, 64 * 3));
    QVERIFY(dev.exactBounds() == QRect(10, 10, 130, 130));

    dev.clear();
    // Covers 11 x 2 tiles
    dev.writeBytes(bytes, -10, -10, 10 * 64, 64);

    QVERIFY(dev.extent() == QRect(-64, -64, 64 * 11, 64 * 2));
    QVERIFY(dev.exactBounds() == QRect(-10, -10, 640, 64));

    delete[] bytes;
}

void KisIteratorTest::fill(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);

    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    dev.fill(0, 0, 5, 5, bytes);
    QVERIFY(dev.extent() == QRect(0, 0, 64, 64));
    QVERIFY(dev.exactBounds() == QRect(0, 0, 5, 5));

    dev.clear();
    dev.fill(5, 5, 5, 5, bytes);
    QVERIFY(dev.extent() == QRect(0, 0, 64, 64));
    QVERIFY(dev.exactBounds() == QRect(5, 5, 5, 5));

    dev.clear();
    dev.fill(5, 5, 500, 500, bytes);
    QVERIFY(dev.extent() == QRect(0, 0, 8 * 64, 8 * 64));
    QVERIFY(dev.exactBounds() == QRect(5, 5, 500, 500));

    dev.clear();
    dev.fill(33, -10, 348, 1028, bytes);
    QCOMPARE(dev.extent(), QRect(0, -64, 6 * 64, 17 * 64));
    QCOMPARE(dev.exactBounds(), QRect(33, -10, 348, 1028));

    delete[] bytes;
}

void KisIteratorTest::rectIter(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);

    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    {
        // Const does not extend the extent

        KisRectConstIteratorPixel cit = dev.createRectConstIterator(0, 0, 128, 128);
        while (!cit.isDone()) ++cit;
        QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));
        QVERIFY(dev.exactBounds() == QRect(qint32_MAX, qint32_MAX, 0, 0));

        // Non-const does

        KisRectIteratorPixel it = dev.createRectIterator(0, 0, 128, 128);
        while (!it.isDone()) {
            memcpy(it.rawData(), bytes, colorSpace->pixelSize());
            ++it;
        }
        QVERIFY(dev.extent() == QRect(0, 0, 128, 128));
        QVERIFY(dev.exactBounds() == QRect(0, 0, 128, 128));
    }

    dev.clear();

    {
        KisRectIteratorPixel it = dev.createRectIterator(10, 10, 128, 128);
        while (!it.isDone()) {
            memcpy(it.rawData(), bytes, colorSpace->pixelSize());
            ++it;
        }
        QVERIFY(dev.extent() == QRect(0, 0, 3 * 64, 3 * 64));
        QVERIFY(dev.exactBounds() == QRect(10, 10, 128, 128));
    }

    delete[] bytes;
}

void KisIteratorTest::hLineIter(const KoColorSpace * colorSpace)
{
    KisPaintDevice dev(colorSpace);

    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    KisHLineConstIteratorPixel cit = dev.createHLineConstIterator(0, 0, 128);
    while (!cit.isDone()) ++cit;
    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));
    QVERIFY(dev.exactBounds() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    {

        dev.clear();
        KisHLineIteratorPixel it = dev.createHLineIterator(0, 0, 128);
        while (!it.isDone()) {
            memcpy(it.rawData(), bytes, colorSpace->pixelSize());
            ++it;
        }


        QCOMPARE(dev.extent(), QRect(0, 0, 128, 64));
        QCOMPARE(dev.exactBounds(), QRect(0, 0, 128, 1));
    }

    dev.clear();

    {
        KisHLineIteratorPixel it = dev.createHLineIterator(0, 1, 128);
        while (!it.isDone()) {
            memcpy(it.rawData(), bytes, colorSpace->pixelSize());
            ++it;
        }

        QVERIFY(dev.extent() == QRect(0, 0, 128, 64));
        QVERIFY(dev.exactBounds() == QRect(0, 1, 128, 1));
    }

    dev.clear();

    {
        KisHLineIteratorPixel it = dev.createHLineIterator(10, 10, 128);
        while (!it.isDone()) {
            memcpy(it.rawData(), bytes, colorSpace->pixelSize());
            ++it;
        }

        QVERIFY(dev.extent() == QRect(0, 0, 192, 64));
        QVERIFY(dev.exactBounds() == QRect(10, 10, 128, 1));
    }

    delete[] bytes;
}

void KisIteratorTest::vLineIter(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);
    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    KisVLineConstIteratorPixel cit = dev.createVLineConstIterator(0, 0, 128);
    while (!cit.isDone()) ++cit;
    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));
    QVERIFY(dev.exactBounds() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    {
        KisVLineIteratorPixel it = dev.createVLineIterator(0, 0, 128);
        while (!it.isDone()) {
            memcpy(it.rawData(), bytes, colorSpace->pixelSize());
            ++it;
        }
        QCOMPARE((QRect) dev.extent(), QRect(0, 0, 64, 128));
        QCOMPARE((QRect) dev.exactBounds(), QRect(0, 0, 1, 128));
    }

    dev.clear();

    {
        KisVLineIteratorPixel it = dev.createVLineIterator(10, 10, 128);
        while (!it.isDone()) {
            memcpy(it.rawData(), bytes, colorSpace->pixelSize());
            ++it;
        }

        QCOMPARE(dev.extent(), QRect(0, 0, 64, 192));
        QCOMPARE(dev.exactBounds(), QRect(10, 10, 1, 128));
    }

    delete[] bytes;

}

void KisIteratorTest::randomAccessor(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);
    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    KisRandomConstAccessorPixel acc = dev.createRandomConstAccessor(0, 0);
    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {
            acc.moveTo(x, y);
        }
    }
    QVERIFY(dev.extent() == QRect(qint32_MAX, qint32_MAX, 0, 0));

    KisRandomAccessorPixel ac = dev.createRandomAccessor(0, 0);
    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {
            ac.moveTo(x, y);
            memcpy(ac.rawData(), bytes, colorSpace->pixelSize());
        }
    }
    QVERIFY(dev.extent() == QRect(0, 0, 128, 128));
    QVERIFY(dev.exactBounds() == QRect(0, 0, 128, 128));

    delete[] bytes;
}


void KisIteratorTest::writeBytes()
{
    allCsApplicator(&KisIteratorTest::writeBytes);
}

void KisIteratorTest::fill()
{
    allCsApplicator(&KisIteratorTest::fill);
}

void KisIteratorTest::rectIter()
{
    allCsApplicator(&KisIteratorTest::rectIter);
}

void KisIteratorTest::hLineIter()
{
    allCsApplicator(&KisIteratorTest::hLineIter);
}

void KisIteratorTest::vLineIter()
{
    allCsApplicator(&KisIteratorTest::vLineIter);
}

void KisIteratorTest::randomAccessor()
{
    allCsApplicator(&KisIteratorTest::randomAccessor);
}

QTEST_KDEMAIN(KisIteratorTest, NoGUI)
#include "kis_iterator_test.moc"
