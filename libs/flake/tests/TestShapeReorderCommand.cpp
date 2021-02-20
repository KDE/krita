/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestShapeReorderCommand.h"
#include <MockShapes.h>
#include <KoShapeReorderCommand.h>
#include <KoShapeManager.h>

#include <QTest>

TestShapeReorderCommand::TestShapeReorderCommand()
{
}

TestShapeReorderCommand::~TestShapeReorderCommand()
{
}

void TestShapeReorderCommand::testZIndexSorting()
{
    MockShape shape1;
    MockShape shape2;
    MockShape shape3;
    MockShape shape4;
    MockShape shape5;

    shape1.setZIndex(-2);
    shape2.setZIndex(5);
    shape3.setZIndex(0);
    shape4.setZIndex(9999);
    shape5.setZIndex(-9999);

    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);
    shapes.append(&shape4);
    shapes.append(&shape5);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    QCOMPARE(shapes.indexOf(&shape1), 1);
    QCOMPARE(shapes.indexOf(&shape2), 3);
    QCOMPARE(shapes.indexOf(&shape3), 2);
    QCOMPARE(shapes.indexOf(&shape4), 4);
    QCOMPARE(shapes.indexOf(&shape5), 0);
}

void TestShapeReorderCommand::testRunThroughSorting()
{
    MockShape shape1;
    MockShape shape2;
    MockShape shape3;
    MockShape shape4;
    MockShape shape5;

    shape1.setZIndex(-2);
    shape2.setZIndex(5);
    shape3.setZIndex(0);
    shape4.setZIndex(9999);
    shape5.setZIndex(-9999);

    shape2.setTextRunAroundSide(KoShape::RunThrough, KoShape::Background);
    shape3.setTextRunAroundSide(KoShape::RunThrough, KoShape::Foreground);

    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);
    shapes.append(&shape4);
    shapes.append(&shape5);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    QCOMPARE(shapes.indexOf(&shape1), 2);
    QCOMPARE(shapes.indexOf(&shape2), 0);
    QCOMPARE(shapes.indexOf(&shape3), 4);
    QCOMPARE(shapes.indexOf(&shape4), 3);
    QCOMPARE(shapes.indexOf(&shape5), 1);
}

void TestShapeReorderCommand::testParentChildSorting()
{
    MockShape *shape1 = new MockShape();
    MockShape *shape2 = new MockShape();
    MockShape *shape3 = new MockShape();
    MockShape *shape4 = new MockShape();
    MockShape *shape5 = new MockShape();
    MockShape *shape6 = new MockShape();
    MockShape *shape7 = new MockShape();
    MockContainer *container1 = new MockContainer();
    MockContainer *container2 = new MockContainer();
    MockContainer *container3 = new MockContainer();

    shape1->setZIndex(-2);
    shape2->setZIndex(5);
    shape3->setZIndex(0);
    shape4->setZIndex(9999);
    shape5->setZIndex(-9999);
    shape6->setZIndex(3);
    shape7->setZIndex(7);
    container1->setZIndex(-55);
    container2->setZIndex(57);

    shape2->setTextRunAroundSide(KoShape::RunThrough, KoShape::Background);
    shape3->setTextRunAroundSide(KoShape::RunThrough, KoShape::Foreground);
    container1->setTextRunAroundSide(KoShape::RunThrough, KoShape::Foreground);

    container1->addShape(shape1);
    //container1.addShape(&shape2); //we shouldn't parent combine fg and bg
    container2->addShape(shape4);
    container2->addShape(shape5);
    container1->addShape(container2);
    container1->addShape(container3);

    QList<KoShape*> shapes;
    shapes.append(shape1);
    shapes.append(shape2);
    shapes.append(shape3);
    shapes.append(shape4);
    shapes.append(shape5);
    shapes.append(shape6);
    shapes.append(shape7);
    shapes.append(container1);
    shapes.append(container2);
    shapes.append(container3);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

/* This is the expected result
s3  0 fg
  s4  9999
  s5 -9999
 c2  57
 c3  0
 s1 -2
c1 -55 fg

s7  7
s6  3

s2  5 bg
*/

    QCOMPARE(shapes.indexOf(shape1), 4);
    QCOMPARE(shapes.indexOf(shape2), 0);
    QCOMPARE(shapes.indexOf(shape3), 9);
    QCOMPARE(shapes.indexOf(shape4), 8);
    QCOMPARE(shapes.indexOf(shape5), 7);
    QCOMPARE(shapes.indexOf(shape6), 1);
    QCOMPARE(shapes.indexOf(shape7), 2);
    QCOMPARE(shapes.indexOf(container1), 3);
    QCOMPARE(shapes.indexOf(container2), 6);
    QCOMPARE(shapes.indexOf(container3), 5);

    delete container1;
    delete shape2;
    delete shape3;
    delete shape6;
    delete shape7;
}

