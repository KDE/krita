/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pixel_selection_test.h"
#include <QTest>


#include <kis_debug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_pixel_selection.h"
#include <testutil.h>
#include <testimage.h>
#include "kis_fill_painter.h"
#include "kis_transaction.h"
#include "kis_surrogate_undo_adapter.h"
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
    KisPixelSelectionSP selection =  image->globalSelection()->pixelSelection();
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
    KisPixelSelectionSP selection = image->globalSelection()->pixelSelection();
    selection->select(QRect(100, 50, 200, 100));
    QCOMPARE(selection->selectedExactRect(), QRect(100, 50, 200, 100));
}



void KisPixelSelectionTest::testUndo()
{
    KisPixelSelectionSP psel = new KisPixelSelection();

    {
        KisTransaction transaction(psel);
        psel->select(QRect(50, 50, 100, 100));
        transaction.end();
    }

    QCOMPARE(psel->selectedExactRect(), QRect(50, 50, 100, 100));

    {
        KisTransaction transaction(psel);
        psel->select(QRect(150, 50, 100, 100));
        transaction.end();
    }

    QCOMPARE(psel->selectedExactRect(), QRect(50, 50, 200, 100));

    {
        KisTransaction transaction(psel);
        psel->crop(QRect(75, 75, 10, 10));
        transaction.revert();
    }

    QCOMPARE(psel->selectedExactRect(), QRect(50, 50, 200, 100));
}

void KisPixelSelectionTest::testCrossColorSpacePainting()
{
    QRect r0(0,0,50,50);
    QRect r1(40,40,60,60);
    QRect r2(80,40,50,50);
    QRect r3(85,45,45,45);

    KisPixelSelectionSP psel1 = new KisPixelSelection();
    psel1->select(r0);

    const KoColorSpace *cs = psel1->compositionSourceColorSpace();

    KisPaintDeviceSP dev1 = new KisPaintDevice(cs);
    KisFixedPaintDeviceSP dev2 = new KisFixedPaintDevice(cs);
    KisFixedPaintDeviceSP dev3 = new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8());

    dev1->fill(r1, KoColor(Qt::white, cs));
    dev2->fill(r2.x(), r2.y(), r2.width(), r2.height() ,KoColor(Qt::white, cs).data());
    dev3->fill(r3.x(), r3.y(), r3.width(), r3.height() ,KoColor(Qt::white, cs).data());

    KisPainter painter(psel1);

    painter.bitBlt(r1.topLeft(), dev1, r1);
    QCOMPARE(psel1->selectedExactRect(), r0 | r1);

    painter.bltFixed(r2.x(), r2.y(), dev2, r2.x(), r2.y(), r2.width(), r2.height());
    QCOMPARE(psel1->selectedExactRect(), r0 | r1 | r2);

    psel1->clear();
    psel1->select(r0);

    painter.bitBltWithFixedSelection(r3.x(), r3.y(), dev1, dev3, r3.x(), r3.y(), r3.x(), r3.y(), r3.width(), r3.height());
    QCOMPARE(psel1->selectedExactRect(), r0 | (r1 & r3));

    psel1->clear();
    psel1->select(r0);

    painter.bltFixedWithFixedSelection(r3.x(), r3.y(), dev2, dev3, r3.x(), r3.y(), r3.x(), r3.y(), r3.width(), r3.height());
    QCOMPARE(psel1->selectedExactRect(), r0 | (r2 & r3));

    psel1->clear();
    psel1->select(r0);

    painter.fill(r3.x(), r3.y(), r3.width(), r3.height(), KoColor(Qt::white, cs));
    QCOMPARE(psel1->selectedExactRect(), r0 | r3);
}

void KisPixelSelectionTest::testOutlineCache()
{
    KisPixelSelectionSP psel1 = new KisPixelSelection();
    KisPixelSelectionSP psel2 = new KisPixelSelection();

    QVERIFY(psel1->outlineCacheValid());
    QVERIFY(psel2->outlineCacheValid());

    psel1->select(QRect(10,10,90,90), 100);
    QVERIFY(psel1->outlineCacheValid());
    QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,90,90));

    psel2->select(QRect(20,20,100,100), 200);
    QVERIFY(psel2->outlineCacheValid());
    QCOMPARE(psel2->outlineCache().boundingRect(), QRectF(20,20,100,100));

    psel1->applySelection(psel2, SELECTION_ADD);
    QVERIFY(psel1->outlineCacheValid());
    QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,110,110));

    psel1->applySelection(psel2, SELECTION_INTERSECT);
    QVERIFY(psel1->outlineCacheValid());
    QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(20,20,100,100));

    psel2->invalidateOutlineCache();
    QVERIFY(!psel2->outlineCacheValid());

    psel1->applySelection(psel2, SELECTION_SUBTRACT);
    QVERIFY(!psel1->outlineCacheValid());

    psel1->clear();
    QVERIFY(psel1->outlineCacheValid());
}

