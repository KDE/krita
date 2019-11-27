/*
 *  Copyright (c) 2008 Sven Langkamp <sven.langkamp@gmail.com>
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
#include "testutil.h"
#include "kistest.h"
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
    QVERIFY(selection->hasPixelSelection() == false);
    QVERIFY(selection->hasShapeSelection() == false);

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
    selection->setShapeSelection(shapeSelection);
    shapeSelection->addShape(shape);

    QVERIFY(selection->hasShapeSelection());

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
    QCOMPARE(selection->hasPixelSelection(), false);
    QCOMPARE(selection->hasShapeSelection(), false);

    selection->setParentNode(image->root());

    KisPixelSelectionSP pixelSelection = selection->pixelSelection();
    pixelSelection->select(QRect(0, 0, 100, 100));

    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 25, 25), MAX_SELECTED);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));

    QTransform matrix;
    matrix.scale(1 / image->xRes(), 1 / image->yRes());
    const QRectF srcRect1(50, 50, 100, 100);
    const QRectF rect1 = matrix.mapRect(srcRect1);

    KisShapeSelection * shapeSelection = new KisShapeSelection(doc->shapeController(), image, selection);
    selection->setShapeSelection(shapeSelection);

    KoPathShape *shape1 = createRectangularShape(rect1);
    shapeSelection->addShape(shape1);

    QVERIFY(selection->hasShapeSelection());

    selection->pixelSelection()->clear();
    QCOMPARE(selection->selectedExactRect(), QRect());

    selection->updateProjection();
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), srcRect1.toRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), srcRect1);
    QCOMPARE(selection->hasShapeSelection(), true);

    KisTransaction t1(selection->pixelSelection());
    selection->pixelSelection()->clear();
    KUndo2Command *cmd1 = t1.endAndTake();

    QTest::qWait(400);
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), QRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), QRectF());
    QCOMPARE(selection->hasShapeSelection(), false);

    const QRectF srcRect2(10, 10, 20, 20);
    const QRectF rect2 = matrix.mapRect(srcRect2);
    KoPathShape *shape2 = createRectangularShape(rect2);
    shapeSelection->addShape(shape2);

    QTest::qWait(400);
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), srcRect2.toRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), srcRect2);
    QCOMPARE(selection->hasShapeSelection(), true);

    shapeSelection->removeShape(shape2);

    QTest::qWait(400);
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), QRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), QRectF());
    QCOMPARE(selection->hasShapeSelection(), false);

    cmd1->undo();

    QTest::qWait(400);
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), srcRect1.toRect());
    QCOMPARE(selection->outlineCacheValid(), true);
    QCOMPARE(selection->outlineCache().boundingRect(), srcRect1);
    QCOMPARE(selection->hasShapeSelection(), true);
}


KISTEST_MAIN(KisShapeSelectionTest)


