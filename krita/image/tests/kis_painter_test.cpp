/*
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

#include <qtest_kde.h>
#include "kis_painter_test.h"

#include <kdebug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pixel_selection.h"

void KisPainterTest::allCsApplicator(void (KisPainterTest::* funcPtr)( KoColorSpace*cs ) )
{
    QList<QString> csIds = KoColorSpaceRegistry::instance()->keys();

    foreach( QString csId, csIds ) {

        kDebug() << "Testing with " << csId << endl;

        QList<KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor ( csId );
        if ( profiles.size() == 0 ) {
            KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace( csId, 0 );
            if ( cs->compositeOp( COMPOSITE_OVER ) != 0) {
                if ( cs ) ( this->*funcPtr )( cs );
            }
            else {
                kDebug() << "Cannot bitBlt for cs " << csId << endl;
            }
        }
        else {
            foreach( KoColorProfile * profile, profiles ) {
                KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace( csId, profile );
                if ( cs->compositeOp( COMPOSITE_OVER ) != 0) {
                    if ( cs ) ( this->*funcPtr )( cs );
                }
                else {
                    kDebug() << "Cannot bitBlt for cs " << csId << endl;
                }
            }

        }
    }
}


/*

Note: the bltMask tests assume the following geometry:

0,0               0,30
  +---------+------+
  |  10,10  |      |
  |    +----+      |
  |    |####|      |
  |    |####|      |
  +----+----+      |
  |       20,20    |
  |                |
  |                |
  +----------------+
                  30,30
 */
