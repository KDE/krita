/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_iterator_benchmark.h"
#include <QApplication>

#include <simpletest.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

#include "kis_random_accessor_ng.h"
#include "kis_random_sub_accessor.h"
#include <kis_iterator_ng.h>
#include <kis_repeat_iterators_pixel.h>

#include "kis_paint_device.h"

#define TEST_WIDTH 3000
#define TEST_HEIGHT 3000


template <bool useXY>
void KisIteratorBenchmark::sequentialIter(const KoColorSpace * colorSpace)
{

    KisPaintDeviceSP dev = new KisPaintDevice(colorSpace);

    quint8 * bytes = new quint8[colorSpace->pixelSize() * 64*64];
    memset(bytes, 128, 64 * 64 * colorSpace->pixelSize());

    QElapsedTimer t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisSequentialIterator it(dev, QRect(0, 0, TEST_WIDTH, TEST_HEIGHT));
        int sum = 0;
        while (it.nextPixel()) {
            if (useXY) {
                sum = it.x() + it.y();
            }

            memcpy(it.rawData(), bytes, colorSpace->pixelSize());
        }

        qDebug() << ppVar(useXY) << "SequentialIterator run " << i  << "took" << t.elapsed();
        Q_UNUSED(sum);
        t.restart();
    }

    t.restart();

    for (int i = 0; i < 3; i++) {
        KisSequentialConstIterator it(dev, QRect(0, 0, TEST_WIDTH, TEST_HEIGHT));
        int sum = 0;
        while (it.nextPixel()) {
            if (useXY) {
                sum = it.x() + it.y();
            }
            //memcpy(it.rawData(), bytes, colorSpace->pixelSize());
        }

        qDebug() << ppVar(useXY) << "SequentialConstIterator run " << i  << "took" << t.elapsed();
        Q_UNUSED(sum);
        t.restart();
    }



    delete[] bytes;
}

void KisIteratorBenchmark::hLineIterNG(const KoColorSpace * colorSpace)
{
    KisPaintDevice dev(colorSpace);

    quint8 * bytes = new quint8[colorSpace->pixelSize() * 128];
    memset(bytes, 128, 128 * colorSpace->pixelSize());

    QElapsedTimer t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisHLineIteratorSP it = dev.createHLineIteratorNG(0, 0, TEST_WIDTH);
        for (int j = 0; j < TEST_HEIGHT; j++) {
            do {
                memcpy(it->rawData(), bytes, colorSpace->pixelSize());
            } while (it->nextPixel());
            it->nextRow();
        }

        qDebug() << "HLineIteratorNG run " << i  << "took" << t.elapsed();
        t.restart();
    }

    for (int i = 0; i < 3; i++) {
        KisHLineIteratorSP it = dev.createHLineIteratorNG(0, 0, TEST_WIDTH);
        for (int j = 0; j < TEST_HEIGHT; j++) {
            int pixels;
            do {
                pixels = it->nConseqPixels();
                memcpy(it->rawData(), bytes, pixels * colorSpace->pixelSize());
            } while (it->nextPixels(pixels));
            it->nextRow();
        }

        qDebug() << "HLineIteratorNG with nConseqHPixels run " << i  << "took" << t.elapsed();
        t.restart();
    }

    KisHLineConstIteratorSP cit = dev.createHLineConstIteratorNG(0, 0, TEST_WIDTH);
    for (int i = 0; i < TEST_HEIGHT; i++) {
        do {
            //do stuff
        } while (cit->nextPixel());
        cit->nextRow();
    }

    qDebug() << "const HLineIteratorNG took" << t.elapsed();

    delete[] bytes;
}

void KisIteratorBenchmark::vLineIterNG(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);
    quint8 * bytes = new quint8[colorSpace->pixelSize()];
    memset(bytes, 128, colorSpace->pixelSize());

    QElapsedTimer t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisVLineIteratorSP it = dev.createVLineIteratorNG(0, 0, TEST_HEIGHT);
        for (int j = 0; j < TEST_WIDTH; j++) {
            do {
                memcpy(it->rawData(), bytes, colorSpace->pixelSize());
            } while(it->nextPixel());
            it->nextColumn();
        }

        qDebug() << "VLineIteratorNG run " << i  << " took" << t.elapsed();
        t.restart();
    }

    KisVLineConstIteratorSP cit = dev.createVLineConstIteratorNG(0, 0, TEST_HEIGHT);
    for (int i = 0; i < TEST_WIDTH; i++) {
        do {} while(cit->nextPixel());
        cit->nextColumn();
    }
    qDebug() << "const VLineIteratorNG took" << t.elapsed();

    delete[] bytes;

}

void KisIteratorBenchmark::randomAccessor(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);
    quint8 * bytes = new quint8[colorSpace->pixelSize() * 128];
    memset(bytes, 128, 128 * colorSpace->pixelSize());

    QElapsedTimer t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisRandomAccessorSP ac = dev.createRandomAccessorNG();
        for (int y = 0; y < TEST_HEIGHT; ++y) {
            for (int x = 0; x < TEST_WIDTH; ++x) {
                ac->moveTo(x, y);
                memcpy(ac->rawData(), bytes, colorSpace->pixelSize());
            }
        }

        qDebug() << "RandomIterator run " << i  << " took" << t.elapsed();
        t.restart();
    }

    for (int i = 0; i < 3; i++) {
        KisRandomAccessorSP ac = dev.createRandomAccessorNG();
        for (int y = 0; y < TEST_HEIGHT; ) {
            int numContiguousRows = qMin(ac->numContiguousRows(y), TEST_HEIGHT - y);

            for (int x = 0; x < TEST_WIDTH; ) {
                int numContiguousColumns = qMin(ac->numContiguousColumns(x), TEST_WIDTH - x);

                ac->moveTo(x, y);
                int rowStride = ac->rowStride(x, y);
                quint8 *data = ac->rawData();


                for (int i = 0; i < numContiguousRows; i++) {
                    memcpy(data, bytes, numContiguousColumns * colorSpace->pixelSize());
                    data += rowStride;
                }

                x += numContiguousColumns;
            }
            y += numContiguousRows;
        }

        qDebug() << "RandomIterator run (with strides)" << i  << " took" << t.elapsed();
        t.restart();
    }

    KisRandomConstAccessorSP cac = dev.createRandomConstAccessorNG();
    for (int y = 0; y < TEST_HEIGHT; ++y) {
        for (int x = 0; x < TEST_WIDTH; ++x) {
            cac->moveTo(x, y);
        }
    }

    qDebug() << "const RandomIterator took" << t.elapsed();

    delete[] bytes;
}


void KisIteratorBenchmark::runBenchmark()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    hLineIterNG(cs);
    vLineIterNG(cs);
    sequentialIter<false>(cs);
    sequentialIter<true>(cs);
    randomAccessor(cs);
}

SIMPLE_TEST_MAIN(KisIteratorBenchmark)
