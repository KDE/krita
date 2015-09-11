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

const int IMG_WIDTH = 4096;
const int IMG_HEIGHT = 4096;

const quint8 OPACITY_HALF = 128;

const int TILES_IN_WIDTH = IMG_WIDTH / TILE_WIDTH;
const int TILES_IN_HEIGHT = IMG_HEIGHT / TILE_HEIGHT;


#define COMPOSITE_BENCHMARK \
        for (int y = 0; y < TILES_IN_HEIGHT; y++){                                              \
            for (int x = 0; x < TILES_IN_WIDTH; x++){                                           \
                compositeOp->composite(m_dstBuffer, TILE_WIDTH * KoBgrU16Traits::pixelSize,      \
                                      m_srcBuffer, TILE_WIDTH * KoBgrU16Traits::pixelSize,      \
                                      0, 0,                                                            \
                                      TILE_WIDTH, TILE_HEIGHT,                                         \
                                      OPACITY_HALF);                                                   \
            }                                                                                   \
        }

void KoCompositeOpsBenchmark::initTestCase()
{
    m_dstBuffer = new quint8[ TILE_WIDTH * TILE_HEIGHT * KoBgrU16Traits::pixelSize ];
    m_srcBuffer = new quint8[ TILE_WIDTH * TILE_HEIGHT * KoBgrU16Traits::pixelSize ];
}

// this is called before every benchmark
void KoCompositeOpsBenchmark::init()
{
    memset(m_dstBuffer, 42 , TILE_WIDTH * TILE_HEIGHT * KoBgrU16Traits::pixelSize);
    memset(m_srcBuffer, 42 , TILE_WIDTH * TILE_HEIGHT * KoBgrU16Traits::pixelSize);
}


void KoCompositeOpsBenchmark::cleanupTestCase()
{
    delete [] m_dstBuffer;
    delete [] m_srcBuffer;
}

void KoCompositeOpsBenchmark::benchmarkCompositeOver()
{
    KoCompositeOp *compositeOp = KoOptimizedCompositeOpFactory::createOverOp32(KoColorSpaceRegistry::instance()->rgb16());
    QBENCHMARK{
        COMPOSITE_BENCHMARK
    }
}

void KoCompositeOpsBenchmark::benchmarkCompositeAlphaDarken()
{
    //KoCompositeOpAlphaDarken<KoBgrU16Traits> compositeOp(0);
    KoCompositeOp *compositeOp = KoOptimizedCompositeOpFactory::createAlphaDarkenOp32(KoColorSpaceRegistry::instance()->rgb16());
    QBENCHMARK{
        COMPOSITE_BENCHMARK
    }
}


QTEST_GUILESS_MAIN(KoCompositeOpsBenchmark)