void TestShapeReorderCommand::testBringToFront()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize(QSizeF(100, 100));
    shape1.setZIndex(1);
    shape2.setSize(QSizeF(100, 100));
    shape2.setZIndex(2);
    shape3.setSize(QSizeF(100, 100));
    shape3.setZIndex(3);
    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape1), 0);
    QCOMPARE(shapes.indexOf(&shape2), 1);
    QCOMPARE(shapes.indexOf(&shape3), 2);

    QList<KoShape*> selectedShapes;
    selectedShapes.append(&shape1);

    KUndo2Command * cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::BringToFront);
    cmd->redo();

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape2), 0);
    QCOMPARE(shapes.indexOf(&shape3), 1);
    QCOMPARE(shapes.indexOf(&shape1), 2);

    delete cmd;
}

void TestShapeReorderCommand::testSendToBack()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize(QSizeF(100, 100));
    shape1.setZIndex(1);
    shape2.setSize(QSizeF(100, 100));
    shape2.setZIndex(2);
    shape3.setSize(QSizeF(100, 100));
    shape3.setZIndex(3);
    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape1), 0);
    QCOMPARE(shapes.indexOf(&shape2), 1);
    QCOMPARE(shapes.indexOf(&shape3), 2);

    QList<KoShape*> selectedShapes;
    selectedShapes.append(&shape3);

    KUndo2Command * cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::SendToBack);
    cmd->redo();

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape3), 0);
    QCOMPARE(shapes.indexOf(&shape1), 1);
    QCOMPARE(shapes.indexOf(&shape2), 2);

    delete cmd;
}

void TestShapeReorderCommand::testMoveUp()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize(QSizeF(100, 100));
    shape1.setZIndex(1);
    shape2.setSize(QSizeF(100, 100));
    shape2.setZIndex(2);
    shape3.setSize(QSizeF(100, 100));
    shape3.setZIndex(3);
    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape1), 0);
    QCOMPARE(shapes.indexOf(&shape2), 1);
    QCOMPARE(shapes.indexOf(&shape3), 2);

    QList<KoShape*> selectedShapes;
    selectedShapes.append(&shape1);

    KUndo2Command * cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::RaiseShape);
    cmd->redo();

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape2), 0);
    QCOMPARE(shapes.indexOf(&shape1), 1);
    QCOMPARE(shapes.indexOf(&shape3), 2);

    delete cmd;
}

void TestShapeReorderCommand::testMoveDown()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize(QSizeF(100, 100));
    shape1.setZIndex(1);
    shape2.setSize(QSizeF(100, 100));
    shape2.setZIndex(2);
    shape3.setSize(QSizeF(100, 100));
    shape3.setZIndex(3);
    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape1), 0);
    QCOMPARE(shapes.indexOf(&shape2), 1);
    QCOMPARE(shapes.indexOf(&shape3), 2);

    QList<KoShape*> selectedShapes;
    selectedShapes.append(&shape2);

    KUndo2Command * cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::LowerShape);
    cmd->redo();

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape2), 0);
    QCOMPARE(shapes.indexOf(&shape1), 1);
    QCOMPARE(shapes.indexOf(&shape3), 2);

    delete cmd;
}

