/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_shape_layer_test.h"

#include <QTest>

#include "kis_global.h"

#include "kis_shape_layer.h"
#include <KoPathShape.h>
#include <KoColorBackground.h>
#include "testutil.h"

#include <KisPart.h>
#include <KisDocument.h>

#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_merge_strategy_registry.h>

#include "kis_filter_strategy.h"

#include "kis_layer_utils.h"

#include <sdk/tests/testutil.h>

void testMergeDownImpl(bool useImageTransformations)
{
    const QString testName = useImageTransformations ? "scale_and_merge_down" : "merge_down";

    using namespace TestUtil;

    ReferenceImageChecker chk(testName, "shape_layer_test", ReferenceImageChecker::InternalStorage);
    chk.setMaxFailingPixels(10);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    const QRect refRect(0,0,64,64);
    MaskParent p(refRect);

    const qreal resolution = 72.0 / 72.0;
    p.image->setResolution(resolution, resolution);

    doc->setCurrentImage(p.image);


    KisShapeLayerSP shapeLayer1 = new KisShapeLayer(doc->shapeController(), p.image, "shapeLayer1", 150);

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(5, 5));
        path->lineTo(QPointF(5, 55));
        path->lineTo(QPointF(20, 55));
        path->lineTo(QPointF(20,  5));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::red)));

        path->setName("shape1");
        path->setZIndex(1);
        shapeLayer1->addShape(path);
    }

    KisShapeLayerSP shapeLayer2 = new KisShapeLayer(doc->shapeController(), p.image, "shapeLayer2", 255);

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(15, 10));
        path->lineTo(QPointF(15, 55));
        path->lineTo(QPointF(50, 55));
        path->lineTo(QPointF(50,  10));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::green)));

        path->setName("shape2");
        path->setZIndex(1);
        shapeLayer2->addShape(path);
    }

    p.image->addNode(shapeLayer1);
    p.image->addNode(shapeLayer2);
    shapeLayer1->setDirty();
    shapeLayer2->setDirty();

    p.waitForImageAndShapeLayers();

    QCOMPARE(int(p.image->root()->childCount()), 3);

    chk.checkImage(p.image, "00_initial_layer_update");

    if (useImageTransformations) {

        KisFilterStrategy *strategy = new KisBilinearFilterStrategy();
        p.image->scaleImage(QSize(32, 32), p.image->xRes(), p.image->yRes(), strategy);
        p.waitForImageAndShapeLayers();

        chk.checkImage(p.image, "01_after_scale_down");
    }

    p.image->mergeDown(shapeLayer2, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
    p.waitForImageAndShapeLayers();

    QCOMPARE(int(p.image->root()->childCount()), 2);

    KisShapeLayer *newShapeLayer = dynamic_cast<KisShapeLayer*>(p.image->root()->lastChild().data());
    QVERIFY(newShapeLayer);

    QVERIFY(newShapeLayer != shapeLayer1.data());
    QVERIFY(newShapeLayer != shapeLayer2.data());

    chk.checkImage(p.image, "02_after_merge_down");

    QVERIFY(chk.testPassed());
}

void KisShapeLayerTest::testMergeDown()
{
    testMergeDownImpl(false);
}

void KisShapeLayerTest::testScaleAndMergeDown()
{
    testMergeDownImpl(true);
}

namespace {
KoPathShape* createSimpleShape(int zIndex)
{
    KoPathShape* path = new KoPathShape();
    path->setShapeId(KoPathShapeId);
    path->moveTo(QPointF(15, 10));
    path->lineTo(QPointF(15, 55));
    path->lineTo(QPointF(50, 55));
    path->lineTo(QPointF(50,  10));
    path->close();
    path->normalize();
    path->setBackground(toQShared(new KoColorBackground(Qt::green)));

    path->setName(QString("shape_%1").arg(zIndex));
    path->setZIndex(zIndex);

    return path;
}
}

#include "commands/KoShapeReorderCommand.h"
#include <limits>