void KisPixelSelectionTest::testOutlineCacheTransactions()
{
    KisSurrogateUndoAdapter undoAdapter;
    KisPixelSelectionSP psel1 = new KisPixelSelection();

    QVERIFY(psel1->outlineCacheValid());

    psel1->clear();
    psel1->select(QRect(10,10,90,90), 100);
    QVERIFY(psel1->outlineCacheValid());
    QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,90,90));

    {
        KisTransaction t(psel1);
        t.end();
        QVERIFY(!psel1->outlineCacheValid());
    }

    psel1->clear();
    psel1->select(QRect(10,10,90,90), 100);
    QVERIFY(psel1->outlineCacheValid());
    QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,90,90));

    {
        KisTransaction t(psel1);
        t.revert();
        QVERIFY(psel1->outlineCacheValid());
        QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,90,90));
    }

    psel1->clear();
    psel1->select(QRect(10,10,90,90), 100);
    QVERIFY(psel1->outlineCacheValid());
    QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,90,90));

    {
        KisSelectionTransaction t(psel1);

        QVERIFY(psel1->outlineCacheValid());
        QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,90,90));

        psel1->select(QRect(10,10,200,200));

        QVERIFY(psel1->outlineCacheValid());
        QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,200,200));

        t.commit(&undoAdapter);

        QVERIFY(psel1->outlineCacheValid());
        QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,200,200));

        undoAdapter.undo();

        QVERIFY(psel1->outlineCacheValid());
        QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,90,90));

        undoAdapter.redo();

        QVERIFY(psel1->outlineCacheValid());
        QCOMPARE(psel1->outlineCache().boundingRect(), QRectF(10,10,200,200));
    }
}

#include "kis_paint_device_debug_utils.h"
#include <sdk/tests/testing_timed_default_bounds.h>

bool compareRect(KisPixelSelectionSP psel, const QRect &rc)
{
    QPolygon poly;
    poly << rc.topLeft();
    poly << QPoint(rc.x(), rc.bottom() + 1);
    poly << QPoint(rc.right() + 1, rc.bottom() + 1);
    poly << QPoint(rc.right() + 1, rc.top());
    poly << rc.topLeft();

    psel->select(rc);
    //KIS_DUMP_DEVICE_2(psel, psel->defaultBounds()->bounds(), "selection_rect", "dd");

    const QVector<QPolygon> outline = psel->outline();
    const QVector<QPolygon> ref({poly});

    const bool result = outline == ref;

    if (!result) {
        qDebug() << "Failed rect" << rc;
        qDebug() << "Exp: " << ref;
        qDebug() << "Act: " << outline;
    }

    psel->clear();

    return result;
}

bool compareRegion(KisPixelSelectionSP psel,
                   const QVector<QRect> &rects,
                   const QVector<QPolygon> &ref)
{
    Q_FOREACH(const QRect &rc, rects) {
        psel->select(rc);
    }

    //KIS_DUMP_DEVICE_2(psel, psel->defaultBounds()->bounds(), "selection_poly", "dd");

    const QVector<QPolygon> outline = psel->outline();

    const bool result = outline == ref;

    if (!result) {
        qDebug() << "Failed rect" << rects;
        qDebug() << "Exp: " << ref;
        qDebug() << "Act: " << outline;
    }

    psel->clear();

    return result;
}