void TestShapeReorderCommand::testMoveUpOverlapping()
{
    MockShape shape1, shape2, shape3, shape4, shape5;

    shape1.setSize(QSizeF(100, 100));
    shape1.setZIndex(1);
    shape2.setSize(QSizeF(100, 100));
    shape2.setZIndex(2);

    shape3.setSize(QSizeF(300, 300));
    shape3.setZIndex(3);

    shape4.setSize(QSizeF(100, 100));
    shape4.setPosition(QPointF(200,200));
    shape4.setZIndex(4);
    shape5.setSize(QSizeF(100, 100));
    shape5.setPosition(QPointF(200,200));
    shape5.setZIndex(5);

    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);
    shapes.append(&shape4);
    shapes.append(&shape5);

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes);

    QVERIFY(shape1.zIndex() < shape2.zIndex());
    QVERIFY(shape2.zIndex() < shape3.zIndex());
    QVERIFY(shape3.zIndex() < shape4.zIndex());
    QVERIFY(shape4.zIndex() < shape5.zIndex());

    QList<KoShape*> selectedShapes;
    selectedShapes.append(&shape1);

    KUndo2Command * cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::RaiseShape);
    cmd->redo();
    delete cmd;

    QVERIFY(shape1.zIndex() > shape2.zIndex());
    QVERIFY(shape2.zIndex() < shape3.zIndex());
    QVERIFY(shape1.zIndex() < shape3.zIndex());
    QVERIFY(shape3.zIndex() < shape4.zIndex());
    QVERIFY(shape4.zIndex() < shape5.zIndex());
}

void TestShapeReorderCommand::testMoveDownOverlapping()
{
#if 0 // disable a current algorithm does not yet support this
    MockShape shape1, shape2, shape3, shape4, shape5;

    shape1.setSize(QSizeF(100, 100));
    shape1.setZIndex(1);
    shape2.setSize(QSizeF(100, 100));
    shape2.setZIndex(2);

    shape3.setSize(QSizeF(300, 300));
    shape3.setZIndex(3);

    shape4.setSize(QSizeF(100, 100));
    shape4.setPosition(QPointF(200,200));
    shape4.setZIndex(4);
    shape5.setSize(QSizeF(100, 100));
    shape5.setPosition(QPointF(200,200));
    shape5.setZIndex(5);

    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);
    shapes.append(&shape4);
    shapes.append(&shape5);

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes);

    QVERIFY(shape1.zIndex() < shape2.zIndex());
    QVERIFY(shape2.zIndex() < shape3.zIndex());
    QVERIFY(shape3.zIndex() < shape4.zIndex());
    QVERIFY(shape4.zIndex() < shape5.zIndex());

    QList<KoShape*> selectedShapes;
    selectedShapes.append(&shape5);

    KUndo2Command * cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::LowerShape);
    cmd->redo();
    delete cmd;

    QVERIFY(shape1.zIndex() < shape2.zIndex());
    QVERIFY(shape2.zIndex() < shape3.zIndex());
    QVERIFY(shape3.zIndex() < shape4.zIndex());
    QVERIFY(shape4.zIndex() > shape5.zIndex());
    QVERIFY(shape3.zIndex() > shape5.zIndex());
#endif
}

void TestShapeReorderCommand::testSendToBackChildren()
{
    MockShape *shape1 = new MockShape();
    MockShape *shape2 = new MockShape();
    MockShape *shape3 = new MockShape();

    shape1->setSize(QSizeF(100, 100));
    shape1->setZIndex(1);
    shape2->setSize(QSizeF(100, 100));
    shape2->setZIndex(2);
    shape3->setSize(QSizeF(100, 100));
    shape3->setZIndex(3);

    QScopedPointer<MockContainer> container(new MockContainer());
    container->addShape(shape1);
    container->addShape(shape2);
    container->addShape(shape3);

    QList<KoShape*> shapes;
    shapes.append(shape1);
    shapes.append(shape2);
    shapes.append(shape3);
    shapes.append(container.data());

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(container.data()), 0); // atm the parent is always lower than its children
    QCOMPARE(shapes.indexOf(shape1), 1);
    QCOMPARE(shapes.indexOf(shape2), 2);
    QCOMPARE(shapes.indexOf(shape3), 3);

    QList<KoShape*> selectedShapes;
    selectedShapes.append(shape3);

    KUndo2Command * cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::SendToBack);
    cmd->redo();
    delete cmd;

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(container.data()), 0); // atm the parent is always lower than its children
    QCOMPARE(shapes.indexOf(shape3), 1);
    QVERIFY(shape3->zIndex() < shape1->zIndex());
    QCOMPARE(shapes.indexOf(shape1), 2);
    QVERIFY(shape1->zIndex() < shape2->zIndex());
    QCOMPARE(shapes.indexOf(shape2), 3);

    selectedShapes.clear();
    selectedShapes.append(shape2);

    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::SendToBack);
    cmd->redo();
    delete cmd;

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(container.data()), 0); // atm the parent is always lower than its children
    QCOMPARE(shapes.indexOf(shape2), 1);
    QVERIFY(shape2->zIndex() < shape3->zIndex());
    QCOMPARE(shapes.indexOf(shape3), 2);
    QVERIFY(shape3->zIndex() < shape1->zIndex());
    QCOMPARE(shapes.indexOf(shape1), 3);

    selectedShapes.clear();
    selectedShapes.append(shape1);

    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::SendToBack);
    cmd->redo();
    delete cmd;

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(container.data()), 0); // atm the parent is always lower than its children
    QCOMPARE(shapes.indexOf(shape1), 1);
    QVERIFY(shape1->zIndex() < shape2->zIndex());
    QCOMPARE(shapes.indexOf(shape2), 2);
    QVERIFY(shape2->zIndex() < shape3->zIndex());
    QCOMPARE(shapes.indexOf(shape3), 3);
}

