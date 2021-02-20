/*
 *  SPDX-FileCopyrightText: 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_shape_selection_test.h"
#include <QTest>

#include <kis_debug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoPathShape.h>

#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "flake/kis_shape_selection.h"
#include "kis_image.h"
#include <testutil.h>
#include "testui.h"
#include <KisPart.h>
#include <KisDocument.h>
#include "kis_transaction.h"

void KisShapeSelectionTest::testAddChild()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    QColor qc(Qt::white);
    qc.setAlpha(0);
    KoColor bgColor(qc, cs);
    doc->newImage("test", 300, 300, cs, bgColor, KisConfig::CANVAS_COLOR, 1, "test", 100);
    KisImageSP image = doc->image();

    KisSelectionSP selection = new KisSelection();
    QVERIFY(!selection->hasNonEmptyPixelSelection());
    QVERIFY(!selection->hasNonEmptyShapeSelection());

    KisPixelSelectionSP pixelSelection = selection->pixelSelection();
    pixelSelection->select(QRect(0, 0, 100, 100));

    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 25, 25), MAX_SELECTED);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));

    QRectF rect(50, 50, 100, 100);
    QTransform matrix;
    matrix.scale(1 / image->xRes(), 1 / image->yRes());
    rect = matrix.mapRect(rect);

    KoPathShape* shape = new KoPathShape();
    shape->setShapeId(KoPathShapeId);
    shape->moveTo(rect.topLeft());
    shape->lineTo(rect.topRight());
    shape->lineTo(rect.bottomRight());
    shape->lineTo(rect.bottomLeft());
    shape->close();

    KisShapeSelection * shapeSelection = new KisShapeSelection(doc->shapeController(), image, selection);
    selection->convertToVectorSelectionNoUndo(shapeSelection);
    shapeSelection->addShape(shape);

    QVERIFY(selection->hasNonEmptyShapeSelection());

    selection->updateProjection();
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), QRect(50, 50, 100, 100));
}

KoPathShape *createRectangularShape(const QRectF &rect)
{
    KoPathShape* shape = new KoPathShape();
    shape->setShapeId(KoPathShapeId);
    shape->moveTo(rect.topLeft());
    shape->lineTo(rect.topRight());
    shape->lineTo(rect.bottomRight());
    shape->lineTo(rect.bottomLeft());
    shape->close();

    return shape;
}

void KisShapeSelectionTest::testUndoFlattening()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    KoColor bgColor(QColor(255, 255, 255, 0), cs);
    doc->newImage("test", 300, 300, cs, bgColor, KisConfig::CANVAS_COLOR, 1, "test", 100);
    KisImageSP image = doc->image();

    QCOMPARE(image->locked(), false);

    KisSelectionSP selection = new KisSelection();
    QCOMPARE(selection->hasNonEmptyPixelSelection(), false);
    QCOMPARE(selection->hasNonEmptyShapeSelection(), false);

    selection->setParentNode(image->root());

    KisPixelSelectionSP pixelSelection = selection->pixelSelection();
    pixelSelection->select(QRect(0, 0, 100, 100));

    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 25, 25), MAX_SELECTED);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));

    QTransform matrix;
    matrix.scale(1 / image->xRes(), 1 / image->yRes());
    const QRectF srcRect1(50, 50, 100, 100);
    const QRectF rect1 = matrix.mapRect(srcRect1);

    KisShapeSelection * shapeSelection1 = new KisShapeSelection(doc->shapeController(), image, selection);
    selection->convertToVectorSelectionNoUndo(shapeSelection1);

    KoPathShape *shape1 = createRectangularShape(rect1);
    shapeSelection1->addShape(shape1);

    QVERIFY(selection->hasNonEmptyShapeSelection());

    selection->pixelSelection()->clear();
    QCOMPARE(selection->selectedExactRect(), QRect());

    selection->updateProjection();
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), srcRect1.toRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), srcRect1);
    QCOMPARE(selection->hasNonEmptyShapeSelection(), true);

    KisSelectionTransaction t1(selection->pixelSelection());
    selection->pixelSelection()->clear();
    KUndo2Command *cmd1 = t1.endAndTake();
    cmd1->redo(); // first redo

    QTest::qWait(400);
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), QRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), QRectF());
    QCOMPARE(selection->hasNonEmptyShapeSelection(), false);

    const QRectF srcRect2(10, 10, 20, 20);
    const QRectF rect2 = matrix.mapRect(srcRect2);
    KoPathShape *shape2 = createRectangularShape(rect2);

    KisShapeSelection * shapeSelection2 = new KisShapeSelection(doc->shapeController(), image, selection);
    KUndo2Command *cmd2 = selection->convertToVectorSelection(shapeSelection2);
    cmd2->redo(); // first redo

    shapeSelection2->addShape(shape2);

    QTest::qWait(400);
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), srcRect2.toRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), srcRect2);
    QCOMPARE(selection->hasNonEmptyShapeSelection(), true);

    shapeSelection1->removeShape(shape2);

    QTest::qWait(400);
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), QRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), QRectF());
    QCOMPARE(selection->hasNonEmptyShapeSelection(), false);

    cmd2->undo();
    cmd1->undo();

    QTest::qWait(400);
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), srcRect1.toRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), srcRect1);
    QCOMPARE(selection->hasNonEmptyShapeSelection(), true);

    delete cmd2;
    delete cmd1;
    QTest::qWait(400);

}

#include "kis_paint_device_debug_utils.h"

void KisShapeSelectionTest::testHistoryOnFlattening()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    KoColor bgColor(QColor(255, 255, 255, 0), cs);
    doc->newImage("test", 300, 300, cs, bgColor, KisConfig::CANVAS_COLOR, 1, "test", 100);
    KisImageSP image = doc->image();

    QCOMPARE(image->locked(), false);

    KisSelectionSP selection = new KisSelection();
    QCOMPARE(selection->hasNonEmptyPixelSelection(), false);
    QCOMPARE(selection->hasNonEmptyShapeSelection(), false);

    selection->setParentNode(image->root());

    KisPixelSelectionSP pixelSelection = selection->pixelSelection();

    KisSelectionTransaction t0(pixelSelection);
    pixelSelection->select(QRect(70, 70, 180, 20));
    QScopedPointer<KUndo2Command> cmd0(t0.endAndTake());
    cmd0->redo(); // first redo

    KisSelectionTransaction t1(pixelSelection);
    pixelSelection->clear();
    pixelSelection->select(QRect(0, 0, 100, 100));
    QScopedPointer<KUndo2Command> cmd1(t1.endAndTake());
    cmd1->redo(); // first redo

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "00_0pixel", "dd");
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 25, 25), MAX_SELECTED);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));

    QTransform matrix;
    matrix.scale(1 / image->xRes(), 1 / image->yRes());
    const QRectF srcRect1(50, 50, 100, 100);
    const QRectF rect1 = matrix.mapRect(srcRect1);

    KisShapeSelection * shapeSelection = new KisShapeSelection(doc->shapeController(), image, selection);

    QScopedPointer<KUndo2Command> cmd2(selection->convertToVectorSelection(shapeSelection));
    cmd2->redo();

    QVERIFY(!selection->hasNonEmptyShapeSelection());
    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "00_1converted", "dd");
    QCOMPARE(selection->selectedExactRect(), QRect());

    KoPathShape *shape1 = createRectangularShape(rect1);
    shapeSelection->addShape(shape1);

    QVERIFY(selection->hasNonEmptyShapeSelection());
    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "01_vector", "dd");
    QCOMPARE(selection->selectedExactRect(), QRect(50, 50, 100, 100));

    KisSelectionTransaction flatteningTransaction(pixelSelection);
    pixelSelection->select(QRect(80, 80, 100, 83));

    QScopedPointer<KUndo2Command> cmd3(flatteningTransaction.endAndTake());
    cmd3->redo(); // first redo!

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "02_flattened", "dd");
    QCOMPARE(selection->selectedExactRect(), QRect(50, 50, 130, 113));
    QVERIFY(!selection->hasNonEmptyShapeSelection());

    cmd3->undo();
    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "03_undo_flattening", "dd");
    QCOMPARE(selection->selectedExactRect(), QRect(50, 50, 100, 100));
    QVERIFY(selection->hasNonEmptyShapeSelection());

    cmd3->redo();
    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "04_redo_flattening", "dd");
    QCOMPARE(selection->selectedExactRect(), QRect(50, 50, 130, 113));
    QVERIFY(!selection->hasNonEmptyShapeSelection());

    cmd3->undo();
    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "05_2ndundo_flattening", "dd");
    QCOMPARE(selection->selectedExactRect(), QRect(50, 50, 100, 100));
    QVERIFY(selection->hasNonEmptyShapeSelection());

    shapeSelection->removeShape(shape1);

    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "06_undo_add_shape", "dd");
    QVERIFY(selection->shapeSelection());
    QVERIFY(!selection->hasNonEmptyShapeSelection());
    QCOMPARE(selection->selectedExactRect(), QRect());

    cmd2->undo();

    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "07_undo_conversion", "dd");
    QVERIFY(!selection->shapeSelection());
    QVERIFY(!selection->hasNonEmptyShapeSelection());
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));

    cmd1->undo();

    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "08_undo_initial_paint", "dd");
    QVERIFY(!selection->shapeSelection());
    QVERIFY(!selection->hasNonEmptyShapeSelection());
    QCOMPARE(selection->selectedExactRect(), QRect(70, 70, 180, 20));

    cmd0->undo();

    QTest::qWait(200);
    image->waitForDone();

    // KIS_DUMP_DEVICE_2(pixelSelection, QRect(0,0,300,300), "09_undo_zero_paint", "dd");
    QVERIFY(!selection->shapeSelection());
    QVERIFY(!selection->hasNonEmptyShapeSelection());
    QCOMPARE(selection->selectedExactRect(), QRect());
}


KISTEST_MAIN(KisShapeSelectionTest)


