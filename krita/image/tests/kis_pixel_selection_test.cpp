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

void KisPixelSelectionTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, cs, "merge test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE);
    KisPaintDeviceSP dev = layer->paintDevice();

    KisPixelSelectionSP selection = new KisPixelSelection();
    QVERIFY(selection);
    QVERIFY(selection->isTotallyUnselected(QRect(0, 0, 512, 512)));
    QVERIFY(selection->interestedInDirtyness() == true);

    selection = new KisPixelSelection(dev);
    QVERIFY(selection);
    QVERIFY(selection->isTotallyUnselected(QRect(0, 0, 512, 512)));
    QVERIFY(selection->interestedInDirtyness() == true);
    selection->setInterestedInDirtyness(true);
    selection->setDirty(QRect(10, 10, 10, 10));
}

void KisPixelSelectionTest::testSetSelected()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    QVERIFY(selection->selected(1, 1) == MIN_SELECTED);
    selection->setSelected(1, 1, MAX_SELECTED);
    QVERIFY(selection->selected(1, 1) == MAX_SELECTED);
    selection->setSelected(1, 1, 128);
    QVERIFY(selection->selected(1, 1) == 128);
}

void KisPixelSelectionTest::testMaskImage()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(10, 10, 50, 50), 128);
    QImage image = selection->maskImage(QRect(0, 0, 70, 70));
    QPoint pt;
    if (!TestUtil::compareQImages(pt, image, QImage(QString(FILES_DATA_DIR) + QDir::separator() + "pixel_selection_test.png"))) {
        QFAIL(QString("Failed to create correct mask image, wrong pixel at: %1,%2 ").arg(pt.x()).arg(pt.y()).toAscii());

    }
}

void KisPixelSelectionTest::testInvert()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(5, 5, 10, 10));
    selection->invert();
    QCOMPARE(selection->selected(20, 20), MAX_SELECTED);
    QCOMPARE(selection->selected(6, 6), MIN_SELECTED);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, qint32_MAX, qint32_MAX));
    QCOMPARE(selection->selectedRect(), QRect(0, 0, qint32_MAX, qint32_MAX));

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisFillPainter gc(dev);
    KoColor c(Qt::red, cs);
    gc.fillRect(5, 5, 100, 100, c);
    gc.end();

    selection = new KisPixelSelection(dev);
    selection->select(QRect(5, 5, 10, 10));
    selection->invert();

    QCOMPARE((int)selection->selected(6, 6), (int)MIN_SELECTED);
    QCOMPARE((int)selection->selected(20, 20), (int)MAX_SELECTED);

    QCOMPARE(selection->selectedExactRect(), QRect(5, 5, 100, 100));
    QCOMPARE(selection->selectedRect(), QRect(0, 0, 128, 128));

}

void KisPixelSelectionTest::testClear()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(5, 5, 300, 300));
    selection->clear(QRect(5, 5, 200, 200));
    QCOMPARE(selection->selected(0, 0), MIN_SELECTED);
    QCOMPARE(selection->selected(5, 5), MIN_SELECTED);
    QCOMPARE(selection->selected(10, 10), MIN_SELECTED);
    QCOMPARE(selection->selected(204, 204), MIN_SELECTED);
    QCOMPARE(selection->selected(205, 205), MAX_SELECTED);
    QCOMPARE(selection->selected(250, 250), MAX_SELECTED);
    // everything deselected
    selection->clear();
    // completely selected
    selection->invert();
    // deselect a certain area
    selection->clear(QRect(5, 5, 200, 200));
    QCOMPARE(selection->selected(0, 0), MAX_SELECTED);
    QCOMPARE(selection->selected(5, 5), MIN_SELECTED);
    QCOMPARE(selection->selected(10, 10), MIN_SELECTED);
    QCOMPARE(selection->selected(204, 204), MIN_SELECTED);
    QCOMPARE(selection->selected(205, 205), MAX_SELECTED);
    QCOMPARE(selection->selected(250, 250), MAX_SELECTED);
}

void KisPixelSelectionTest::testSelect()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(0, 0, 512, 441));
    for (int i = 0; i < 441; ++i) {
        for (int j = 0; j < 512; ++j) {
            QVERIFY(selection->selected(j, i) == MAX_SELECTED);
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
    sel1->addSelection(sel2);
    QCOMPARE(sel1->selectedExactRect(), QRect(0, 0, 75, 50));
}

void KisPixelSelectionTest::testSubtractSelection()
{
    KisPixelSelectionSP sel1 = new KisPixelSelection();
    KisPixelSelectionSP sel2 = new KisPixelSelection();
    sel1->select(QRect(0, 0, 50, 50));
    sel2->select(QRect(25, 0, 50, 50));
    sel1->subtractSelection(sel2);
    QCOMPARE(sel1->selectedExactRect(), QRect(0, 0, 25, 50));
}

void KisPixelSelectionTest::testIntersectSelection()
{
    KisPixelSelectionSP sel1 = new KisPixelSelection();
    KisPixelSelectionSP sel2 = new KisPixelSelection();
    sel1->select(QRect(0, 0, 50, 50));
    sel2->select(QRect(25, 0, 50, 50));
    sel1->intersectSelection(sel2);
    QCOMPARE(sel1->selectedExactRect(), QRect(25, 0, 25, 50));
}

void KisPixelSelectionTest::testTotally()
{
    KisPixelSelectionSP sel = new KisPixelSelection();
    sel->select(QRect(0, 0, 100, 100));
    QVERIFY(sel->isTotallyUnselected(QRect(100, 0, 100, 100)));
    QVERIFY(!sel->isTotallyUnselected(QRect(50, 0, 100, 100)));
    QVERIFY(sel->isProbablyTotallyUnselected(QRect(128, 0, 100, 100)));
}

void KisPixelSelectionTest::testUpdateProjection()
{
    KisSelectionSP sel = new KisSelection();
    KisPixelSelectionSP psel = new KisPixelSelection();
    psel->select(QRect(0, 0, 100, 100));
    psel->renderToProjection(sel.data());
    QCOMPARE(sel->selectedExactRect(), QRect(0, 0, 100, 100));
}

void KisPixelSelectionTest::testExactRectWithParent()
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPixelSelectionSP sel = new KisPixelSelection(dev);
    sel->select(QRect(10, 10, 200, 143));
    // Parent paint dev is empty
    QCOMPARE(sel->selectedExactRect(), QRect(0, 0, 0, 0));
    QCOMPARE(sel->selectedRect(), QRect(0, 0, 0, 0));
    KisFillPainter gc(dev);
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, dev->colorSpace()));

    QCOMPARE(sel->selectedExactRect(), QRect(10, 10, 90, 90));
    QCOMPARE(sel->selectedRect(), QRect(0, 0, 128, 128));

    gc.fillRect(0, 0, 500, 500, KoColor(Qt::blue, dev->colorSpace()));

    QCOMPARE(sel->selectedExactRect(), QRect(10, 10, 200, 143));
    QCOMPARE(sel->selectedRect(), QRect(0, 0, 256, 192));
}

QTEST_KDEMAIN(KisPixelSelectionTest, NoGUI)
#include "kis_pixel_selection_test.moc"