void KisPainterTest::testPaintDeviceBltMask(KoColorSpace * cs)
{

    KisPaintDeviceSP dst = new KisPaintDevice( cs, "dst");

    KisPaintDeviceSP src = new KisPaintDevice( cs, "src" );
    src->fill( 0, 0, 20, 20, KoColor( Qt::red, 128, cs ).data() );

    QCOMPARE( src->exactBounds(), QRect( 0, 0, 20, 20 ) );

    KisPixelSelectionSP mask = KisPixelSelectionSP(new KisPixelSelection(dst));
    mask->select(QRect(10,10,20,20));
    QCOMPARE( mask->selectedExactRect(), QRect( 10, 10, 20, 20 ) );


    KisPainter painter(dst);

    painter.bltMask(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    mask,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

    QImage img = dst->convertToQImage(0);
    img.save( "bla_" + cs->name() + ".png" );

    QCOMPARE( dst->exactBounds(), QRect( 10, 10, 10, 10 ) );
}

void KisPainterTest::testPaintDeviceBltMask()
{
    allCsApplicator( &KisPainterTest::testPaintDeviceBltMask );
}

void KisPainterTest::testPaintDeviceBltMaskIrregular(KoColorSpace * cs)
{

    KisPaintDeviceSP dst = new KisPaintDevice( cs, "dst");

    KisPaintDeviceSP src = new KisPaintDevice( cs, "src" );
    src->fill( 0, 0, 20, 20, KoColor( Qt::red, 128, cs ).data() );

    QCOMPARE( src->exactBounds(), QRect( 0, 0, 20, 20 ) );

    KisPixelSelectionSP mask = KisPixelSelectionSP(new KisPixelSelection(dst));
    mask->select(QRect(10,15,20,15));
    mask->select(QRect(15,10,15,5));
    QCOMPARE( mask->selectedExactRect(), QRect( 10, 10, 20, 20 ) );
    QCOMPARE( mask->selected(13,13), MIN_SELECTED);

    KisPainter painter(dst);

    painter.bltMask(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    mask,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

    QImage img = dst->convertToQImage(0);
    img.save( "bla_" + cs->name() + ".png" );

    QCOMPARE( dst->exactBounds(), QRect( 10, 10, 10, 10 ) );
    foreach ( KoChannelInfo * channel, cs->channels() ) {
        // Only compare alpha if there actually is an alpha channel in
        // this colorspace
        if ( channel->channelType() == KoChannelInfo::ALPHA ) {
            QColor c;
            quint8 alpha;

            dst->pixel( 13, 13, &c, &alpha );

            QCOMPARE( ( int ) alpha, ( int ) OPACITY_TRANSPARENT );
        }
    }
}


void KisPainterTest::testPaintDeviceBltMaskIrregular()
{
    allCsApplicator( &KisPainterTest::testPaintDeviceBltMaskIrregular );
}

void KisPainterTest::testPaintDeviceBltMaskInverted(KoColorSpace * cs)
{

    KisPaintDeviceSP dst = new KisPaintDevice( cs, "dst");

    KisPaintDeviceSP src = new KisPaintDevice( cs, "src" );
    src->fill( 0, 0, 30, 30, KoColor( Qt::red, 128, cs ).data() );

    QCOMPARE( src->exactBounds(), QRect( 0, 0, 30, 30 ) );

    KisPixelSelectionSP mask = KisPixelSelectionSP(new KisPixelSelection(dst));
    mask->select(QRect(10,10,20,20));
    mask->invert();
    QCOMPARE( mask->selectedExactRect(), QRect( 10, 10, 20, 20 ) );


    KisPainter painter(dst);

    painter.bltMask(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    mask,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

    QImage img = dst->convertToQImage(0);
    img.save( "bla_" + cs->name() + ".png" );

    QCOMPARE( dst->exactBounds(), QRect( 0, 0, 30, 30 ) );
}

void KisPainterTest::testPaintDeviceBltMaskInverted()
{
    allCsApplicator( &KisPainterTest::testPaintDeviceBltMaskInverted );
}


void KisPainterTest::testSelectionBltMask()
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "temporary");

    KisPixelSelectionSP src = KisPixelSelectionSP(new KisPixelSelection(dev));
    src->select(QRect(0,0,20,20));
    QCOMPARE( src->selectedExactRect(), QRect( 0, 0, 20, 20 ) );

    KisPixelSelectionSP mask = KisPixelSelectionSP(new KisPixelSelection(dev));
    mask->select(QRect(10,10,20,20));
    QCOMPARE( mask->selectedExactRect(), QRect( 10, 10, 20, 20 ) );

    KisPixelSelectionSP dst = KisPixelSelectionSP(new KisPixelSelection(dev));
    KisPainter painter(dst);


    painter.bltMask(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    mask,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

    dst->convertToQImage(0).save( "bla.png" );

    QCOMPARE( dst->selectedExactRect(), QRect( 10, 10, 10, 10 ) );

    KisRectConstIteratorPixel it = dst->createRectConstIterator(10, 10, 10, 10);
    while ( !it.isDone() ) {
        // These are selections, so only one channel and it should
        // be totally selected
        QCOMPARE( it.rawData()[0], MAX_SELECTED );
        ++it;
    }
}

/*

Test with non-square selection

0,0               0,30
  +-----------+------+
  |    13,13  |      |
  |      x +--+      |
  |     +--+##|      |
  |     |#####|      |
  +-----+-----+      |
  |         20,20    |
  |                  |
  |                  |
  +------------------+
                  30,30
 */
void KisPainterTest::testSelectionBltMaskIrregular()
{

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "temporary");

    KisPixelSelectionSP src = KisPixelSelectionSP(new KisPixelSelection(dev));
    src->select(QRect(0,0,20,20));
    QCOMPARE( src->selectedExactRect(), QRect( 0, 0, 20, 20 ) );

    KisPixelSelectionSP mask = KisPixelSelectionSP(new KisPixelSelection(dev));
    mask->select(QRect(10,15,20,15));
    mask->select(QRect(15,10,15,5));
    QCOMPARE( mask->selectedExactRect(), QRect( 10, 10, 20, 20 ) );
    QCOMPARE( mask->selected(13,13), MIN_SELECTED);

    KisPixelSelectionSP dst = KisPixelSelectionSP(new KisPixelSelection(dev));
    KisPainter painter(dst);


    painter.bltMask(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    mask,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

    dst->convertToQImage(0).save( "bla.png" );

    QCOMPARE( dst->selectedExactRect(), QRect( 10, 10, 10, 10 ) );
    QCOMPARE( dst->selected(13,13), MIN_SELECTED);
}



QTEST_KDEMAIN(KisPainterTest, NoGUI)
#include "kis_painter_test.moc"


