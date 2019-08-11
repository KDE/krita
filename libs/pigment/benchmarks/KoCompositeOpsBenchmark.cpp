/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoCompositeOpsBenchmark.h"

#include "../compositeops/KoCompositeOpAlphaDarken.h"
#include "../compositeops/KoCompositeOpOver.h"
#include <KoOptimizedCompositeOpFactory.h>

#include <KoColorSpaceTraits.h>
#include <KoColorSpaceRegistry.h>

#include <QTest>

const int TILE_WIDTH = 64;
const int TILE_HEIGHT = 64;

const int IMG_WIDTH = 2048;
const int IMG_HEIGHT = 2048;

const quint8 OPACITY_HALF = 128;

const int TILES_IN_WIDTH = IMG_WIDTH / TILE_WIDTH;
const int TILES_IN_HEIGHT = IMG_HEIGHT / TILE_HEIGHT;


#define COMPOSITE_BENCHMARK \
        for (int y = 0; y < TILES_IN_HEIGHT; y++){                                              \
            for (int x = 0; x < TILES_IN_WIDTH; x++) {                                           \
                const int rowStride = IMG_WIDTH * KoBgrU8Traits::pixelSize;  \
                const int bufOffset = y * rowStride + x * TILE_WIDTH * KoBgrU8Traits::pixelSize;  \
                compositeOp->composite(m_dstBuffer + bufOffset, rowStride,      \
                                      m_srcBuffer + bufOffset, rowStride,      \
                                      m_mskBuffer + bufOffset, rowStride,                                                            \
                                      TILE_WIDTH, TILE_HEIGHT,                                         \
                                      OPACITY_HALF);                                                   \
            }                                                                                   \
        }

void KoCompositeOpsBenchmark::initTestCase()
{
    const int bufLen = IMG_HEIGHT * IMG_WIDTH * KoBgrU8Traits::pixelSize;

    m_dstBuffer = new quint8[bufLen];
    m_srcBuffer = new quint8[bufLen];
    m_mskBuffer = new quint8[bufLen];
}

// this is called before every benchmark
void KoCompositeOpsBenchmark::init()
{
    qsrand(42);

    for (int i = 0; i < int(IMG_WIDTH * IMG_HEIGHT * KoBgrU8Traits::pixelSize); i++) {
        const int randVal = qrand();

        m_srcBuffer[i] = randVal & 0x0000FF;
        m_dstBuffer[i] = (randVal & 0x00FF000) >> 8;
        m_mskBuffer[i] = (randVal & 0xFF0000) >> 16;
    }
}


void KoCompositeOpsBenchmark::cleanupTestCase()
{
    delete [] m_dstBuffer;
    delete [] m_srcBuffer;
    delete [] m_mskBuffer;
}

void KoCompositeOpsBenchmark::benchmarkCompositeOver()
{
    KoCompositeOp *compositeOp = KoOptimizedCompositeOpFactory::createOverOp32(KoColorSpaceRegistry::instance()->rgb8());
    QBENCHMARK{
        COMPOSITE_BENCHMARK
    }
}

void KoCompositeOpsBenchmark::benchmarkCompositeAlphaDarkenHard()
{
    KoCompositeOp *compositeOp = KoOptimizedCompositeOpFactory::createAlphaDarkenOpHard32(KoColorSpaceRegistry::instance()->rgb8());
    QBENCHMARK{
        COMPOSITE_BENCHMARK
    }
}


void KoCompositeOpsBenchmark::benchmarkCompositeAlphaDarkenCreamy()
{
    KoCompositeOp *compositeOp = KoOptimizedCompositeOpFactory::createAlphaDarkenOpCreamy32(KoColorSpaceRegistry::instance()->rgb8());
    QBENCHMARK{
        COMPOSITE_BENCHMARK
    }
}


QTEST_GUILESS_MAIN(KoCompositeOpsBenchmark)
