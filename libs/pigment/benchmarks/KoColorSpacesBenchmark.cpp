/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoColorSpacesBenchmark.h"

#include <simpletest.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#define NB_PIXELS 1000000

void KoColorSpacesBenchmark::createRowsColumns()
{
    QTest::addColumn<QString>("modelID");
    QTest::addColumn<QString>("depthID");
    QList<const KoColorSpace*> colorSpaces = KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile);
    Q_FOREACH (const KoColorSpace* colorSpace, colorSpaces) {
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
            colorSpace->opacityU8(data_it);
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
            colorSpace->opacityF(data_it);
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
        colorSpace->setOpacity(data, OPACITY_OPAQUE_U8, NB_PIXELS);
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
        colorSpace->setOpacity(data, OPACITY_OPAQUE_F, NB_PIXELS);
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
            colorSpace->setOpacity(data_it, OPACITY_OPAQUE_U8, 1);
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
            colorSpace->setOpacity(data_it, OPACITY_OPAQUE_F, 1);
            data_it += pixelSize;
        }
    }
    END_BENCHMARK
}

SIMPLE_TEST_MAIN(KoColorSpacesBenchmark)
