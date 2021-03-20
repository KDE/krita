/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestShapeGroupCommand.h"
#include "MockShapes.h"
#include <KoShapeGroup.h>
#include <KoShapeGroupCommand.h>
#include <KoShapeUngroupCommand.h>
#include <KoShapeStroke.h>
#include <KoShapeShadow.h>
#include <kundo2command.h>

#include <KoShapeReorderCommand.h>

#include "kis_pointer_utils.h"
#include "kis_algebra_2d.h"
#include "kis_debug.h"

#include <simpletest.h>

TestShapeGroupCommand::TestShapeGroupCommand()
        : toplevelGroup(0), sublevelGroup(0), strokeGroup(0)
        , cmd1(0), cmd2(0), strokeCmd(0)
        , toplevelShape1(0), toplevelShape2(0), toplevelShape3(0), toplevelShape4(0)
        , sublevelShape1(0), sublevelShape2(0)
        , extraShape1(0), extraShape2(0)
        , strokeShape1(0), strokeShape2(0)
{
}

TestShapeGroupCommand::~TestShapeGroupCommand()
{
}

MockShape* createMockShape(const QRectF &rect)
{
    MockShape *result = new MockShape();
    result->setAbsolutePosition(rect.topLeft(), KoFlake::TopLeft);
    result->setSize(rect.size());
    return result;
}

template <class Shape>
void removeShape(Shape* &shape) {
    if (!shape->parent()) {
        delete shape;
    }
    shape = 0;
}

void TestShapeGroupCommand::init()
{
    toplevelShape1 = createMockShape(QRectF(50, 50, 50, 50));
    toplevelShape2 = createMockShape(QRectF(50, 150, 50, 50));
    toplevelShape3 = createMockShape(QRectF(50, 250, 50, 50));
    toplevelShape4 = createMockShape(QRectF(50, 350, 50, 50));

    sublevelShape1 = createMockShape(QRectF(150, 150, 50, 50));
    sublevelShape2 = createMockShape(QRectF(250, 150, 50, 50));

    extraShape1 = createMockShape(QRectF(150, 50, 50, 50));
    extraShape2 = createMockShape(QRectF(250, 50, 50, 50));

    toplevelGroup = new KoShapeGroup();
    sublevelGroup = new KoShapeGroup();

    strokeShape1 = createMockShape(QRectF(0, 0, 50, 50));
    strokeShape2 = createMockShape(QRectF(25, 25, 50, 50));

    strokeGroup = new KoShapeGroup();
    strokeGroup->setStroke(toQShared(new KoShapeStroke(2.0f)));
    strokeGroup->setShadow(new KoShapeShadow());
}

void TestShapeGroupCommand::cleanup()
{
    removeShape(toplevelShape1);
    removeShape(toplevelShape2);
    removeShape(toplevelShape3);
    removeShape(toplevelShape4);

    removeShape(sublevelShape1);
    removeShape(sublevelShape2);

    removeShape(extraShape1);
    removeShape(extraShape2);

    removeShape(strokeShape1);
    removeShape(strokeShape2);

    removeShape(sublevelGroup);
    removeShape(toplevelGroup);

    delete cmd1;
    cmd1 = 0;

    delete cmd2;
    cmd2 = 0;

    delete strokeCmd;
    strokeCmd = 0;
}

void TestShapeGroupCommand::testToplevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;

    QCOMPARE(toplevelGroup->boundingRect(), QRectF());
    QCOMPARE(toplevelGroup->outlineRect(), QRectF());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));

    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes);
    cmd1->redo();

    QCOMPARE(toplevelShape1->parent(), toplevelGroup);
    QCOMPARE(toplevelShape2->parent(), toplevelGroup);

    QCOMPARE(toplevelGroup->boundingRect(), QRectF(50, 50, 50, 150));
    QCOMPARE(toplevelGroup->outlineRect(), QRectF(50, 50, 50, 150));
    QVERIFY(toplevelGroup->transformation().isIdentity());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape1->transformation(),
                                             QTransform::fromTranslate(50, 50),
                                             0.01));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape2->transformation(),
                                             QTransform::fromTranslate(50, 150),
                                             0.01));

    cmd1->undo();

    QVERIFY(!toplevelShape1->parent());
    QVERIFY(!toplevelShape2->parent());

    QCOMPARE(toplevelGroup->boundingRect(), QRectF());
    QCOMPARE(toplevelGroup->outlineRect(), QRectF());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));
}

