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

#include "kis_datamanager.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pixel_selection.h"

void KisPainterTest::allCsApplicator(void (KisPainterTest::* funcPtr)( const KoColorSpace*cs ) )
{
    QList<QString> csIds = KoColorSpaceRegistry::instance()->keys();

    foreach( QString csId, csIds ) {

        kDebug() <<"Testing with" << csId;

        QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor ( csId );
        if ( profiles.size() == 0 ) {
            const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace( csId, 0 );
            if ( cs->compositeOp( COMPOSITE_OVER ) != 0) {
                if ( cs ) ( this->*funcPtr )( cs );
            }
            else {
                kDebug() <<"Cannot bitBlt for cs" << csId;
            }
        }
        else {
            foreach( const KoColorProfile * profile, profiles ) {
                const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace( csId, profile );
                if ( cs->compositeOp( COMPOSITE_OVER ) != 0) {
                    if ( cs ) ( this->*funcPtr )( cs );
                }
                else {
                    kDebug() <<"Cannot bitBlt for cs" << csId;
                }
            }

        }
    }
}


/*

Note: the bltSelection tests assume the following geometry:

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
void KisPainterTest::testPaintDeviceBltSelection(const KoColorSpace * cs)
{
    KisPaintDeviceSP dst = new KisPaintDevice( cs, "dst");

    KisPaintDeviceSP src = new KisPaintDevice( cs, "src" );
    src->fill( 0, 0, 20, 20, KoColor( Qt::red, 128, cs ).data() );

    QCOMPARE( src->exactBounds(), QRect( 0, 0, 20, 20 ) );

    KisSelectionSP selection = new KisSelection();
    selection->getOrCreatePixelSelection()->select(QRect(10,10,20,20));
    selection->updateProjection();
    QCOMPARE( selection->selectedExactRect(), QRect( 10, 10, 20, 20 ) );

    KisPainter painter(dst);
    painter.setSelection(selection);
    
    painter.bltSelection(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

    QImage img = dst->convertToQImage(0);
    img.save( "blt_Selection_" + cs->name() + ".png" );

    QCOMPARE( dst->exactBounds(), QRect( 10, 10, 10, 10 ) );
}

void KisPainterTest::testPaintDeviceBltSelection()
{
    allCsApplicator( &KisPainterTest::testPaintDeviceBltSelection );
}

void KisPainterTest::testPaintDeviceBltSelectionIrregular(const KoColorSpace * cs)
{

    KisPaintDeviceSP dst = new KisPaintDevice( cs, "dst");

    KisPaintDeviceSP src = new KisPaintDevice( cs, "src" );
    src->fill( 0, 0, 20, 20, KoColor( Qt::red, 128, cs ).data() );

    QCOMPARE( src->exactBounds(), QRect( 0, 0, 20, 20 ) );

    KisPixelSelectionSP psel = new KisPixelSelection();
    psel->select(QRect(10,15,20,15));
    psel->select(QRect(15,10,15,5));
    QCOMPARE( psel->selectedExactRect(), QRect( 10, 10, 20, 20 ) );
    QCOMPARE( psel->selected(13,13), MIN_SELECTED);
    KisSelectionSP sel = new KisSelection();
    sel->setPixelSelection(psel);
    KisPainter painter(dst);
    painter.setSelection(sel);
    painter.bltSelection(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

    QImage img = dst->convertToQImage(0);
    img.save( "blt_Selection_irregular" + cs->name() + ".png" );

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


void KisPainterTest::testPaintDeviceBltSelectionIrregular()
{
    allCsApplicator( &KisPainterTest::testPaintDeviceBltSelectionIrregular );
}

void KisPainterTest::testPaintDeviceBltSelectionInverted(const KoColorSpace * cs)
{
    KisPaintDeviceSP dst = new KisPaintDevice( cs, "dst");

    KisPaintDeviceSP src = new KisPaintDevice( cs, "src" );
    src->fill( 0, 0, 30, 30, KoColor( Qt::red, 128, cs ).data() );

    QCOMPARE( src->exactBounds(), QRect( 0, 0, 30, 30 ) );

    KisPixelSelectionSP Selection = KisPixelSelectionSP(new KisPixelSelection());
    Selection->select(QRect(10,10,20,20));
    Selection->invert();
    KisSelectionSP sel = new KisSelection();
    sel->setPixelSelection(Selection);
    sel->updateProjection();
    
    KisPainter painter(dst);
    painter.setSelection(sel);
    painter.bltSelection(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();
    qDebug() << ">>>>>>>>>>>>>>>>>>>>> going to save";
    dst->convertToQImage(0, 0, 0, 50, 50).save( "blt_Selection_inverted.png" );
    
    QCOMPARE( dst->exactBounds(), QRect( 0, 0, 30, 30 ) );
}

void KisPainterTest::testPaintDeviceBltSelectionInverted()
{
    //allCsApplicator( &KisPainterTest::testPaintDeviceBltSelectionInverted );
    testPaintDeviceBltSelectionInverted(KoColorSpaceRegistry::instance()->rgb8());
}


void KisPainterTest::testSelectionBltSelection()
{
    KisPixelSelectionSP src = new KisPixelSelection();
    src->select(QRect(0,0,20,20));
    QCOMPARE( src->selectedExactRect(), QRect( 0, 0, 20, 20 ) );

    KisPixelSelectionSP Selection = new KisPixelSelection();
    Selection->select(QRect(10,10,20,20));
    QCOMPARE( Selection->selectedExactRect(), QRect( 10, 10, 20, 20 ) );
    KisSelectionSP sel = new KisSelection();
    sel->setPixelSelection(Selection);
    sel->updateProjection();
    KisPixelSelectionSP dst = new KisPixelSelection();
    KisPainter painter(dst);
    painter.setSelection(sel);
    painter.bltSelection(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

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
void KisPainterTest::testSelectionBltSelectionIrregular()
{

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "temporary");

    KisPixelSelectionSP src = KisPixelSelectionSP(new KisPixelSelection(dev));
    src->select(QRect(0,0,20,20));
    QCOMPARE( src->selectedExactRect(), QRect( 0, 0, 20, 20 ) );

    KisPixelSelectionSP Selection = KisPixelSelectionSP(new KisPixelSelection(dev));
    Selection->select(QRect(10,15,20,15));
    Selection->select(QRect(15,10,15,5));
    QCOMPARE( Selection->selectedExactRect(), QRect( 10, 10, 20, 20 ) );
    QCOMPARE( Selection->selected(13,13), MIN_SELECTED);

    KisSelectionSP sel = new KisSelection();
    sel->setPixelSelection(Selection);
    sel->updateProjection();

    KisPixelSelectionSP dst = KisPixelSelectionSP(new KisPixelSelection(dev));
    KisPainter painter(dst);
    painter.setSelection(sel);

    painter.bltSelection(0, 0,
                    dst->colorSpace()->compositeOp(COMPOSITE_OVER),
                    src,
                    OPACITY_OPAQUE,
                    0, 0, 30, 30);
    painter.end();

    QCOMPARE( dst->selectedExactRect(), QRect( 10, 10, 10, 10 ) );
    QCOMPARE( dst->selected(13,13), MIN_SELECTED);
}

void KisPainterTest::testSimpleAlphaCopy()
{
    KisPaintDeviceSP src = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8(), "src");
    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8(), "dst");
    quint8 p = 128;
    src->fill(0, 0, 100, 100, &p);
    QVERIFY(src->exactBounds() == QRect(0, 0, 100, 100));
    KisPainter gc(dst);
    gc.setCompositeOp(KoColorSpaceRegistry::instance()->alpha8()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(QPoint(0, 0), src, src->exactBounds());
    gc.end();
    QCOMPARE(dst->exactBounds(), QRect(0, 0, 100, 100));
    
}

QTEST_KDEMAIN(KisPainterTest, NoGUI);
#include "kis_painter_test.moc"