void KisPixelSelectionTest::testOutlineArtifacts()
{
    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(QRect(0,0,20,22));
    KisPixelSelectionSP psel = new KisPixelSelection();
    psel->setDefaultBounds(bounds);

    QVERIFY(compareRect(psel, QRect(10,10,1,4)));
    QVERIFY(compareRect(psel, QRect(10,10,2,4)));
    QVERIFY(compareRect(psel, QRect(10,10,4,1)));
    QVERIFY(compareRect(psel, QRect(10,10,4,2)));
    QVERIFY(compareRect(psel, QRect(10,10,1,1)));


    QVERIFY(compareRegion(psel,
        {QRect(10,10,5,4), QRect(10,15,5,4), QRect(13,14,1,1)},
        {QPolygon({QPoint(10,10),
                   QPoint(10,14),
                   QPoint(13,14),
                   QPoint(13,15),
                   QPoint(10,15),
                   QPoint(10,19),
                   QPoint(15,19),
                   QPoint(15,15),
                   QPoint(14,15),
                   QPoint(14,14),
                   QPoint(15,14),
                   QPoint(15,10),
                   QPoint(10,10)})}));

    QVERIFY(compareRegion(psel,
        {QRect(10,10,5,4), QRect(10,16,5,4), QRect(12,14,2,2)},
        {QPolygon({QPoint(10,10),
                   QPoint(10,14),
                   QPoint(12,14),
                   QPoint(12,16),
                   QPoint(10,16),
                   QPoint(10,20),
                   QPoint(15,20),
                   QPoint(15,16),
                   QPoint(14,16),
                   QPoint(14,14),
                   QPoint(15,14),
                   QPoint(15,10),
                   QPoint(10,10)})}));

    QVERIFY(compareRegion(psel,
        {QRect(10,10,4,5), QRect(15,10,4,5), QRect(14,13,1,1)},
        {QPolygon({QPoint(10,10),
                   QPoint(10,15),
                   QPoint(14,15),
                   QPoint(14,14),
                   QPoint(15,14),
                   QPoint(15,15),
                   QPoint(19,15),
                   QPoint(19,10),
                   QPoint(15,10),
                   QPoint(15,13),
                   QPoint(14,13),
                   QPoint(14,10),
                   QPoint(10,10)})}));

    QVERIFY(compareRegion(psel,
        {QRect(10,10,3,5), QRect(15,10,4,5), QRect(13,13,2,1)},
        {QPolygon({QPoint(10,10),
                   QPoint(10,15),
                   QPoint(13,15),
                   QPoint(13,14),
                   QPoint(15,14),
                   QPoint(15,15),
                   QPoint(19,15),
                   QPoint(19,10),
                   QPoint(15,10),
                   QPoint(15,13),
                   QPoint(13,13),
                   QPoint(13,10),
                   QPoint(10,10)})}));

    QVERIFY(compareRegion(psel,
        {QRect(10,10,1,4), QRect(15,10,1,4)},
        {QPolygon({QPoint(10,10),
                   QPoint(10,14),
                   QPoint(11,14),
                   QPoint(11,10),
                   QPoint(10,10)}),
         QPolygon({QPoint(15,10),
                   QPoint(15,14),
                   QPoint(16,14),
                   QPoint(16,10),
                   QPoint(15,10)})}));

    QVERIFY(compareRegion(psel,
        {QRect(10,10,4,1), QRect(15,10,4,1)},
        {QPolygon({QPoint(10,10),
                            QPoint(10,11),
                            QPoint(14,11),
                            QPoint(14,10),
                            QPoint(10,10)}),
         QPolygon({QPoint(15,10),
                            QPoint(15,11),
                            QPoint(19,11),
                            QPoint(19,10),
                            QPoint(15,10)})}));

    QVERIFY(compareRegion(psel,
        {QRect(10,10,4,1), QRect(14,11,4,1)},
        {QPolygon({QPoint(10,10),
                   QPoint(10,11),
                   QPoint(14,11),
                   QPoint(14,12),
                   QPoint(18,12),
                   QPoint(18,11),
                   QPoint(14,11),
                   QPoint(14,10),
                   QPoint(10,10)})}));

    QVERIFY(compareRegion(psel,
        {QRect(0,0,10,22), QRect(15,0,5,22),
         QRect(10,0,5,10), QRect(10,15,5,7)},
        {QPolygon({QPoint(0,0),
                   QPoint(0,22),
                   QPoint(20,22),
                   QPoint(20,0),
                   QPoint(0,0)}),
         QPolygon({QPoint(10,10),
                   QPoint(15,10),
                   QPoint(15,15),
                   QPoint(10,15),
                   QPoint(10,10)})}));

    QVERIFY(compareRegion(psel,
        {QRect(0,0,20,5), QRect(0,10,20,12),
         QRect(5,5,15,5)},
        {QPolygon({QPoint(0,0),
                   QPoint(0,5),
                   QPoint(5,5),
                   QPoint(5,10),
                   QPoint(0,10),
                   QPoint(0,22),
                   QPoint(20,22),
                   QPoint(20,0),
                   QPoint(0,0)})}));

    QVERIFY(compareRegion(psel,
        {QRect(0,0,20,5), QRect(0,10,20,12),
         QRect(0,5,15,5)},
        {QPolygon({QPoint(0,0),
                   QPoint(0,22),
                   QPoint(20,22),
                   QPoint(20,10),
                   QPoint(15,10),
                   QPoint(15,5),
                   QPoint(20,5),
                   QPoint(20,0),
                   QPoint(0,0)})}));
}

KISTEST_MAIN(KisPixelSelectionTest)

