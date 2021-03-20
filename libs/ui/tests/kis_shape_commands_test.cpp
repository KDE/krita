/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_shape_commands_test.h"

#include <simpletest.h>

#include "kis_global.h"

#include "kis_shape_layer.h"
#include <KoPathShape.h>
#include <KoColorBackground.h>
#include <testutil.h>

#include <KisPart.h>
#include <KisDocument.h>

#include <KoShapeStroke.h>
#include <KoShapeGroup.h>
#include <KoShapeGroupCommand.h>
#include <sdk/tests/testutil.h>
#include <sdk/tests/testui.h>

void KisShapeCommandsTest::testGrouping()
{
    TestUtil::ReferenceImageChecker chk("grouping", "shape_commands_test");

    QRect refRect(0,0,64,64);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    TestUtil::MaskParent p(refRect);

    const qreal resolution = 72.0 / 72.0;
    p.image->setResolution(resolution, resolution);

    doc->setCurrentImage(p.image);


    KisShapeLayerSP shapeLayer = new KisShapeLayer(doc->shapeController(), p.image, "shapeLayer1", 75);

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(5, 5));
        path->lineTo(QPointF(5, 55));
        path->lineTo(QPointF(55, 55));
        path->lineTo(QPointF(55,  5));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::red)));

        path->setName("shape1");
        path->setZIndex(1);
        shapeLayer->addShape(path);
    }

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(30, 30));
        path->lineTo(QPointF(30, 60));
        path->lineTo(QPointF(60, 60));
        path->lineTo(QPointF(60, 30));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::green)));

        path->setName("shape2");
        path->setZIndex(2);
        shapeLayer->addShape(path);
    }

    p.image->addNode(shapeLayer);

    shapeLayer->setDirty();
    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    QList<KoShape*> shapes = shapeLayer->shapes();

    KoShapeGroup *group = new KoShapeGroup();
    group->setName("group_shape");
    shapeLayer->addShape(group);

    QScopedPointer<KoShapeGroupCommand> cmd(
        new KoShapeGroupCommand(group, shapes, true));

    cmd->redo();

    shapeLayer->setDirty();
    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    cmd->undo();

    shapeLayer->setDirty();
    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    QVERIFY(chk.testPassed());

}

void KisShapeCommandsTest::testResizeShape(bool normalizeGroup)
{
    TestUtil::ReferenceImageChecker chk("resize_shape", "shape_commands_test");

    QRect refRect(0,0,64,64);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    TestUtil::MaskParent p(refRect);

    const qreal resolution = 72.0 / 72.0;
    p.image->setResolution(resolution, resolution);

    doc->setCurrentImage(p.image);

    KisShapeLayerSP shapeLayer = new KisShapeLayer(doc->shapeController(), p.image, "shapeLayer1", 75);

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(5, 5));
        path->lineTo(QPointF(5, 55));
        path->lineTo(QPointF(55, 55));
        path->lineTo(QPointF(55,  5));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::red)));

        path->setName("shape1");
        path->setZIndex(1);
        shapeLayer->addShape(path);
    }

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(30, 30));
        path->lineTo(QPointF(30, 60));
        path->lineTo(QPointF(60, 60));
        path->lineTo(QPointF(60, 30));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::green)));

        path->setName("shape2");
        path->setZIndex(2);
        shapeLayer->addShape(path);
    }

    p.image->addNode(shapeLayer);

    shapeLayer->setDirty();
    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    QList<KoShape*> shapes = shapeLayer->shapes();

    KoShapeGroup *group = new KoShapeGroup();
    group->setName("group_shape");
    shapeLayer->addShape(group);

    QScopedPointer<KoShapeGroupCommand> cmd(
        new KoShapeGroupCommand(group, shapes, normalizeGroup));

    cmd->redo();

    shapeLayer->setDirty();
    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    qDebug() << "Before:";
    qDebug() << ppVar(group->absolutePosition(KoFlake::TopLeft));
    qDebug() << ppVar(group->absolutePosition(KoFlake::BottomRight));
    qDebug() << ppVar(group->outlineRect());
    qDebug() << ppVar(group->transformation());

    QCOMPARE(group->absolutePosition(KoFlake::TopLeft), QPointF(5,5));
    QCOMPARE(group->absolutePosition(KoFlake::BottomRight), QPointF(60,60));


    const QPointF stillPoint = group->absolutePosition(KoFlake::BottomRight);
    KoFlake::resizeShapeCommon(group, 1.2, 1.4, stillPoint, false, true, QTransform());

    qDebug() << "After:";
    qDebug() << ppVar(group->absolutePosition(KoFlake::TopLeft));
    qDebug() << ppVar(group->absolutePosition(KoFlake::BottomRight));
    qDebug() << ppVar(group->outlineRect());
    qDebug() << ppVar(group->transformation());

    QCOMPARE(group->absolutePosition(KoFlake::TopLeft), QPointF(-6,-17));
    QCOMPARE(group->absolutePosition(KoFlake::BottomRight), QPointF(60,60));
}