void TestShapeGroupCommand::testToplevelGroupWithExistingTransform()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;

    toplevelGroup->setAbsolutePosition(QPointF(100,100), KoFlake::TopLeft);

    QCOMPARE(toplevelGroup->boundingRect(), QRectF());
    QCOMPARE(toplevelGroup->outlineRect(), QRectF());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));

    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes);
    cmd1->redo();

    QCOMPARE(toplevelShape1->parent(), toplevelGroup);
    QCOMPARE(toplevelShape2->parent(), toplevelGroup);

    QCOMPARE(toplevelGroup->boundingRect(), QRectF(50, 50, 50, 150));
    // NOTE: The shapes are now at the negative side of the group coordinate system!
    QCOMPARE(toplevelGroup->outlineRect(), QRectF(-50, -50, 50, 150));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelGroup->transformation(),
                                             QTransform::fromTranslate(100, 100),
                                             0.01));

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape1->transformation(),
                                             QTransform::fromTranslate(-50, -50),
                                             0.01));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape2->transformation(),
                                             QTransform::fromTranslate(-50, 50),
                                             0.01));

    cmd1->undo();
    QVERIFY(!toplevelShape1->parent());
    QVERIFY(!toplevelShape2->parent());

    QCOMPARE(toplevelGroup->boundingRect(), QRectF());
    QCOMPARE(toplevelGroup->outlineRect(), QRectF());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));
}

void TestShapeGroupCommand::testToplevelGroupTransformLater()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;

    QCOMPARE(toplevelGroup->boundingRect(), QRectF());
    QCOMPARE(toplevelGroup->outlineRect(), QRectF());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));

    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes);
    cmd1->redo();

    QCOMPARE(toplevelShape1->parent(), toplevelGroup);
    QCOMPARE(toplevelShape2->parent(), toplevelGroup);

    QCOMPARE(toplevelGroup->boundingRect(), QRectF(50, 50, 50, 150));
    QCOMPARE(toplevelGroup->outlineRect(), QRectF(50, 50, 50, 150));
    QVERIFY(toplevelGroup->transformation().isIdentity());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape1->transformation(),
                                             QTransform::fromTranslate(50, 50),
                                             0.01));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape2->transformation(),
                                             QTransform::fromTranslate(50, 150),
                                             0.01));

    toplevelGroup->applyAbsoluteTransformation(QTransform::fromTranslate(100, 100));

    QCOMPARE(toplevelGroup->boundingRect(), QRectF(150, 150, 50, 150));
    QCOMPARE(toplevelGroup->outlineRect(), QRectF(50, 50, 50, 150));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelGroup->transformation(),
                                             QTransform::fromTranslate(100, 100),
                                             0.01));

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(150, 150, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape1->transformation(),
                                             QTransform::fromTranslate(50, 50),
                                             0.01));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(150, 250, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape2->transformation(),
                                             QTransform::fromTranslate(50, 150),
                                             0.01));

}

void TestShapeGroupCommand::testToplevelGroupWithNormalization()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;

    QCOMPARE(toplevelGroup->boundingRect(), QRectF());
    QCOMPARE(toplevelGroup->outlineRect(), QRectF());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));

    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes, true);
    cmd1->redo();

    QCOMPARE(toplevelShape1->parent(), toplevelGroup);
    QCOMPARE(toplevelShape2->parent(), toplevelGroup);

    QCOMPARE(toplevelGroup->boundingRect(), QRectF(50, 50, 50, 150));
    QCOMPARE(toplevelGroup->outlineRect(), QRectF(0, 0, 50, 150));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelGroup->transformation(),
                                             QTransform::fromTranslate(50, 50),
                                             0.01));

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(toplevelShape1->transformation().isIdentity());

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(toplevelShape2->transformation(),
                                             QTransform::fromTranslate(0, 100),
                                             0.01));


    cmd1->undo();

    QVERIFY(!toplevelShape1->parent());
    QVERIFY(!toplevelShape2->parent());

    QCOMPARE(toplevelGroup->boundingRect(), QRectF());
    QCOMPARE(toplevelGroup->outlineRect(), QRectF());

    QCOMPARE(toplevelShape1->boundingRect(), QRectF(50, 50, 50, 50));
    QCOMPARE(toplevelShape1->outlineRect(), QRectF(0, 0, 50, 50));

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape2->outlineRect(), QRectF(0, 0, 50, 50));
}

KoShapeGroupCommand * createAddCommand(KoShapeContainer *container, const QList<KoShape *> &shapes, KUndo2Command *parent = 0)
{
    QList<KoShape*> orderedShapes(shapes);
    std::stable_sort(orderedShapes.begin(), orderedShapes.end(), KoShape::compareShapeZIndex);
    return new KoShapeGroupCommand(container, orderedShapes, false, parent);
}

