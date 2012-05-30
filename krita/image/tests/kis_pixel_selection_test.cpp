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

#include "kis_pixel_selection_test.h"
#include <qtest_kde.h>


#include <kis_debug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_pixel_selection.h"
#include "testutil.h"
#include "kis_fill_painter.h"
#include "kis_transaction.h"
#include "commands/kis_selection_commands.h"


void KisPixelSelectionTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, cs, "merge test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP dev = layer->paintDevice();

    KisPixelSelectionSP selection = new KisPixelSelection();
    QVERIFY(selection);
    QVERIFY(selection->isTotallyUnselected(QRect(0, 0, 512, 512)));

    selection = new KisPixelSelection(new KisSelectionDefaultBounds(dev));
    QVERIFY(selection);
    QVERIFY(selection->isTotallyUnselected(QRect(0, 0, 512, 512)));
    selection->setDirty(QRect(10, 10, 10, 10));
}

void KisPixelSelectionTest::testSetSelected()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    QVERIFY(TestUtil::alphaDevicePixel(selection, 1, 1) == MIN_SELECTED);
    TestUtil::alphaDeviceSetPixel(selection, 1, 1, MAX_SELECTED);
    QVERIFY(TestUtil::alphaDevicePixel(selection, 1, 1) == MAX_SELECTED);
    TestUtil::alphaDeviceSetPixel(selection, 1, 1, 128);
    QVERIFY(TestUtil::alphaDevicePixel(selection, 1, 1) == 128);
}

void KisPixelSelectionTest::testInvert()
{
    KisDefaultBounds defaultBounds;
    
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(5, 5, 10, 10));
    selection->invert();

    QCOMPARE(TestUtil::alphaDevicePixel(selection, 20, 20), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 6, 6), MIN_SELECTED);
    QCOMPARE(selection->selectedExactRect(), defaultBounds.bounds());
    QCOMPARE(selection->selectedRect(), defaultBounds.bounds());
}

void KisPixelSelectionTest::testInvertWithImage()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 200, 200, cs, "merge test");

    KisSetEmptyGlobalSelectionCommand(image).redo();
    KisPixelSelectionSP selection =  image->globalSelection()->getOrCreatePixelSelection();
    selection->select(QRect(5, 5, 10, 10));
    selection->invert();
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 200, 200));

    // round trip
    selection->invert();
    QCOMPARE(selection->selectedExactRect(), QRect(5, 5, 10, 10));
}

void KisPixelSelectionTest::testClear()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(5, 5, 300, 300));
    selection->clear(QRect(5, 5, 200, 200));

    QCOMPARE(TestUtil::alphaDevicePixel(selection, 0, 0), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 5, 5), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 10, 10), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 204, 204), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 205, 205), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 250, 250), MAX_SELECTED);

    // everything deselected
    selection->clear();
    // completely selected
    selection->invert();
    // deselect a certain area
    selection->clear(QRect(5, 5, 200, 200));

    QCOMPARE(TestUtil::alphaDevicePixel(selection, 0, 0), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 5, 5), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 10, 10), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 204, 204), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 205, 205), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection, 250, 250), MAX_SELECTED);
}

void KisPixelSelectionTest::testSelect()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(0, 0, 512, 441));
    for (int i = 0; i < 441; ++i) {
        for (int j = 0; j < 512; ++j) {
            QCOMPARE(TestUtil::alphaDevicePixel(selection, j, i), MAX_SELECTED);
        }
    }
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 512, 441));
    QCOMPARE(selection->selectedRect(), QRect(0, 0, 512, 448));
}

void KisPixelSelectionTest::testExtent()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(0, 0, 516, 441));
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 516, 441));
    QCOMPARE(selection->selectedRect(), QRect(0, 0, 576, 448));
}


void KisPixelSelectionTest::testAddSelection()
{
    KisPixelSelectionSP sel1 = new KisPixelSelection();
    KisPixelSelectionSP sel2 = new KisPixelSelection();
    sel1->select(QRect(0, 0, 50, 50));
    sel2->select(QRect(25, 0, 50, 50));
    sel1->applySelection(sel2, SELECTION_ADD);
    QCOMPARE(sel1->selectedExactRect(), QRect(0, 0, 75, 50));
}

void KisPixelSelectionTest::testSubtractSelection()
{
    KisPixelSelectionSP sel1 = new KisPixelSelection();
    KisPixelSelectionSP sel2 = new KisPixelSelection();
    sel1->select(QRect(0, 0, 50, 50));
    sel2->select(QRect(25, 0, 50, 50));
    sel1->applySelection(sel2, SELECTION_SUBTRACT);
    QCOMPARE(sel1->selectedExactRect(), QRect(0, 0, 25, 50));
}

void KisPixelSelectionTest::testIntersectSelection()
{
    KisPixelSelectionSP sel1 = new KisPixelSelection();
    KisPixelSelectionSP sel2 = new KisPixelSelection();
    sel1->select(QRect(0, 0, 50, 50));
    sel2->select(QRect(25, 0, 50, 50));
    sel1->applySelection(sel2, SELECTION_INTERSECT);
    QCOMPARE(sel1->selectedExactRect(), QRect(25, 0, 25, 50));
}

void KisPixelSelectionTest::testTotally()
{
    KisPixelSelectionSP sel = new KisPixelSelection();
    sel->select(QRect(0, 0, 100, 100));
    QVERIFY(sel->isTotallyUnselected(QRect(100, 0, 100, 100)));
    QVERIFY(!sel->isTotallyUnselected(QRect(50, 0, 100, 100)));
}

void KisPixelSelectionTest::testUpdateProjection()
{
    KisSelectionSP sel = new KisSelection();
    KisPixelSelectionSP psel = new KisPixelSelection();
    psel->select(QRect(0, 0, 100, 100));
    psel->renderToProjection(sel->projection().data());
    QCOMPARE(sel->selectedExactRect(), QRect(0, 0, 100, 100));
}

void KisPixelSelectionTest::testExactRectWithImage()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 200, 200, cs, "merge test");

    KisSetEmptyGlobalSelectionCommand(image).redo();
    KisPixelSelectionSP selection = image->globalSelection()->getOrCreatePixelSelection();
    selection->select(QRect(100, 50, 200, 100));
    QCOMPARE(selection->selectedExactRect(), QRect(100, 50, 200, 100));
}



void KisPixelSelectionTest::testUndo()
{
    KisPixelSelectionSP psel = new KisPixelSelection();

    {
        KisTransaction transaction("", psel);
        psel->select(QRect(50, 50, 100, 100));
        transaction.end();
    }

    QCOMPARE(psel->selectedExactRect(), QRect(50, 50, 100, 100));

    {
        KisTransaction transaction("", psel);
        psel->select(QRect(150, 50, 100, 100));
        transaction.end();
    }

    QCOMPARE(psel->selectedExactRect(), QRect(50, 50, 200, 100));

    {
        KisTransaction transaction("", psel);
        psel->crop(QRect(75, 75, 10, 10));
        transaction.revert();
    }

    QCOMPARE(psel->selectedExactRect(), QRect(50, 50, 200, 100));
}

QTEST_KDEMAIN(KisPixelSelectionTest, NoGUI)
#include "kis_pixel_selection_test.moc"

