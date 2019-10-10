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

#include <QTest>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColor.h>

#include "kis_random_accessor_ng.h"
#include "kis_random_sub_accessor.h"
#include <kis_iterator_ng.h>

#include "kis_paint_device.h"
#include <kis_repeat_iterators_pixel.h>

#include "sdk/tests/kistest.h"

void KisIteratorTest::allCsApplicator(void (KisIteratorTest::* funcPtr)(const KoColorSpace*cs))
{
    QList<const KoColorSpace*> colorsapces = KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile);

    Q_FOREACH (const KoColorSpace* cs, colorsapces) {

        dbgKrita << "Testing with" << cs->id();

        if (cs->id() != "GRAYU16") // No point in testing extend for GRAYU16
            (this->*funcPtr)(cs);

    }
}

inline quint8* allocatePixels(const KoColorSpace *colorSpace, int numPixels)
{
    quint8 *bytes = new quint8[colorSpace->pixelSize() * 64 * 64 * 10];

    KoColor color(Qt::red, colorSpace);
    const int pixelSize = colorSpace->pixelSize();
    for(int i = 0; i < numPixels; i++) {
        memcpy(bytes + i * pixelSize, color.data(), pixelSize);
    }

    return bytes;
}

void KisIteratorTest::writeBytes(const KoColorSpace * colorSpace)
{


    KisPaintDevice dev(colorSpace);

    QCOMPARE(dev.extent(), QRect());

    // Check allocation on tile boundaries

    // Allocate memory for a 2 * 5 tiles grid
    QScopedArrayPointer<quint8> bytes(allocatePixels(colorSpace, 64 * 64 * 10));

    // Covers 5 x 2 tiles
    dev.writeBytes(bytes.data(), 0, 0, 5 * 64, 2 * 64);

    // Covers
    QCOMPARE(dev.extent(), QRect(0, 0, 64 * 5, 64 * 2));
    QCOMPARE(dev.exactBounds(), QRect(0, 0, 64 * 5, 64 * 2));

    dev.clear();
    QCOMPARE(dev.extent(), QRect());

    dev.clear();
    // Covers three by three tiles
    dev.writeBytes(bytes.data(), 10, 10, 130, 130);

    QCOMPARE(dev.extent(), QRect(0, 0, 64 * 3, 64 * 3));
    QCOMPARE(dev.exactBounds(), QRect(10, 10, 130, 130));

    dev.clear();
    // Covers 11 x 2 tiles
    dev.writeBytes(bytes.data(), -10, -10, 10 * 64, 64);

    QCOMPARE(dev.extent(), QRect(-64, -64, 64 * 11, 64 * 2));
    QCOMPARE(dev.exactBounds(), QRect(-10, -10, 640, 64));
}

void KisIteratorTest::fill(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);

    QCOMPARE(dev.extent(), QRect());

    QScopedArrayPointer<quint8> bytes(allocatePixels(colorSpace, 1));

    dev.fill(0, 0, 5, 5, bytes.data());
    QCOMPARE(dev.extent(), QRect(0, 0, 64, 64));
    QCOMPARE(dev.exactBounds(), QRect(0, 0, 5, 5));

    dev.clear();
    dev.fill(5, 5, 5, 5, bytes.data());
    QCOMPARE(dev.extent(), QRect(0, 0, 64, 64));
    QCOMPARE(dev.exactBounds(), QRect(5, 5, 5, 5));

    dev.clear();
    dev.fill(5, 5, 500, 500, bytes.data());
    QCOMPARE(dev.extent(), QRect(0, 0, 8 * 64, 8 * 64));
    QCOMPARE(dev.exactBounds(), QRect(5, 5, 500, 500));

    dev.clear();
    dev.fill(33, -10, 348, 1028, bytes.data());
    QCOMPARE(dev.extent(), QRect(0, -64, 6 * 64, 17 * 64));
    QCOMPARE(dev.exactBounds(), QRect(33, -10, 348, 1028));
}

