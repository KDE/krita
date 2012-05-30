/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_iterator_benchmark.h"
#include <QApplication>

#include <qtest_kde.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>

#include "kis_random_accessor_ng.h"
#include "kis_random_sub_accessor.h"
#include <kis_iterator_ng.h>
#include <kis_repeat_iterators_pixel.h>

#include "kis_paint_device.h"

#define TEST_WIDTH 3000
#define TEST_HEIGHT 3000

void KisIteratorBenchmark::allCsApplicator(void (KisIteratorBenchmark::* funcPtr)(const KoColorSpace*cs))
{
    QList<const KoColorSpace*> colorsapces = KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile);
    int counter = 0;
    foreach(const KoColorSpace* cs, colorsapces) {

        dbgImage << "Testing with" << cs->id();

        counter++;

        (this->*funcPtr)(cs);

        if (counter > 10) return;
    }
}

void KisIteratorBenchmark::rectIter(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);

    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QTime t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisRectIteratorSP it = dev.createRectIteratorNG(0, 0, TEST_WIDTH, TEST_HEIGHT);
        do {
            //memcpy(it->rawData(), bytes, colorSpace->pixelSize());
        } while (it->nextPixel());

        qDebug() << "RectIterator run " << i  << "took" << t.elapsed();
        t.restart();
    }

    delete[] bytes;
}

void KisIteratorBenchmark::hLineIter(const KoColorSpace * colorSpace)
{
    KisPaintDevice dev(colorSpace);

    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QTime t;
    t.start();


    for (int i = 0; i < 3; i++) {
        KisHLineIteratorSP it = dev.createHLineIteratorNG(0, 0, TEST_WIDTH);
        for (int j = 0; j < TEST_HEIGHT; j++) {
            do {
                //memcpy(it->rawData(), bytes, colorSpace->pixelSize());
            } while (it->nextPixel());
            it->nextRow();
        }

        qDebug() << "HLineIterator run " << i  << "took" << t.elapsed();
        t.restart();
    }

    for (int i = 0; i < 3; i++) {
        KisHLineIteratorSP it = dev.createHLineIteratorNG(0, 0, TEST_WIDTH);
        for (int j = 0; j < TEST_HEIGHT; j++) {
            int pixels = 0;
            do {
                pixels = it->nConseqPixels();
                //memcpy(it->rawData(), bytes, colorSpace->pixelSize());
            } while (it->nextPixels(pixels));
            it->nextRow();
        }

        kDebug() << "HLineIterator with nConseqHPixels run " << i  << "took" << t.elapsed();
        t.restart();
    }

    KisHLineConstIteratorSP cit = dev.createHLineConstIteratorNG(0, 0, TEST_WIDTH);
    for (int i = 0; i < TEST_HEIGHT; i++) {
        do {} while (cit->nextPixel());
        cit->nextRow();
    }

    qDebug() << "const HLineIterator took" << t.elapsed();

    delete[] bytes;
}

void KisIteratorBenchmark::hLineIterNG(const KoColorSpace * colorSpace)
{
    KisPaintDevice dev(colorSpace);

    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QTime t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisHLineIteratorSP it = dev.createHLineIteratorNG(0, 0, TEST_WIDTH);
        for (int j = 0; j < TEST_HEIGHT; j++) {
            do {
                //memcpy(it->rawData(), bytes, colorSpace->pixelSize());
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
                //memcpy(it->rawData(), bytes, colorSpace->pixelSize());
            } while (it->nextPixels(pixels));
            it->nextRow();
        }

        kDebug() << "HLineIterator with nConseqHPixels run " << i  << "took" << t.elapsed();
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


void KisIteratorBenchmark::vLineIter(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);
    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QTime t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisVLineIteratorSP it = dev.createVLineIteratorNG(0, 0, TEST_HEIGHT);
        for (int j = 0; j < TEST_WIDTH; j++) {
            do {
                //memcpy(it->rawData(), bytes, colorSpace->pixelSize());

            } while (it->nextPixel());
            it->nextColumn();
        }

        qDebug() << "VLineIterator run " << i  << " took" << t.elapsed();
        t.restart();
    }

    KisVLineConstIteratorSP cit = dev.createVLineConstIteratorNG(0, 0, TEST_HEIGHT);
    for (int i = 0; i < TEST_WIDTH; i++) {
        do {} while (cit->nextPixel());
        cit->nextColumn();
    }
    qDebug() << "const VLineIterator took" << t.elapsed();

    delete[] bytes;

}

void KisIteratorBenchmark::vLineIterNG(const KoColorSpace * colorSpace)
{

    KisPaintDevice dev(colorSpace);
    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QTime t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisVLineIteratorSP it = dev.createVLineIteratorNG(0, 0, TEST_HEIGHT);
        for (int j = 0; j < TEST_WIDTH; j++) {
            do {
                //memcpy(it->rawData(), bytes, colorSpace->pixelSize());
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
    quint8 * bytes = colorSpace->allocPixelBuffer(1);
    memset(bytes, 128, colorSpace->pixelSize());

    QTime t;
    t.start();

    for (int i = 0; i < 3; i++) {
        KisRandomAccessorSP ac = dev.createRandomAccessorNG(0, 0);
        for (int y = 0; y < TEST_HEIGHT; ++y) {
            for (int x = 0; x < TEST_WIDTH; ++x) {
                ac->moveTo(x, y);
                //             memcpy(ac->rawData(), bytes, colorSpace->pixelSize());
            }
        }

        qDebug() << "RandomIterator run " << i  << " took" << t.elapsed();
        t.restart();
    }

    KisRandomConstAccessorSP cac = dev.createRandomConstAccessorNG(0, 0);
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

    hLineIter(cs);
    hLineIterNG(cs);
    vLineIter(cs);
    vLineIterNG(cs);
    rectIter(cs);
    randomAccessor(cs);
}

QTEST_KDEMAIN(KisIteratorBenchmark, NoGUI)
#include "kis_iterator_benchmark.moc"
