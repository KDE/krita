/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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

#include "TestShapePainting.h"

#include <QtGui>
#include "KoShapeContainer.h"
#include "KoShapeManager.h"
#include "KoShapePaintingContext.h"
#include "KoViewConverter.h"
#include <sdk/tests/kistest.h>
#include <MockShapes.h>

#include <QTest>

void TestShapePainting::testPaintShape()
{
    MockShape *shape1 = new MockShape();
    MockShape *shape2 = new MockShape();
    QScopedPointer<MockContainer> container(new MockContainer());

    container->addShape(shape1);
    container->addShape(shape2);
    QCOMPARE(shape1->parent(), container.data());
    QCOMPARE(shape2->parent(), container.data());
    container->setClipped(shape1, false);
    container->setClipped(shape2, false);
    QCOMPARE(container->isClipped(shape1), false);
    QCOMPARE(container->isClipped(shape2), false);

    MockCanvas canvas;
    KoShapeManager manager(&canvas);
    manager.addShape(container.data());
    QCOMPARE(manager.shapes().count(), 3);

    QImage image(100, 100,  QImage::Format_Mono);
    QPainter painter(&image);
    KoViewConverter vc;
    manager.paint(painter, vc, false);

    // with the shape not being clipped, the shapeManager will paint it for us.
    QCOMPARE(shape1->paintedCount, 1);
    QCOMPARE(shape2->paintedCount, 1);
    QCOMPARE(container->paintedCount, 1);

    // the container should thus not paint the shape
    shape1->paintedCount = 0;
    shape2->paintedCount = 0;
    container->paintedCount = 0;
    KoShapePaintingContext paintContext;
    container->paint(painter, vc, paintContext);
    QCOMPARE(shape1->paintedCount, 0);
    QCOMPARE(shape2->paintedCount, 0);
    QCOMPARE(container->paintedCount, 1);


    container->setClipped(shape1, false);
    container->setClipped(shape2, true);
    QCOMPARE(container->isClipped(shape1), false);
    QCOMPARE(container->isClipped(shape2), true);

    shape1->paintedCount = 0;
    shape2->paintedCount = 0;
    container->paintedCount = 0;
    manager.paint(painter, vc, false);


    // with this shape not being clipped, the shapeManager will paint the container and this shape
    QCOMPARE(shape1->paintedCount, 1);
    // with this shape being clipped, the container will paint it for us.
    QCOMPARE(shape2->paintedCount, 1);
    QCOMPARE(container->paintedCount, 1);
}

void TestShapePainting::testPaintHiddenShape()
{
    QScopedPointer<MockContainer> top(new MockContainer());

    MockShape *shape = new MockShape();
    MockContainer *fourth = new MockContainer();
    MockContainer *thirth = new MockContainer();
    MockContainer *second = new MockContainer();


    top->addShape(second);
    second->addShape(thirth);
    thirth->addShape(fourth);
    fourth->addShape(shape);

    second->setVisible(false);

    MockCanvas canvas;
    KoShapeManager manager(&canvas);
    manager.addShape(top.data());
    QCOMPARE(manager.shapes().count(), 5);

    QImage image(100, 100,  QImage::Format_Mono);
    QPainter painter(&image);
    KoViewConverter vc;
    manager.paint(painter, vc, false);

    QCOMPARE(top->paintedCount, 1);
    QCOMPARE(second->paintedCount, 0);
    QCOMPARE(thirth->paintedCount, 0);
    QCOMPARE(fourth->paintedCount, 0);
    QCOMPARE(shape->paintedCount, 0);
}