void KisIteratorTest::hLineIter(const KoColorSpace * colorSpace)
{
    KisPaintDevice dev(colorSpace);

    QScopedArrayPointer<quint8> bytes(allocatePixels(colorSpace, 1));

    QCOMPARE(dev.extent(), QRect());

    KisHLineConstIteratorSP cit = dev.createHLineConstIteratorNG(0, 0, 128);
    do {} while (cit->nextPixel());
    QCOMPARE(dev.extent(), QRect());
    QCOMPARE(dev.exactBounds(), QRect(QPoint(0, 0), QPoint(-1, -1)));

    {

        dev.clear();
        KisHLineIteratorSP it = dev.createHLineIteratorNG(0, 0, 128);
        do {
            memcpy(it->rawData(), bytes.data(), colorSpace->pixelSize());
        } while (it->nextPixel());


        QCOMPARE(dev.extent(), QRect(0, 0, 128, 64));
        QCOMPARE(dev.exactBounds(), QRect(0, 0, 128, 1));
    }

    dev.clear();

    {
        KisHLineIteratorSP it = dev.createHLineIteratorNG(0, 1, 128);
        do {
            memcpy(it->rawData(), bytes.data(), colorSpace->pixelSize());
        } while (it->nextPixel());

        QCOMPARE(dev.extent(), QRect(0, 0, 128, 64));
        QCOMPARE(dev.exactBounds(), QRect(0, 1, 128, 1));
    }

    dev.clear();

    {
        KisHLineIteratorSP it = dev.createHLineIteratorNG(10, 10, 128);
        do {
            memcpy(it->rawData(), bytes.data(), colorSpace->pixelSize());
        } while (it->nextPixel());

        QCOMPARE(dev.extent(), QRect(0, 0, 192, 64));
        QCOMPARE(dev.exactBounds(), QRect(10, 10, 128, 1));
    }

    dev.clear();
    dev.setX(10);
    dev.setY(-15);

    {
        KisHLineIteratorSP it = dev.createHLineIteratorNG(10, 10, 128);
        do {
            memcpy(it->rawData(), bytes.data(), colorSpace->pixelSize());
        } while (it->nextPixel());

        QCOMPARE(dev.extent(), QRect(10, -15, 128, 64));
        QCOMPARE(dev.exactBounds(), QRect(10, 10, 128, 1));
    }
}

void KisIteratorTest::vLineIter(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);
    QScopedArrayPointer<quint8> bytes(allocatePixels(colorSpace, 1));

    QCOMPARE(dev.extent(), QRect());

    KisVLineConstIteratorSP cit = dev.createVLineConstIteratorNG(0, 0, 128);
    do {} while (cit->nextPixel());
    QCOMPARE(dev.extent(), QRect());
    QCOMPARE(dev.exactBounds(), QRect(QPoint(0, 0), QPoint(-1, -1)));

    {
        KisVLineIteratorSP it = dev.createVLineIteratorNG(0, 0, 128);
        do {
            memcpy(it->rawData(), bytes.data(), colorSpace->pixelSize());
        } while (it->nextPixel());
        QCOMPARE((QRect) dev.extent(), QRect(0, 0, 64, 128));
        QCOMPARE((QRect) dev.exactBounds(), QRect(0, 0, 1, 128));
    }

    dev.clear();

    {
        KisVLineIteratorSP it = dev.createVLineIteratorNG(10, 10, 128);
        do {
            memcpy(it->rawData(), bytes.data(), colorSpace->pixelSize());
        } while (it->nextPixel());

        QCOMPARE(dev.extent(), QRect(0, 0, 64, 192));
        QCOMPARE(dev.exactBounds(), QRect(10, 10, 1, 128));
    }
    
    dev.clear();
    dev.setX(10);
    dev.setY(-15);

    {
        KisVLineIteratorSP it = dev.createVLineIteratorNG(10, 10, 128);
        do {
            memcpy(it->rawData(), bytes.data(), colorSpace->pixelSize());
        } while (it->nextPixel());

        QCOMPARE(dev.extent(), QRect(10, -15, 64, 192));
        QCOMPARE(dev.exactBounds(), QRect(10, 10, 1, 128));
    }

}

void KisIteratorTest::randomAccessor(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);
    QScopedArrayPointer<quint8> bytes(allocatePixels(colorSpace, 1));

    QCOMPARE(dev.extent(), QRect());

    KisRandomConstAccessorSP acc = dev.createRandomConstAccessorNG(0, 0);
    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {
            acc->moveTo(x, y);
        }
    }
    QCOMPARE(dev.extent(), QRect());

    KisRandomAccessorSP ac = dev.createRandomAccessorNG(0, 0);
    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {
            ac->moveTo(x, y);
            memcpy(ac->rawData(), bytes.data(), colorSpace->pixelSize());
        }
    }
    QCOMPARE(dev.extent(), QRect(0, 0, 128, 128));
    QCOMPARE(dev.exactBounds(), QRect(0, 0, 128, 128));
    
    dev.clear();
    dev.setX(10);
    dev.setY(-15);

    {
        KisRandomAccessorSP ac = dev.createRandomAccessorNG(0, 0);
        for (int y = 0; y < 128; ++y) {
            for (int x = 0; x < 128; ++x) {
                ac->moveTo(x, y);
                memcpy(ac->rawData(), bytes.data(), colorSpace->pixelSize());
            }
        }
        QCOMPARE(dev.extent(), QRect(-54, -15, 192, 192));
        QCOMPARE(dev.exactBounds(), QRect(0, 0, 128, 128));
    }
}

void KisIteratorTest::repeatHLineIter(const KoColorSpace* cs)
{
    KoColor color(Qt::green, cs);
    
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(5, 5, 10, 10, color.data());

    KisRepeatHLineConstIteratorSP iter = dev->createRepeatHLineConstIterator(0, 0, 20, QRect(5, 5, 10, 10));
    for(int i = 0; i < 20; i++) {
        do {
            QVERIFY(!memcmp(color.data(), iter->oldRawData(), cs->pixelSize()));
        } while (iter->nextPixel());
        iter->nextRow();
    }
}