void testMergingShapeZIndexesImpl(int firstIndexStart,
                                  int firstIndexStep,
                                  int firstIndexSize,
                                  int secondIndexStart,
                                  int secondIndexStep,
                                  int secondIndexSize)
{
    QList<KoShape *> shapesBelow;
    QList<KoShape *> shapesAbove;

    qDebug() << "Test zIndex merge:";
    qDebug() << "    " << ppVar(firstIndexStart) << ppVar(firstIndexStep) << ppVar(firstIndexSize);
    qDebug() << "    " << ppVar(secondIndexStart) << ppVar(secondIndexStep) << ppVar(secondIndexSize);


    for (int i = 0; i < firstIndexSize; i++) {
        shapesBelow.append(createSimpleShape(firstIndexStart + firstIndexStep * i));
    }

    for (int i = 0; i < secondIndexSize; i++) {
        shapesAbove.append(createSimpleShape(secondIndexStart + secondIndexStep * i));
    }

    QList<KoShapeReorderCommand::IndexedShape> shapes =
        KoShapeReorderCommand::mergeDownShapes(shapesBelow, shapesAbove);

    KoShapeReorderCommand cmd(shapes);
    cmd.redo();

    for (int i = 0; i < shapesBelow.size(); i++) {
        if (i > 0 && shapesBelow[i - 1]->zIndex() >= shapesBelow[i]->zIndex()) {
            qDebug()  << ppVar(i);
            qDebug() << ppVar(shapesBelow[i - 1]->zIndex()) << ppVar(shapesBelow[i]->zIndex());
            QFAIL("Shapes have wrong ordering after merge!");
        }
    }

    if (!shapesBelow.isEmpty() && !shapesAbove.isEmpty()) {
        if (shapesBelow.last()->zIndex() >= shapesAbove.first()->zIndex()) {
            qDebug() << ppVar(shapesBelow.last()->zIndex()) << ppVar(shapesAbove.first()->zIndex());
            QFAIL("Two shape groups have intersections after merge!");
        }
    }

    for (int i = 0; i < shapesAbove.size(); i++) {
        if (i > 0 && shapesAbove[i - 1]->zIndex() >= shapesAbove[i]->zIndex()) {
            qDebug()  << ppVar(i);
            qDebug() << ppVar(shapesAbove[i - 1]->zIndex()) << ppVar(shapesAbove[i]->zIndex());
            QFAIL("Shapes have wrong ordering after merge!");
        }
    }

}

void KisShapeLayerTest::testMergingShapeZIndexes()
{
    testMergingShapeZIndexesImpl(0, 1, 10,
                                 5, 1, 10);

    testMergingShapeZIndexesImpl(0, 1, 0,
                                 5, 1, 10);

    testMergingShapeZIndexesImpl(0, 1, 10,
                                 5, 1, 0);

    testMergingShapeZIndexesImpl(std::numeric_limits<qint16>::max() - 10, 1, 10,
                                 5, 1, 10);

    testMergingShapeZIndexesImpl(-32768, 1024, 64,
                                 0, 1024, 31);

    testMergingShapeZIndexesImpl(-32768+1023, 1024, 64,
                                 0, 1, 1024);
}

void KisShapeLayerTest::testCloneScaledLayer()
{
    using namespace TestUtil;

    ReferenceImageChecker chk("scale_and_clone", "shape_layer_test", ReferenceImageChecker::InternalStorage);
    chk.setMaxFailingPixels(10);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    const QRect refRect(0,0,64,64);
    MaskParent p(refRect);

    const qreal resolution = 72.0 / 72.0;
    p.image->setResolution(resolution, resolution);

    doc->setCurrentImage(p.image);


    KisShapeLayerSP shapeLayer1 = new KisShapeLayer(doc->shapeController(), p.image, "shapeLayer1", 150);

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(5, 5));
        path->lineTo(QPointF(5, 55));
        path->lineTo(QPointF(20, 55));
        path->lineTo(QPointF(20,  5));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::red)));

        path->setName("shape1");
        path->setZIndex(1);
        shapeLayer1->addShape(path);
    }

    p.image->addNode(shapeLayer1);
    shapeLayer1->setDirty();

    p.waitForImageAndShapeLayers();

    QCOMPARE(int(p.image->root()->childCount()), 2);

    chk.checkImage(p.image, "00_initial_layer_update");

    {

        KisFilterStrategy *strategy = new KisBilinearFilterStrategy();
        p.image->scaleImage(QSize(32, 32), p.image->xRes(), p.image->yRes(), strategy);
        p.waitForImageAndShapeLayers();

        chk.checkImage(p.image, "01_after_scale_down");
    }

    KisNodeSP clonedLayer = shapeLayer1->clone();

    p.image->removeNode(shapeLayer1);
    p.image->addNode(clonedLayer);
    clonedLayer->setDirty();

    p.waitForImageAndShapeLayers();

    QCOMPARE(int(p.image->root()->childCount()), 2);
    chk.checkImage(p.image, "01_after_scale_down");

    QVERIFY(chk.testPassed());
}

KISTEST_MAIN(KisShapeLayerTest)