void TestShapeGroupCommand::testZIndexFromNowhere()
{
    QList<KoShape*> toplevelShapes;

    toplevelShapes << toplevelShape1 << toplevelShape2 << toplevelShape3;

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 0);
    QCOMPARE(int(toplevelShape3->zIndex()), 0);
    QCOMPARE(int(toplevelShape4->zIndex()), 0);

    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes);
    cmd1->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 1);
    QCOMPARE(int(toplevelShape3->zIndex()), 2);
    QCOMPARE(int(toplevelShape4->zIndex()), 0);

    cmd2 = createAddCommand(toplevelGroup, {toplevelShape4});
    cmd2->redo();

    // the new shape is added to the top
    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 1);
    QCOMPARE(int(toplevelShape3->zIndex()), 2);
    QCOMPARE(int(toplevelShape4->zIndex()), 3);

    cmd2->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 1);
    QCOMPARE(int(toplevelShape3->zIndex()), 2);
    QCOMPARE(int(toplevelShape4->zIndex()), 0);

    cmd1->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 0);
    QCOMPARE(int(toplevelShape3->zIndex()), 0);
    QCOMPARE(int(toplevelShape4->zIndex()), 0);
}

void TestShapeGroupCommand::testZIndexFromNowhereReordered()
{
    toplevelShape1->setZIndex(10);
    toplevelShape2->setZIndex(11);
    toplevelShape3->setZIndex(12);
    toplevelShape4->setZIndex(13);


    QCOMPARE(int(toplevelShape1->zIndex()), 10);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13);

    // shapes are reordered!
    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, {toplevelShape4, toplevelShape3});
    cmd1->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 10);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13);

    // shapes are reordered!
    cmd2 = createAddCommand(toplevelGroup, {toplevelShape2, toplevelShape1});
    cmd2->redo();

    // the new shapes are added to the top in a sorted way
    QCOMPARE(int(toplevelShape1->zIndex()), 14); // corrected
    QCOMPARE(int(toplevelShape2->zIndex()), 15); // corrected
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13);

    cmd2->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 10);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13);

    cmd1->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 10);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13);
}

void TestShapeGroupCommand::testZIndexFromSameGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2 << toplevelShape3 << toplevelShape4;

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 0);
    QCOMPARE(int(toplevelShape3->zIndex()), 0);
    QCOMPARE(int(toplevelShape4->zIndex()), 0);

    // create initial group of 4 shapes
    QScopedPointer<KUndo2Command> cmd1(
        KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes));
    cmd1->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 1);
    QCOMPARE(int(toplevelShape3->zIndex()), 2);
    QCOMPARE(int(toplevelShape4->zIndex()), 3);

    // add two middle shapes to a subgroup
    QScopedPointer<KUndo2Command> cmd2(
        KoShapeGroupCommand::createCommand(sublevelGroup, {toplevelShape2, toplevelShape3}));
    cmd2->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 1);
    QCOMPARE(int(toplevelShape3->zIndex()), 2);
    QCOMPARE(int(toplevelShape4->zIndex()), 3);
    QCOMPARE(int(sublevelGroup->zIndex()), 2);


    // offset z-index of subgroup'ed shape to higher values
    QScopedPointer<KUndo2Command> cmd3;
    {
        QList<KoShapeReorderCommand::IndexedShape> indexedShapes({toplevelShape2, toplevelShape3});
        indexedShapes[0].zIndex = 11;
        indexedShapes[1].zIndex = 12;
        cmd3.reset(new KoShapeReorderCommand(indexedShapes));
    }
    cmd3->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 11); // changed!
    QCOMPARE(int(toplevelShape3->zIndex()), 12); // changed!
    QCOMPARE(int(toplevelShape4->zIndex()), 3);
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    // add the last shape and check it is **appended**
    QScopedPointer<KUndo2Command> cmd4(createAddCommand(sublevelGroup, {toplevelShape4}));
    cmd4->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13); // corrected!
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    // add a bit of poison to the toplevel group by setting z-index to higher values
    QScopedPointer<KUndo2Command> cmd5;
    {
        QList<KoShapeReorderCommand::IndexedShape> indexedShapes({toplevelShape1, sublevelGroup});
        indexedShapes[0].zIndex = 100;
        indexedShapes[1].zIndex = 101;
        cmd5.reset(new KoShapeReorderCommand(indexedShapes));
    }
    cmd5->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 100);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13);
    QCOMPARE(int(sublevelGroup->zIndex()), 101);


    // add the first shape and check it is prepended
    QScopedPointer<KUndo2Command> cmd6(createAddCommand(sublevelGroup, {toplevelShape1}));
    cmd6->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 100); // added and all the shapes are offset
    QCOMPARE(int(toplevelShape2->zIndex()), 101);
    QCOMPARE(int(toplevelShape3->zIndex()), 102);
    QCOMPARE(int(toplevelShape4->zIndex()), 103);
    QCOMPARE(int(sublevelGroup->zIndex()), 101);

    /*************************************************************************/
    /*   Start undoing stuff                                                 */
    /*************************************************************************/

    cmd6->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 100);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13);
    QCOMPARE(int(sublevelGroup->zIndex()), 101);

    cmd5->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13); // corrected!
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    cmd4->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 11); // changed!
    QCOMPARE(int(toplevelShape3->zIndex()), 12); // changed!
    QCOMPARE(int(toplevelShape4->zIndex()), 3);
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    cmd3->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 1);
    QCOMPARE(int(toplevelShape3->zIndex()), 2);
    QCOMPARE(int(toplevelShape4->zIndex()), 3);
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    cmd2->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 1);
    QCOMPARE(int(toplevelShape3->zIndex()), 2);
    QCOMPARE(int(toplevelShape4->zIndex()), 3);
    //QCOMPARE(int(sublevelGroup->zIndex()), 0); // TODO!!!

    cmd1->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 0);
    QCOMPARE(int(toplevelShape3->zIndex()), 0);
    QCOMPARE(int(toplevelShape4->zIndex()), 0);
    //QCOMPARE(int(sublevelGroup->zIndex()), 0); // TODO!!!
}

