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

#include "kis_selection_test.h"
#include <qtest_kde.h>


#include <kis_debug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_pixel_selection.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_mask.h"
#include "kis_transparency_mask.h"
#include "testutil.h"

void KisSelectionTest::testSelectionComponents()
{

    KisSelectionSP selection = new KisSelection();
    QVERIFY(selection->hasPixelSelection() == false);
    QVERIFY(selection->hasShapeSelection() == false);
    QVERIFY(selection->pixelSelection() == 0);
    QVERIFY(selection->shapeSelection() == 0);

    KisPixelSelectionSP pixelSelection = selection->getOrCreatePixelSelection();
    QVERIFY(selection->pixelSelection() == pixelSelection);
    QVERIFY(selection->hasPixelSelection() == true);

    KisMaskSP mask = new KisTransparencyMask();
    mask->select(QRect(0, 0, 100, 100));
    selection = new KisSelection(0, mask);
    selection->updateProjection();
    QVERIFY(selection->hasPixelSelection() == true);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));
}

void KisSelectionTest::testSelectionActions()
{
    KisPixelSelectionSP pixelSelection = new KisPixelSelection();

    KisSelectionSP selection = new KisSelection();
    QVERIFY(selection->hasPixelSelection() == false);
    QVERIFY(selection->hasShapeSelection() == false);
    selection->setPixelSelection(pixelSelection);

    pixelSelection->select(QRect(0, 0, 20, 20));

    KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());
    tmpSel->select(QRect(10, 0, 20, 20));

    pixelSelection->addSelection(tmpSel);
    QCOMPARE(pixelSelection->selectedExactRect(), QRect(0, 0, 30, 20));
    selection->updateProjection();
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 30, 20));

    pixelSelection->clear();
    pixelSelection->select(QRect(0, 0, 20, 20));

    pixelSelection->subtractSelection(tmpSel);
    selection->updateProjection();
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 10, 20));

    pixelSelection->clear();
    selection->updateProjection();
    pixelSelection->select(QRect(0, 0, 20, 20));

    pixelSelection->intersectSelection(tmpSel);
    selection->updateProjection();
    QCOMPARE(selection->selectedExactRect(), QRect(10, 0, 10, 20));
}

void KisSelectionTest::testInvertSelection()
{
    KisSelectionSP selection = new KisSelection();
    KisPixelSelectionSP pixelSelection = selection->getOrCreatePixelSelection();
    pixelSelection->select(QRect(20, 20, 20, 20));
    QCOMPARE(pixelSelection->selected(30, 30), MAX_SELECTED);
    QCOMPARE(pixelSelection->selected(0, 0), MIN_SELECTED);
    QCOMPARE(pixelSelection->selected(512, 512), MIN_SELECTED);

    pixelSelection->invert();


    QCOMPARE(pixelSelection->selected(100, 100), MAX_SELECTED);
    QCOMPARE(pixelSelection->selected(22, 22), MIN_SELECTED);
    QCOMPARE(pixelSelection->selected(0, 0), MAX_SELECTED);
    QCOMPARE(pixelSelection->selected(512, 512), MAX_SELECTED);
    pixelSelection->convertToQImage(0, 0, 0, 100, 100).save("yyy.png");
    // XXX: This should happen automatically
    selection->updateProjection();
    selection->convertToQImage(0, 0, 0, 100, 100).save("zzz.png");

    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, qint32_MAX, qint32_MAX));
    QCOMPARE(selection->selectedRect(), QRect(0, 0, qint32_MAX, qint32_MAX));

    QCOMPARE(selection->selected(100, 100), MAX_SELECTED);
    QCOMPARE(selection->selected(22, 22), MIN_SELECTED);
    QCOMPARE(selection->selected(10, 10), MAX_SELECTED);
    QCOMPARE(selection->selected(0, 0), MAX_SELECTED);
    QCOMPARE(selection->selected(512, 512), MAX_SELECTED);

}

void KisSelectionTest::testUpdateSelectionProjection()
{
    KisSelectionSP selection = new KisSelection();
    QVERIFY(selection->selectedExactRect().isNull());

    // Now fill the layer with some opaque pixels
    KisFillPainter gc(selection->getOrCreatePixelSelection());
    gc.fillRect(QRect(0, 0, 100, 100),
                KoColor(QColor(0, 0, 0, 0), KoColorSpaceRegistry::instance()->rgb8()),
                MAX_SELECTED);
    gc.end();

    QVERIFY(selection->pixelSelection()->selectedExactRect() == QRect(0, 0, 100, 100));
    selection->updateProjection();
    QCOMPARE(selection->pixelSelection()->selectedExactRect(), QRect(0, 0, 100, 100));
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));
}

void KisSelectionTest::testUpdatePixelSelection()
{
    KisSelectionSP selection = new KisSelection();
    KisPixelSelectionSP pSel = selection->getOrCreatePixelSelection();
    pSel->select(QRect(0, 0, 348, 212));
    QVERIFY(selection->pixelSelection()->selectedExactRect() == QRect(0, 0, 348, 212));
    selection->updateProjection(QRect(0, 0, 348, 212));
    for (int i = 0; i < 212; ++i) {
        for (int j = 0; j < 348; ++j) {
            QVERIFY(selection->selected(j, i) == MAX_SELECTED);
        }
    }

}

void KisSelectionTest::testCopy()
{
    KisSelectionSP sel = new KisSelection();
    sel->getOrCreatePixelSelection()->select(QRect(10, 10, 200, 200), 128);
    KisSelectionSP sel2 = new KisSelection(*sel.data());
    QCOMPARE(sel2->selectedExactRect(), sel->selectedExactRect());
    QPoint errpoint;
    if (!TestUtil::comparePaintDevices(errpoint, sel, sel2)) {
        sel2->convertToQImage(0, 0, 0, 200, 200).save("merge_visitor6.png");
        QFAIL(QString("Failed to copy selection, first different pixel: %1,%2 ")
              .arg(errpoint.x())
              .arg(errpoint.y())
              .toAscii());
    }
}

QTEST_KDEMAIN(KisSelectionTest, NoGUI)
#include "kis_selection_test.moc"


