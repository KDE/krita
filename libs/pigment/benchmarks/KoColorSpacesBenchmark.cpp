/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
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

#include "KoColorSpacesBenchmark.h"

#include <qtest_kde.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#define NB_PIXELS 1000000

void KoColorSpacesBenchmark::createRowsColumns()
{
    QTest::addColumn<QString>("modelID");
    QTest::addColumn<QString>("depthID");
    QList<const KoColorSpace*> colorSpaces = KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile);
    foreach(const KoColorSpace* colorSpace, colorSpaces) {
        QTest::newRow(colorSpace->name().toLatin1().data()) << colorSpace->colorModelId().id() << colorSpace->colorDepthId().id();
    }
}

#define START_BENCHMARK \
    QFETCH(QString, modelID); \
    QFETCH(QString, depthID); \
    \
    const KoColorSpace* colorSpace = KoColorSpaceRegistry::instance()->colorSpace(modelID, depthID, 0); \
    int pixelSize = colorSpace->pixelSize(); \
    quint8* data = new quint8[NB_PIXELS * pixelSize]; \
    memset(data, 0, NB_PIXELS * pixelSize);

#define END_BENCHMARK \
    delete[] data;

void KoColorSpacesBenchmark::benchmarkAlpha_data()
{
    createRowsColumns();
}

void KoColorSpacesBenchmark::benchmarkAlpha()
{
    START_BENCHMARK
    QBENCHMARK {
        quint8* data_it = data;
        for (int i = 0; i < NB_PIXELS; ++i) {
            colorSpace->alpha(data_it);
            data_it += pixelSize;
        }
    }
    END_BENCHMARK
}

void KoColorSpacesBenchmark::benchmarkAlpha2_data()
{
    createRowsColumns();
}

void KoColorSpacesBenchmark::benchmarkAlpha2()
{
    START_BENCHMARK
    QBENCHMARK {
        quint8* data_it = data;
        for (int i = 0; i < NB_PIXELS; ++i) {
            colorSpace->alpha2(data_it);
            data_it += pixelSize;
        }
    }
    END_BENCHMARK
}

void KoColorSpacesBenchmark::benchmarkSetAlpha_data()
{
    createRowsColumns();
}

void KoColorSpacesBenchmark::benchmarkSetAlpha()
{
    START_BENCHMARK
    QBENCHMARK {
        colorSpace->setAlpha(data, OPACITY_OPAQUE, NB_PIXELS);
    }
    END_BENCHMARK
}

void KoColorSpacesBenchmark::benchmarkSetAlpha2_data()
{
    createRowsColumns();
}

void KoColorSpacesBenchmark::benchmarkSetAlpha2()
{
    START_BENCHMARK
    QBENCHMARK {
        colorSpace->setAlpha2(data, OPACITY_OPAQUE2, NB_PIXELS);
    }
    END_BENCHMARK
}

void KoColorSpacesBenchmark::benchmarkSetAlphaIndividualCall_data()
{
    createRowsColumns();
}

void KoColorSpacesBenchmark::benchmarkSetAlphaIndividualCall()
{
    START_BENCHMARK
    QBENCHMARK {
        quint8* data_it = data;
        for (int i = 0; i < NB_PIXELS; ++i) {
            colorSpace->setAlpha(data_it, OPACITY_OPAQUE, 1);
            data_it += pixelSize;
        }
    }
    END_BENCHMARK
}

void KoColorSpacesBenchmark::benchmarkSetAlpha2IndividualCall_data()
{
    createRowsColumns();
}

void KoColorSpacesBenchmark::benchmarkSetAlpha2IndividualCall()
{
    START_BENCHMARK
    QBENCHMARK {
        quint8* data_it = data;
        for (int i = 0; i < NB_PIXELS; ++i) {
            colorSpace->setAlpha2(data_it, OPACITY_OPAQUE2, 1);
            data_it += pixelSize;
        }
    }
    END_BENCHMARK
}

QTEST_KDEMAIN(KoColorSpacesBenchmark, GUI)

#include "KoColorSpacesBenchmark.moc"
