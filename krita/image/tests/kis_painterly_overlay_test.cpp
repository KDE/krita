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

#include <qtest_kde.h>
#include "kis_painterly_overlay_test.h"
#include "kis_painterly_overlay.h"
#include "kis_painterly_overlay_colorspace.h"
#include "kis_types.h"
#include <kdebug.h>


void KisPainterlyOverlayTester::testConstructor()
{
    KisPainterlyOverlay * overlay = new KisPainterlyOverlay();
    Q_ASSERT( overlay );
    delete overlay;

    KisPainterlyOverlayColorSpace * cs = KisPainterlyOverlayColorSpace::instance();
    Q_ASSERT( cs );

}

void KisPainterlyOverlayTester::testPainterlyOverlayColorSpace()
{
    KisPainterlyOverlayColorSpace * cs = KisPainterlyOverlayColorSpace::instance();
    Q_ASSERT( cs );
    QCOMPARE( cs->pixelSize(), uint(8 * sizeof( float )) );
}

void KisPainterlyOverlayTester::testPainterlyOverlayColorSpaceCell()
{
    KisPainterlyOverlayColorSpace * cs = KisPainterlyOverlayColorSpace::instance();
    Q_ASSERT( cs );
    Q_ASSERT( cs->compositeOp( COMPOSITE_OVER ) != 0 );
    Q_ASSERT( cs->compositeOp( COMPOSITE_COPY ) != 0 );

    quint8 * bytes = cs->allocPixelBuffer( 1 );
    memset( bytes, 0, cs->pixelSize() );

    PainterlyOverlayFloatTraits::Cell * cell =
        reinterpret_cast<PainterlyOverlayFloatTraits::Cell *>( bytes );

    QVERIFY( cell->adsorbency == 0.0 );
    QVERIFY( cell->gravity == 0.0 );
    QVERIFY( cell->mixability == 0.0 );
    QVERIFY( cell->height == 0.0 );
    QVERIFY( cell->pigment_concentration == 0.0 );
    QVERIFY( cell->viscosity == 0.0 );
    QVERIFY( cell->volume == 0.0 );
    QVERIFY( cell->wetness == 0.0 );

    delete cell;
}

void KisPainterlyOverlayTester::testPainterlyOverlay()
{
    KisPainterlyOverlaySP overlay = new KisPainterlyOverlay();
    Q_ASSERT( overlay );
    KisPainterlyOverlayColorSpace * cs = KisPainterlyOverlayColorSpace::instance();
    Q_ASSERT( cs );
    QVERIFY( overlay->colorSpace() == cs );
    quint8 * bytes = cs->allocPixelBuffer( 1 );
    memset( bytes, 0,  cs->pixelSize());

    overlay->fill( 0, 0, 100, 100, bytes );

    {
        KisRectIteratorPixel it = overlay->createRectIterator(0, 0, 128, 128);
        while ( !it.isDone() ) {

            PainterlyOverlayFloatTraits::Cell * cell =
                reinterpret_cast<PainterlyOverlayFloatTraits::Cell *>( it.rawData() );

            QVERIFY( cell->adsorbency == 0.0 );
            QVERIFY( cell->gravity == 0.0 );
            QVERIFY( cell->mixability == 0.0 );
            QVERIFY( cell->height == 0.0 );
            QVERIFY( cell->pigment_concentration == 0.0 );
            QVERIFY( cell->viscosity == 0.0 );
            QVERIFY( cell->volume == 0.0 );
            QVERIFY( cell->wetness == 0.0 );

            cell->wetness = 1.0;
            cell->mixability = 1.1;

            ++it;
        }
        QVERIFY( overlay->exactBounds() == QRect( 0, 0, 128, 128 ) );
    }


    {
        KisRectConstIteratorPixel it = overlay->createRectConstIterator(10, 10, 10, 10);
        while ( !it.isDone() ) {

            const PainterlyOverlayFloatTraits::Cell * cell =
                reinterpret_cast<const PainterlyOverlayFloatTraits::Cell *>( it.rawData() );

            QVERIFY( cell->wetness == 1.0 );
            QVERIFY( cell->mixability < 1.2 && cell->mixability > 1.0 );

            ++it;
        }
    }


}

QTEST_KDEMAIN(KisPainterlyOverlayTester, NoGUI)
#include "kis_painterly_overlay_test.moc"
