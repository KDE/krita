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

void KisSelectionTest::testSelectionActions()
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "tmp");

    KisPixelSelectionSP pixelSelection = dev->pixelSelection();
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
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "tmp");

    KisPixelSelectionSP pixelSelection = dev->pixelSelection();
    pixelSelection->select(QRect(0,0,20,20));
    pixelSelection->invert();

    QCOMPARE( dev->pixelSelection()->selected(100,100), MAX_SELECTED);
    QCOMPARE( dev->pixelSelection()->selected(22,22), MAX_SELECTED);
    QCOMPARE( dev->selection()->selected(100,100), MAX_SELECTED);
    QCOMPARE( dev->selection()->selected(22,22), MAX_SELECTED);
    QCOMPARE( dev->selection()->selected(10,10), MIN_SELECTED);
}



QTEST_KDEMAIN(KisSelectionTest, NoGUI)
#include "kis_selection_test.moc"