void TestShapeReorderCommand::testNoCommand()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize(QSizeF(100, 100));
    shape1.setZIndex(1);
    shape2.setSize(QSizeF(100, 100));
    shape2.setZIndex(2);
    shape3.setSize(QSizeF(100, 100));
    shape3.setZIndex(3);
    QList<KoShape*> shapes;
    shapes.append(&shape1);
    shapes.append(&shape2);
    shapes.append(&shape3);

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE(shapes.indexOf(&shape1), 0);
    QCOMPARE(shapes.indexOf(&shape2), 1);
    QCOMPARE(shapes.indexOf(&shape3), 2);

    QList<KoShape*> selectedShapes;
    selectedShapes.append(&shape3);

    KUndo2Command * cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::BringToFront);
    QVERIFY(cmd == 0);

    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::RaiseShape);
    QVERIFY(cmd == 0);

    selectedShapes.append(&shape1);
    selectedShapes.append(&shape2);
    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::BringToFront);
    QVERIFY(cmd == 0);

    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::RaiseShape);
    QVERIFY(cmd == 0);

    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::LowerShape);
    QVERIFY(cmd == 0);

    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::SendToBack);
    QVERIFY(cmd == 0);

    selectedShapes.clear();
    selectedShapes.append(&shape1);

    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::SendToBack);
    QVERIFY(cmd == 0);

    cmd = KoShapeReorderCommand::createCommand(selectedShapes, &manager, KoShapeReorderCommand::LowerShape);
    QVERIFY(cmd == 0);
}
#include <kis_assert.h>
#include <kis_debug.h>
void testMergeInShapeImpl(const QVector<int> indexesProfile,
                          int newShapeIndex,
                          const QVector<qint16> expectedIndexes)
{
    KIS_ASSERT(indexesProfile.size() == expectedIndexes.size());

    QVector<MockShape> shapesStore(indexesProfile.size());

    QList<KoShape*> managedShapes;

    for (int i = 0; i < shapesStore.size(); i++) {
        shapesStore[i].setSize(QSizeF(100,100));
        shapesStore[i].setZIndex(indexesProfile[i]);

        managedShapes << &shapesStore[i];
    }

    QScopedPointer<KUndo2Command> cmd(
        KoShapeReorderCommand::mergeInShape(managedShapes, &shapesStore[newShapeIndex]));
    cmd->redo();

    for (int i = 0; i < shapesStore.size(); i++) {
        //qDebug() << ppVar(i) << ppVar(shapesStore[i].zIndex());
        QCOMPARE(shapesStore[i].zIndex(), expectedIndexes[i]);
    }
}

void TestShapeReorderCommand::testMergeInShape()
{
    QVector<int> indexesProfile({1,1,2,2,2,3,3,4,5,6});
    int newShapeIndex = 3;
    QVector<qint16> expectedIndexes({1,1,2,3,2,4,4,5,6,7});

    testMergeInShapeImpl(indexesProfile, newShapeIndex, expectedIndexes);
}

void TestShapeReorderCommand::testMergeInShapeDistant()
{
    QVector<int> indexesProfile({1,1,2,2,2,4,4,5,6,7});
    int newShapeIndex = 3;
    QVector<qint16> expectedIndexes({1,1,2,3,2,4,4,5,6,7});

    testMergeInShapeImpl(indexesProfile, newShapeIndex, expectedIndexes);
}

QTEST_MAIN(TestShapeReorderCommand)
