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
#include "kis_selection_test.h"

#include <kdebug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_pixel_selection.h"


void KisSelectionTest::testSelectionComponents()
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "tmp");
    QVERIFY( dev->hasSelection() == false );
    QVERIFY( dev->selection() != 0 );
    QVERIFY( dev->hasSelection() == true );

    KisSelectionSP selection = dev->selection();
    QVERIFY( selection->hasPixelSelection() == false );
    QVERIFY( selection->hasShapeSelection() == false );
    QVERIFY( selection->pixelSelection() == 0 );
    QVERIFY( selection->shapeSelection() == 0 );

    KisPixelSelectionSP pixelSelection = selection->getOrCreatePixelSelection();
    QVERIFY( selection->pixelSelection() == pixelSelection );
    QVERIFY( selection->hasPixelSelection() == true );

}

void KisSelectionTest::testSelectionActions()
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "tmp");
    KisPixelSelectionSP pixelSelection = new KisPixelSelection();

    KisSelectionSP selection = dev->selection();
    QVERIFY( selection->hasPixelSelection() == false );
    QVERIFY( selection->hasShapeSelection() == false );
    selection->setPixelSelection( pixelSelection );

    pixelSelection->select(QRect(0,0,20,20));

    KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection(dev));
    tmpSel->select(QRect(10,0,20,20));

    pixelSelection->addSelection(tmpSel);
    QCOMPARE( dev->selection()->selectedExactRect(), QRect( 0, 0, 30, 20 ) );


    pixelSelection->clear();
    pixelSelection->select(QRect(0,0,20,20));

    pixelSelection->subtractSelection(tmpSel);
    QCOMPARE( dev->selection()->selectedExactRect(), QRect( 0, 0, 10, 20 ) );


    pixelSelection->clear();
    pixelSelection->select(QRect(0,0,20,20));

    pixelSelection->intersectSelection(tmpSel);
    QCOMPARE( dev->selection()->selectedExactRect(), QRect( 10, 0, 10, 20 ) );
}

void KisSelectionTest::testInvertSelection()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs, "tmp");
    quint8* pixel = cs->allocPixelBuffer( 1 );
    cs->fromQColor( Qt::white, pixel );
    dev->fill( 0, 0, 512, 512, pixel);

    KisSelectionSP selection = dev->selection();
    KisPixelSelectionSP pixelSelection = selection->getOrCreatePixelSelection();
    pixelSelection->select(QRect(20,20,20,20));
    QCOMPARE( pixelSelection->selected( 30, 30 ), MAX_SELECTED );
    QCOMPARE( pixelSelection->selected( 0, 0 ), MIN_SELECTED );
    QCOMPARE( pixelSelection->selected( 512, 512 ), MIN_SELECTED );

    pixelSelection->invert();

    QCOMPARE( pixelSelection->selected(100,100), MAX_SELECTED);
    QCOMPARE( pixelSelection->selected(22,22), MIN_SELECTED);
    QCOMPARE( pixelSelection->selected( 0, 0 ), MAX_SELECTED );
    QCOMPARE( pixelSelection->selected( 512, 512 ), MAX_SELECTED );

    // XXX: This should happen automatically
    selection->updateProjection();

    QCOMPARE( selection->selected(100,100), MAX_SELECTED);
    QCOMPARE( selection->selected(22,22), MIN_SELECTED);
    QCOMPARE( selection->selected(10,10), MAX_SELECTED);
}



QTEST_KDEMAIN(KisSelectionTest, NoGUI)
#include "kis_selection_test.moc"


