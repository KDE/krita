/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_shape_commands_test.h"

#include <QTest>

#include "kis_global.h"

#include "kis_shape_layer.h"
#include <KoPathShape.h>
#include <KoColorBackground.h>
#include "testutil.h"

#include <KisPart.h>
#include <KisDocument.h>

#include <KoShapeGroup.h>
#include <KoShapeGroupCommand.h>
#include <sdk/tests/testutil.h>

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
    KoFlake::resizeShape(group, 1.2, 1.4, stillPoint, false, true, QTransform());

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

KISTEST_MAIN(KisShapeCommandsTest)
