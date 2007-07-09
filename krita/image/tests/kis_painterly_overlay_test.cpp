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
#include <kdebug.h>


void KisPainterlyOverlayTester::testConstructor()
{
    KisPainterlyOverlay * overlay = new KisPainterlyOverlay();
    Q_ASSERT( overlay );
    delete overlay;
}

void KisPainterlyOverlayTester::testPainterlyOverlayColorSpace()
{
    KisPainterlyOverlayColorSpace * cs = KisPainterlyOverlayColorSpace::instance();
    Q_ASSERT( cs );
    QCOMPARE( cs->pixelSize(), uint(9 * sizeof( float )) );
}

void KisPainterlyOverlayTester::testPainterlyOverlayColorSpaceCell()
{
    KisPainterlyOverlayColorSpace * cs = KisPainterlyOverlayColorSpace::instance();
    Q_ASSERT( cs );
    Q_ASSERT( cs->compositeOp( COMPOSITE_OVER ) == 0 );
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



QTEST_KDEMAIN(KisPainterlyOverlayTester, NoGUI)
#include "kis_painterly_overlay_test.moc"