void TestShapePainting::testPaintOrder()
{
    // the stacking order determines the painting order so things on top
    // get their paint called last.
    // Each shape has a zIndex and within the children a container has
    // it determines the stacking order. Its important to realize that
    // the zIndex is thus local to a container, if you have layer1 and layer2
    // with both various child shapes the stacking order of the layer shapes
    // is most important, then within this the child shape index is used.

    class OrderedMockShape : public MockShape {
    public:
        OrderedMockShape(QList<MockShape*> &list) : order(list) {}
        void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext) override {
            order.append(this);
            MockShape::paint(painter, converter, paintcontext);
        }
        QList<MockShape*> &order;
    };

    QList<MockShape*> order;

    {
        QScopedPointer<MockContainer> top(new MockContainer());
        top->setZIndex(2);
        OrderedMockShape *shape1 = new OrderedMockShape(order);
        shape1->setZIndex(5);
        OrderedMockShape *shape2 = new OrderedMockShape(order);
        shape2->setZIndex(0);
        top->addShape(shape1);
        top->addShape(shape2);

        QScopedPointer<MockContainer> bottom(new MockContainer());
        bottom->setZIndex(1);
        OrderedMockShape *shape3 = new OrderedMockShape(order);
        shape3->setZIndex(-1);
        OrderedMockShape *shape4 = new OrderedMockShape(order);
        shape4->setZIndex(9);
        bottom->addShape(shape3);
        bottom->addShape(shape4);

        MockCanvas canvas;
        KoShapeManager manager(&canvas);
        manager.addShape(top.data());
        manager.addShape(bottom.data());
        QCOMPARE(manager.shapes().count(), 6);

        QImage image(100, 100,  QImage::Format_Mono);
        QPainter painter(&image);
        KoViewConverter vc;
        manager.paint(painter, vc, false);
        QCOMPARE(top->paintedCount, 1);
        QCOMPARE(bottom->paintedCount, 1);
        QCOMPARE(shape1->paintedCount, 1);
        QCOMPARE(shape2->paintedCount, 1);
        QCOMPARE(shape3->paintedCount, 1);
        QCOMPARE(shape4->paintedCount, 1);

        QCOMPARE(order.count(), 4);
        QVERIFY(order[0] == shape3); // lowest first
        QVERIFY(order[1] == shape4);
        QVERIFY(order[2] == shape2);
        QVERIFY(order[3] == shape1);

        // again, with clipping.
        order.clear();
        painter.setClipRect(0, 0, 100, 100);
        manager.paint(painter, vc, false);
        QCOMPARE(top->paintedCount, 2);
        QCOMPARE(bottom->paintedCount, 2);
        QCOMPARE(shape1->paintedCount, 2);
        QCOMPARE(shape2->paintedCount, 2);
        QCOMPARE(shape3->paintedCount, 2);
        QCOMPARE(shape4->paintedCount, 2);

        QCOMPARE(order.count(), 4);
        QVERIFY(order[0] == shape3); // lowest first
        QVERIFY(order[1] == shape4);
        QVERIFY(order[2] == shape2);
        QVERIFY(order[3] == shape1);

    }

    order.clear();

    {
        QScopedPointer<MockContainer> root(new MockContainer());
        root->setZIndex(0);

        MockContainer *branch1 = new MockContainer();
        branch1->setZIndex(1);
        OrderedMockShape *child1_1 = new OrderedMockShape(order);
        child1_1->setZIndex(1);
        OrderedMockShape *child1_2 = new OrderedMockShape(order);
        child1_2->setZIndex(2);
        branch1->addShape(child1_1);
        branch1->addShape(child1_2);

        MockContainer *branch2 = new MockContainer();
        branch2->setZIndex(2);
        OrderedMockShape *child2_1 = new OrderedMockShape(order);
        child2_1->setZIndex(1);
        OrderedMockShape *child2_2 = new OrderedMockShape(order);
        child2_2->setZIndex(2);
        branch2->addShape(child2_1);
        branch2->addShape(child2_2);

        root->addShape(branch1);
        root->addShape(branch2);

        QList<KoShape*> sortedShapes;
        sortedShapes.append(root.data());
        sortedShapes.append(branch1);
        sortedShapes.append(branch2);
        sortedShapes.append(branch1->shapes());
        sortedShapes.append(branch2->shapes());

        std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);
        QCOMPARE(sortedShapes.count(), 7);
        QVERIFY(sortedShapes[0] == root.data());
        QVERIFY(sortedShapes[1] == branch1);
        QVERIFY(sortedShapes[2] == child1_1);
        QVERIFY(sortedShapes[3] == child1_2);
        QVERIFY(sortedShapes[4] == branch2);
        QVERIFY(sortedShapes[5] == child2_1);
        QVERIFY(sortedShapes[6] == child2_2);
    }
}
#include <kundo2command.h>
#include <KoShapeController.h>
#include <KoShapeGroupCommand.h>
#include <KoShapeUngroupCommand.h>
#include "kis_debug.h"
void TestShapePainting::testGroupUngroup()
{
    QScopedPointer<MockContainer> shapesFakeLayer(new MockContainer);
    MockShape *shape1(new MockShape());
    MockShape *shape2(new MockShape());
    shape1->setName("shape1");
    shape2->setName("shape2");
    shape1->setParent(shapesFakeLayer.data());
    shape2->setParent(shapesFakeLayer.data());

    QList<KoShape*> groupedShapes = {shape1, shape2};


    MockShapeController controller;
    MockCanvas canvas(&controller);
    KoShapeManager *manager = canvas.shapeManager();

    controller.addShape(shape1);
    controller.addShape(shape2);

    QImage image(100, 100,  QImage::Format_Mono);
    QPainter painter(&image);
    painter.setClipRect(image.rect());
    KoViewConverter vc;

    for (int i = 0; i < 3; i++) {
        KoShapeGroup *group = new KoShapeGroup();
        group->setParent(shapesFakeLayer.data());

        {
            group->setName("group");

            KUndo2Command groupingCommand;
            canvas.shapeController()->addShapeDirect(group, 0, &groupingCommand);
            new KoShapeGroupCommand(group, groupedShapes, true, &groupingCommand);

            groupingCommand.redo();

            manager->paint(painter, vc, false);

            QCOMPARE(shape1->paintedCount, 2 * i + 1);
            QCOMPARE(shape2->paintedCount, 2 * i + 1);
            QCOMPARE(manager->shapes().size(), 3);
        }

        {
            KUndo2Command ungroupingCommand;

            new KoShapeUngroupCommand(group, group->shapes(), QList<KoShape*>(), &ungroupingCommand);
            canvas.shapeController()->removeShape(group, &ungroupingCommand);
            // NOTE: group will be deleted in ungroupingCommand's d-tor

            ungroupingCommand.redo();

            manager->paint(painter, vc, false);

            QCOMPARE(shape1->paintedCount, 2 * i + 2);
            QCOMPARE(shape2->paintedCount, 2 * i + 2);
            QCOMPARE(manager->shapes().size(), 2);
        }
    }
}

KISTEST_MAIN(TestShapePainting)
