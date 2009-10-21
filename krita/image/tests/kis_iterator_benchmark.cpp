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

#include "kis_iterators_pixel.h"
#include "kis_random_accessor.h"
#include "kis_random_sub_accessor.h"

#include "kis_paint_device.h"

#define TEST_WIDTH 3000
#define TEST_HEIGHT 3000

void KisIteratorBenchmark::allCsApplicator(void (KisIteratorBenchmark::* funcPtr)(const KoColorSpace*cs))
{
    QList<QString> csIds = KoColorSpaceRegistry::instance()->keys();
    int counter = 0;
    foreach(QString csId, csIds) {

        dbgImage << "Testing with" << csId;

        counter++;

        QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor(csId);
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(csId, 0);
        if (cs)(this->*funcPtr)(cs);

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
        KisRectIteratorPixel it = dev.createRectIterator(0, 0, TEST_WIDTH, TEST_HEIGHT);
        while (!it.isDone()) {
            //memcpy(it.rawData(), bytes, colorSpace->pixelSize());
            ++it;
        }

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
        KisHLineIteratorPixel it = dev.createHLineIterator(0, 0, TEST_WIDTH);
        for (int j = 0; j < TEST_HEIGHT; j++) {
            while (!it.isDone()) {
                //memcpy(it.rawData(), bytes, colorSpace->pixelSize());
                ++it;
            }
            it.nextRow();
        }

        qDebug() << "HLineIterator run " << i  << "took" << t.elapsed();
        t.restart();
    }

    for (int i = 0; i < 3; i++) {
        KisHLineIteratorPixel it = dev.createHLineIterator(0, 0, TEST_WIDTH);
        for (int j = 0; j < TEST_HEIGHT; j++) {
            while (!it.isDone()) {
                int pixels = it.nConseqHPixels();
                //memcpy(it.rawData(), bytes, colorSpace->pixelSize());
                it += pixels;
            }
            it.nextRow();
        }

        kDebug() << "HLineIterator with nConseqHPixels run " << i  << "took" << t.elapsed();
        t.restart();
    }

    KisHLineConstIteratorPixel cit = dev.createHLineConstIterator(0, 0, TEST_WIDTH);
    for (int i = 0; i < TEST_HEIGHT; i++) {
        while (!cit.isDone())
            ++cit;
        cit.nextRow();
    }

    qDebug() << "const HLineIterator took" << t.elapsed();

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
        KisVLineIteratorPixel it = dev.createVLineIterator(0, 0, TEST_HEIGHT);
        for (int j = 0; j < TEST_WIDTH; j++) {
            while (!it.isDone()) {
                //memcpy(it.rawData(), bytes, colorSpace->pixelSize());
                ++it;
            }
            it.nextCol();
        }

        qDebug() << "VLineIterator run " << i  << " took" << t.elapsed();
        t.restart();
    }

    KisVLineConstIteratorPixel cit = dev.createVLineConstIterator(0, 0, TEST_HEIGHT);
    for (int i = 0; i < TEST_WIDTH; i++) {
        while (!cit.isDone()) {
            ++cit;
        }
        cit.nextCol();
    }
    qDebug() << "const VLineIterator took" << t.elapsed();

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
        KisRandomAccessorPixel ac = dev.createRandomAccessor(0, 0);
        for (int y = 0; y < TEST_HEIGHT; ++y) {
            for (int x = 0; x < TEST_WIDTH; ++x) {
                ac.moveTo(x, y);
                //             memcpy(ac.rawData(), bytes, colorSpace->pixelSize());
            }
        }

        qDebug() << "RandomIterator run " << i  << " took" << t.elapsed();
        t.restart();
    }

    KisRandomConstAccessorPixel cac = dev.createRandomConstAccessor(0, 0);
    for (int y = 0; y < TEST_HEIGHT; ++y) {
        for (int x = 0; x < TEST_WIDTH; ++x) {
            cac.moveTo(x, y);
        }
    }

    qDebug() << "const RandomIterator took" << t.elapsed();

    delete[] bytes;
}


void KisIteratorBenchmark::runBenchmark()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    hLineIter(cs);
    vLineIter(cs);
    rectIter(cs);
    randomAccessor(cs);
}

QTEST_KDEMAIN(KisIteratorBenchmark, NoGUI)
#include "kis_iterator_benchmark.moc"