void KisShapeCommandsTest::testResizeShape()
{
    testResizeShape(false);
}

void KisShapeCommandsTest::testResizeShapeNormalized()
{
    testResizeShape(true);
}

namespace {
struct ShapeWrapper
{
    ShapeWrapper() {
        group.reset(new KoShapeGroup());

        path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(10, 10));
        path->lineTo(QPointF(110, 10));
        path->setStroke(toQShared(new KoShapeStroke()));
        path->setName("shape1");
        path->setZIndex(1);
        group->addShape(path);

        QCOMPARE(path->outlineRect(), QRectF(10,10,100,0));
    }

    void rotate90cw() {
        QTransform t;
        t.rotate(90);
        path->setTransformation(t);
        QCOMPARE(path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    QScopedPointer<KoShapeGroup> group;
    KoPathShape *path = 0;

};
}

void KisShapeCommandsTest::testResizeNullShape()
{
    {
        ShapeWrapper w;
        qDebug() << "Normal resize mode, X, 200%, top-left";
        KoFlake::resizeShape(w.path, 2.0, 1.0, QPointF(10,10), false);
        QCOMPARE(w.path->outlineRect(), QRectF(20, 10, 200, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 200, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Normal resize mode, X, 200%, top-right";
        KoFlake::resizeShape(w.path, 2.0, 1.0, QPointF(110,10), false);
        QCOMPARE(w.path->outlineRect(), QRectF(20, 10, 200, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-90, 10, 200, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Normal resize mode, X, 200%, outside-x";
        KoFlake::resizeShape(w.path, 2.0, 1.0, QPointF(120,10), false);
        QCOMPARE(w.path->outlineRect(), QRectF(20, 10, 200, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-100, 10, 200, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Normal resize mode, X, 200%, outside-y";
        KoFlake::resizeShape(w.path, 2.0, 1.0, QPointF(110,20), false);
        QCOMPARE(w.path->outlineRect(), QRectF(20, 10, 200, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-90, 10, 200, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Normal resize mode, Y, 200%, top-left";
        KoFlake::resizeShape(w.path, 1.0, 2.0, QPointF(10,10), false);
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 100, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Normal resize mode, Y, 200%, top-right";
        KoFlake::resizeShape(w.path, 1.0, 2.0, QPointF(110,10), false);
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 100, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Normal resize mode, Y, 200%, outside-x";
        KoFlake::resizeShape(w.path, 1.0, 2.0, QPointF(120,10), false);
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 100, 0));
    }

    {
        // TODO: perhaps wrong? (though this combination is not used atm)
        ShapeWrapper w;
        qDebug() << "Normal resize mode, Y, 200%, outside-y";
        KoFlake::resizeShape(w.path, 1.0, 2.0, QPointF(110,20), false);
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 100, 0));
    }
}

void KisShapeCommandsTest::testResizeNullShapeGlobal()
{
    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global resize mode (not scale), X, 200%, top-left";
        KoFlake::resizeShape(w.path, 2.0, 1.0, QPointF(-10,10), true);
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global resize mode (not scale), X, 200%, top-right";
        KoFlake::resizeShape(w.path, 2.0, 1.0, QPointF(-10,110), true);
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global resize mode (not scale), X, 200%, outside-x";
        KoFlake::resizeShape(w.path, 2.0, 1.0, QPointF(-10,120), true);
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global resize mode (not scale), X, 200%, outside-y";
        KoFlake::resizeShape(w.path, 2.0, 1.0, QPointF(-20,110), true);
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));

        // TODO: perhaps wrong? (though this combination is not used atm)
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global resize mode (not scale), Y, 200%, top-left";
        KoFlake::resizeShape(w.path, 1.0, 2.0, QPointF(-10,10), true);
        QCOMPARE(w.path->outlineRect(), QRectF(20, 10, 200, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global resize mode (not scale), Y, 200%, top-right";
        KoFlake::resizeShape(w.path, 1.0, 2.0, QPointF(-10,110), true);
        QCOMPARE(w.path->outlineRect(), QRectF(20, 10, 200, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -90, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global resize mode (not scale), Y, 200%, outside-x";
        KoFlake::resizeShape(w.path, 1.0, 2.0, QPointF(-10,120), true);
        QCOMPARE(w.path->outlineRect(), QRectF(20, 10, 200, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -100, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global resize mode (not scale), Y, 200%, outside-y";
        KoFlake::resizeShape(w.path, 1.0, 2.0, QPointF(-20,110), true);
        QCOMPARE(w.path->outlineRect(), QRectF(20, 10, 200, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -90, 0, 200));
    }
}
void KisShapeCommandsTest::testScaleNullShape()
{
    {
        ShapeWrapper w;
        qDebug() << "Post-scaling mode, X, 200%, top-left";
        KoFlake::scaleShape(w.path, 2.0, 1.0, QPointF(10,10), QTransform());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 200, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Post-scaling mode, X, 200%, top-right";
        KoFlake::scaleShape(w.path, 2.0, 1.0, QPointF(110,10), QTransform());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-90, 10, 200, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Post-scaling mode, X, 200%, outside-x";
        KoFlake::scaleShape(w.path, 2.0, 1.0, QPointF(120,10), QTransform());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-100, 10, 200, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Post-scaling mode, X, 200%, outside-y";
        KoFlake::scaleShape(w.path, 2.0, 1.0, QPointF(110,20), QTransform());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-90, 10, 200, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Post-scaling mode, Y, 200%, top-left";
        KoFlake::scaleShape(w.path, 1.0, 2.0, QPointF(10,10), QTransform());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 100, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Post-scaling mode, Y, 200%, top-right";
        KoFlake::scaleShape(w.path, 1.0, 2.0, QPointF(110,10), QTransform());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 100, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Post-scaling mode, Y, 200%, outside-x";
        KoFlake::scaleShape(w.path, 1.0, 2.0, QPointF(120,10), QTransform());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 10, 100, 0));
    }

    {
        ShapeWrapper w;
        qDebug() << "Post-scaling mode, Y, 200%, outside-y";
        KoFlake::scaleShape(w.path, 1.0, 2.0, QPointF(110,20), QTransform());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(10, 0, 100, 0));
    }
}
void KisShapeCommandsTest::testScaleNullShapeCovered()
{
    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Post scale covered mode, X, 200%, top-left";
        KoFlake::scaleShape(w.path, 2.0, 1.0, QPointF(-10,10), w.path->transformation());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Post scale covered mode, X, 200%, top-right";
        KoFlake::scaleShape(w.path, 2.0, 1.0, QPointF(-10,110), w.path->transformation());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -90, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Post scale covered mode, X, 200%, outside-x";
        KoFlake::scaleShape(w.path, 2.0, 1.0, QPointF(-10,120), w.path->transformation());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -100, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Post scale covered mode, X, 200%, outside-y";
        KoFlake::scaleShape(w.path, 2.0, 1.0, QPointF(-20,110), w.path->transformation());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -90, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Post scale covered mode, Y, 200%, top-left";
        KoFlake::scaleShape(w.path, 1.0, 2.0, QPointF(-10,10), w.path->transformation());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Post scale covered mode, Y, 200%, top-right";
        KoFlake::scaleShape(w.path, 1.0, 2.0, QPointF(-10,110), w.path->transformation());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Post scale covered mode, Y, 200%, outside-x";
        KoFlake::scaleShape(w.path, 1.0, 2.0, QPointF(-10,120), w.path->transformation());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Post scale covered mode, Y, 200%, outside-y";
        KoFlake::scaleShape(w.path, 1.0, 2.0, QPointF(-20,110), w.path->transformation());
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(0, 10, 0, 100));
    }
}
void KisShapeCommandsTest::testScaleNullShapeGlobal()
{
    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global post-scale mode, X, 200%, top-left";
        KoFlake::scaleShapeGlobal(w.path, 2.0, 1.0, QPointF(-10,10));
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global post-scale mode, X, 200%, top-right";
        KoFlake::scaleShapeGlobal(w.path, 2.0, 1.0, QPointF(-10,110));
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global post-scale mode, X, 200%, outside-x";
        KoFlake::scaleShapeGlobal(w.path, 2.0, 1.0, QPointF(-10,120));
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global post-scale mode, X, 200%, outside-y";
        KoFlake::scaleShapeGlobal(w.path, 2.0, 1.0, QPointF(-20,110));
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(0, 10, 0, 100));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global post-scale mode, Y, 200%, top-left";
        KoFlake::scaleShapeGlobal(w.path, 1.0, 2.0, QPointF(-10,10));
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, 10, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global post-scale mode, Y, 200%, top-right";
        KoFlake::scaleShapeGlobal(w.path, 1.0, 2.0, QPointF(-10,110));
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -90, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global post-scale mode, Y, 200%, outside-x";
        KoFlake::scaleShapeGlobal(w.path, 1.0, 2.0, QPointF(-10,120));
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -100, 0, 200));
    }

    {
        ShapeWrapper w;
        w.rotate90cw();
        qDebug() << "Global post-scale mode, Y, 200%, outside-y";
        KoFlake::scaleShapeGlobal(w.path, 1.0, 2.0, QPointF(-20,110));
        QCOMPARE(w.path->outlineRect(), QRectF(10, 10, 100, 0));
        QCOMPARE(w.path->absoluteOutlineRect(), QRectF(-10, -90, 0, 200));
    }
}

KISTEST_MAIN(KisShapeCommandsTest)