#include "KoShapeTransformCommand.h"

void TestShapeGroupCommand::testUngrouping()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2 << toplevelShape3 << toplevelShape4;

    // create initial group of 4 shapes
    QScopedPointer<KUndo2Command> cmd1(
        KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes));
    cmd1->redo();

    // add two middle shapes to a subgroup
    QScopedPointer<KUndo2Command> cmd2(
        KoShapeGroupCommand::createCommand(sublevelGroup, {toplevelShape2, toplevelShape3}));
    cmd2->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 1);
    QCOMPARE(int(toplevelShape3->zIndex()), 2);
    QCOMPARE(int(toplevelShape4->zIndex()), 3);
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    // offset z-index of subgroup'ed shape to higher values
    QScopedPointer<KUndo2Command> cmd3;
    {
        QList<KoShapeReorderCommand::IndexedShape> indexedShapes({toplevelShape2, toplevelShape3});
        indexedShapes[0].zIndex = 11;
        indexedShapes[1].zIndex = 12;
        cmd3.reset(new KoShapeReorderCommand(indexedShapes));
    }
    cmd3->redo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 11); // changed!
    QCOMPARE(int(toplevelShape3->zIndex()), 12); // changed!
    QCOMPARE(int(toplevelShape4->zIndex()), 3);
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    QCOMPARE(toplevelShape2->parent(), sublevelGroup);
    QCOMPARE(toplevelShape3->parent(), sublevelGroup);

    QCOMPARE(toplevelShape2->boundingRect(), QRectF(50, 150, 50, 50));
    QCOMPARE(toplevelShape3->boundingRect(), QRectF(50, 250, 50, 50));

    QScopedPointer<KUndo2Command> cmd4(
                new KoShapeTransformCommand({sublevelGroup},
                                            {sublevelGroup->transformation()},
                                            {QTransform::fromTranslate(100, 100)}));
    cmd4->redo();

    // child shapes got transformed
    QCOMPARE(toplevelShape2->boundingRect(), QRectF(150, 250, 50, 50));
    QCOMPARE(toplevelShape3->boundingRect(), QRectF(150, 350, 50, 50));

    QScopedPointer<KUndo2Command> cmd5(
        new KoShapeUngroupCommand(sublevelGroup, sublevelGroup->shapes(), {toplevelGroup}));
    cmd5->redo();

    QCOMPARE(toplevelShape2->parent(), toplevelGroup);
    QCOMPARE(toplevelShape3->parent(), toplevelGroup);

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 11);
    QCOMPARE(int(toplevelShape3->zIndex()), 12);
    QCOMPARE(int(toplevelShape4->zIndex()), 13); // changed!
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    // transformations didn't change!
    QCOMPARE(toplevelShape2->boundingRect(), QRectF(150, 250, 50, 50));
    QCOMPARE(toplevelShape3->boundingRect(), QRectF(150, 350, 50, 50));

    cmd5->undo();

    QCOMPARE(int(toplevelShape1->zIndex()), 0);
    QCOMPARE(int(toplevelShape2->zIndex()), 11); // changed!
    QCOMPARE(int(toplevelShape3->zIndex()), 12); // changed!
    QCOMPARE(int(toplevelShape4->zIndex()), 3);
    QCOMPARE(int(sublevelGroup->zIndex()), 2);

    QCOMPARE(toplevelShape2->parent(), sublevelGroup);
    QCOMPARE(toplevelShape3->parent(), sublevelGroup);

    // transformations didn't change!
    QCOMPARE(toplevelShape2->boundingRect(), QRectF(150, 250, 50, 50));
    QCOMPARE(toplevelShape3->boundingRect(), QRectF(150, 350, 50, 50));

}

SIMPLE_TEST_MAIN(TestShapeGroupCommand)