void KisIteratorTest::repeatVLineIter(const KoColorSpace* cs)
{
    KoColor color(Qt::green, cs);
    
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(5, 5, 10, 10, color.data());

    KisRepeatVLineConstIteratorSP iter = dev->createRepeatVLineConstIterator(0, 0, 20, QRect(5, 5, 10, 10));
    for(int i = 0; i < 20; i++) {
        do {
            QVERIFY(!memcmp(color.data(), iter->oldRawData(), cs->pixelSize()));
        } while (iter->nextPixel());
        iter->nextColumn();
    }
}

void KisIteratorTest::writeBytes()
{
    allCsApplicator(&KisIteratorTest::writeBytes);
}

void KisIteratorTest::fill()
{
    allCsApplicator(&KisIteratorTest::fill);
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

void KisIteratorTest::repeatHLineIter()
{
    allCsApplicator(&KisIteratorTest::repeatHLineIter);
}

void KisIteratorTest::repeatVLineIter()
{
    allCsApplicator(&KisIteratorTest::repeatVLineIter);
}

#define NUM_CYCLES 10000
#define NUM_THREADS 10

class DataReaderThread : public QRunnable {
public:
    DataReaderThread(KisPaintDeviceSP device, const QRect &rect)
        : m_device(device),
          m_rect(rect)
    {}

    void run() override {
        for(int i = 0; i < NUM_CYCLES; i++) {
            KisRandomAccessorSP iter =
                m_device->createRandomAccessorNG(m_rect.x(), m_rect.y());

            qint32 rowsRemaining = m_rect.height();
            qint32 y = m_rect.y();

            while (rowsRemaining > 0) {
                qint32 columnsRemaining = m_rect.width();
                qint32 x = m_rect.x();

                qint32 numContiguousRows = iter->numContiguousRows(y);
                qint32 rows = qMin(numContiguousRows, rowsRemaining);

                while (columnsRemaining > 0) {
                    qint32 numContiguousColumns = iter->numContiguousColumns(x);
                    qint32 columns = qMin(numContiguousColumns, columnsRemaining);

                    qint32 rowStride = iter->rowStride(x, y);
                    iter->moveTo(x, y);

                    // dbgKrita << "BitBlt:" << ppVar(x) << ppVar(y)
                    //          << ppVar(columns) << ppVar(rows)
                    //          << ppVar(rowStride);

                    doBitBlt(iter->rawData(), rowStride, m_device->pixelSize(),
                             rows, columns);

                    x += columns;
                    columnsRemaining -= columns;
                }
                y += rows;
                rowsRemaining -= rows;
            }
        }
    }

private:

    void doBitBltConst(const quint8* data, qint32 rowStride, qint32 pixelSize,
                       qint32 rows, qint32 columns) {
        for(int i = 0; i < rows; i++) {
            Q_ASSERT(columns * pixelSize < 256);
            quint8 tempData[256];
            memcpy(tempData, data, columns * pixelSize);

            data += rowStride;
        }
    }

    void doBitBlt(quint8* data, qint32 rowStride, qint32 pixelSize,
                  qint32 rows, qint32 columns) {
        for(int i = 0; i < rows; i++) {
            // Let's write something here...
            memset(data, 0x13, columns * pixelSize);

            data += rowStride;
        }
    }

private:
    KisPaintDeviceSP m_device;
    QRect m_rect;
};

class NastyThread : public QRunnable {
public:
    NastyThread(KisPaintDeviceSP device)
        : m_device(device)
    {}

    void run() override {
        for(int i = 0; i < NUM_CYCLES; i++) {
            m_device->setX(-0x400 + (qrand() & 0x7FF));
            m_device->setY(-0x400 + (qrand() & 0x7FF));
            QTest::qSleep(10);
        }
    }

private:
    KisPaintDeviceSP m_device;
};

void KisIteratorTest::stressTest()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();

    QRect imageRect(0,0,2000,2000);

    KisPaintDeviceSP device = new KisPaintDevice(colorSpace);
    device->fill(imageRect, KoColor(Qt::red, colorSpace));


    QThreadPool threadPool;
    threadPool.setMaxThreadCount(NUM_THREADS);

    for(int i = 0; i< NUM_THREADS; i++) {
        QRect rc = QRect(double(i) / NUM_THREADS * 2000, 0, 2000 / NUM_THREADS, 2000);
//        dbgKrita << rc;

        DataReaderThread *reader = new DataReaderThread(device, rc);
        threadPool.start(reader);

        if(!(i & 0x1)) {
            NastyThread *nasty = new NastyThread(device);
            threadPool.start(nasty);
        }
    }

    threadPool.waitForDone();
}

KISTEST_MAIN(KisIteratorTest)
