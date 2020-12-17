/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestPointRemoveCommand.h"

#include <QPainterPath>
#include "KoPathShape.h"
#include "KoPathPointRemoveCommand.h"
#include "KoShapeController.h"
#include <MockShapes.h>
#include <sdk/tests/testflake.h>
#include <QTest>

void TestPointRemoveCommand::redoUndoPointRemove()
{
    KoPathShape path1;
    path1.moveTo(QPointF(0, 0));
    path1.lineTo(QPointF(0, 100));
    KoPathPoint *point1 = path1.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KoPathPoint *point2 = path1.lineTo(QPointF(200, 100));
    path1.curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));

    QPainterPath orig1(QPointF(0, 0));
    orig1.lineTo(0, 100);
    orig1.cubicTo(0, 50, 100, 50, 100, 100);
    orig1.lineTo(200, 100);
    orig1.cubicTo(200, 50, 300, 50, 300, 100);

    QVERIFY(orig1 == path1.outline());

    KoPathShape path2;
    path2.moveTo(QPointF(0, 0));
    KoPathPoint *point3 = path2.curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2.curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2.closeMerge();

    QList<KoPathPointData> pd;
    pd.append(KoPathPointData(&path1, path1.pathPointIndex(point1)));
    pd.append(KoPathPointData(&path1, path1.pathPointIndex(point2)));
    pd.append(KoPathPointData(&path2, path2.pathPointIndex(point3)));

    QPainterPath ppath1Org = path1.outline();
    QPainterPath ppath2Org = path2.outline();

    MockShapeController mockController;
    KoShapeController shapeController(0, &mockController);

    KUndo2Command *cmd = KoPathPointRemoveCommand::createCommand(pd, &shapeController);
    cmd->redo();

    QPainterPath ppath1(QPointF(0, 0));
    ppath1.lineTo(0, 100);
    ppath1.cubicTo(0, 50, 300, 50, 300, 100);

    QPainterPath ppath2(QPointF(0, 0));
    ppath2.cubicTo(50, 0, 0, 50, 0, 0);
    ppath2.closeSubpath();

    QVERIFY(ppath1 == path1.outline());
    QVERIFY(ppath2 == path2.outline());

    cmd->undo();

    QVERIFY(ppath1Org == path1.outline());
    QVERIFY(ppath2Org == path2.outline());

    delete cmd;
}

void TestPointRemoveCommand::redoUndoSubpathRemove()
{
    KoPathShape path1;
    KoPathPoint *point11 = path1.moveTo(QPointF(0, 0));
    KoPathPoint *point12 = path1.lineTo(QPointF(0, 100));
    KoPathPoint *point13 = path1.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KoPathPoint *point14 = path1.lineTo(QPointF(200, 100));
    KoPathPoint *point15 = path1.curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    KoPathPoint *point21 = path1.moveTo(QPointF(0, 0));
    KoPathPoint *point22 = path1.curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path1.curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path1.closeMerge();
    path1.moveTo(QPointF(100, 0));
    path1.lineTo(QPointF(100, 100));

    QList<KoPathPointData> pd;
    pd.append(KoPathPointData(&path1, path1.pathPointIndex(point11)));
    pd.append(KoPathPointData(&path1, path1.pathPointIndex(point12)));
    pd.append(KoPathPointData(&path1, path1.pathPointIndex(point13)));
    pd.append(KoPathPointData(&path1, path1.pathPointIndex(point14)));
    pd.append(KoPathPointData(&path1, path1.pathPointIndex(point15)));

    QPainterPath ppath1Org = path1.outline();

    MockShapeController mockController;
    KoShapeController shapeController(0, &mockController);

    KUndo2Command *cmd1 = KoPathPointRemoveCommand::createCommand(pd, &shapeController);
    cmd1->redo();

    QPainterPath ppath1(QPointF(0, 0));
    ppath1.cubicTo(50, 0, 100, 50, 100, 100);
    ppath1.cubicTo(50, 100, 0, 50, 0, 0);
    ppath1.closeSubpath();
    ppath1.moveTo(100, 0);
    ppath1.lineTo(100, 100);

    QPainterPath ppath1mod = path1.outline();
    QVERIFY(ppath1 == ppath1mod);

    QList<KoPathPointData> pd2;
    pd2.append(KoPathPointData(&path1, path1.pathPointIndex(point21)));
    pd2.append(KoPathPointData(&path1, path1.pathPointIndex(point22)));

    KUndo2Command *cmd2 = KoPathPointRemoveCommand::createCommand(pd2, &shapeController);
    cmd2->redo();

    QPainterPath ppath2(QPointF(0, 0));
    ppath2.lineTo(0, 100);

    QVERIFY(ppath2 == path1.outline());

    cmd2->undo();

    QVERIFY(ppath1mod == path1.outline());

    cmd1->undo();

    QVERIFY(ppath1Org == path1.outline());

    delete cmd2;
    delete cmd1;
}

void TestPointRemoveCommand::redoUndoShapeRemove()
{
    KoPathShape *path1 = new KoPathShape();
    KoPathPoint *point11 = path1->moveTo(QPointF(0, 0));
    KoPathPoint *point12 = path1->lineTo(QPointF(0, 100));
    KoPathPoint *point13 = path1->curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KoPathPoint *point14 = path1->lineTo(QPointF(200, 100));
    KoPathPoint *point15 = path1->curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    KoPathShape *path2 = new KoPathShape();
    KoPathPoint *point21 = path2->moveTo(QPointF(0, 0));
    KoPathPoint *point22 = path2->curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2->curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2->closeMerge();

    QList<KoPathPointData> pd;
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point12)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point11)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point13)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point15)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point14)));
    pd.append(KoPathPointData(path2, path2->pathPointIndex(point22)));
    pd.append(KoPathPointData(path2, path2->pathPointIndex(point21)));

    QPainterPath ppath1Org = path1->outline();
    QPainterPath ppath2Org = path2->outline();

    MockShapeController mockController;
    KoShapeController shapeController(0, &mockController);
    QScopedPointer<MockContainer> rootContainer(new MockContainer());
    rootContainer->addShape(path1);
    rootContainer->addShape(path2);

    KUndo2Command *cmd = KoPathPointRemoveCommand::createCommand(pd, &shapeController);
    cmd->redo();
    QVERIFY(!rootContainer->contains(path1));
    QVERIFY(!rootContainer->contains(path2));
    cmd->undo();

    QVERIFY(rootContainer->contains(path1));
    QVERIFY(rootContainer->contains(path2));

    QVERIFY(ppath1Org == path1->outline());
    QVERIFY(ppath2Org == path2->outline());

    delete cmd;
    rootContainer.reset();
}

void TestPointRemoveCommand::redoUndo()
{
    KoPathShape *path1 = new KoPathShape();
    KoPathPoint *point11 = path1->moveTo(QPointF(0, 0));
    KoPathPoint *point12 = path1->lineTo(QPointF(0, 100));
    KoPathPoint *point13 = path1->curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KoPathPoint *point14 = path1->lineTo(QPointF(200, 100));
    KoPathPoint *point15 = path1->curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    KoPathPoint *point16 = path1->moveTo(QPointF(100, 0));
    KoPathPoint *point17 = path1->curveTo(QPointF(150, 0), QPointF(200, 50), QPointF(200, 100));
    path1->curveTo(QPointF(150, 100), QPointF(100, 50), QPointF(100, 0));
    path1->closeMerge();
    KoPathPoint *point18 = path1->moveTo(QPointF(200, 0));
    KoPathPoint *point19 = path1->lineTo(QPointF(200, 100));

    KoPathShape *path2 = new KoPathShape();
    KoPathPoint *point21 = path2->moveTo(QPointF(0, 0));
    KoPathPoint *point22 = path2->curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2->curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2->closeMerge();

    KoPathShape *path3 = new KoPathShape();
    KoPathPoint *point31 = path3->moveTo(QPointF(0, 0));
    KoPathPoint *point32 = path3->lineTo(QPointF(100, 100));
    KoPathPoint *point33 = path3->lineTo(QPointF(200, 150));

    MockShapeController mockController;
    QScopedPointer<MockContainer> rootContainer(new MockContainer());
    rootContainer->addShape(path1);
    rootContainer->addShape(path2);
    rootContainer->addShape(path3);

    QList<KoPathPointData> pd;
    pd.append(KoPathPointData(path2, path2->pathPointIndex(point21)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point13)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point11)));
    pd.append(KoPathPointData(path3, path3->pathPointIndex(point31)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point12)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point15)));
    pd.append(KoPathPointData(path2, path2->pathPointIndex(point22)));
    pd.append(KoPathPointData(path1, path1->pathPointIndex(point14)));

    KoShapeController shapeController(0, &mockController);

    QPainterPath ppath1Org = path1->outline();
    QPainterPath ppath2Org = path2->outline();
    QPainterPath ppath3Org = path3->outline();

    KUndo2Command *cmd1 = KoPathPointRemoveCommand::createCommand(pd, &shapeController);
    cmd1->redo();

    QVERIFY(rootContainer->contains(path1));
    QVERIFY(!rootContainer->contains(path2));
    QVERIFY(rootContainer->contains(path3));

    QPainterPath ppath1(QPointF(0, 0));
    ppath1.cubicTo(50, 0, 100, 50, 100, 100);
    ppath1.cubicTo(50, 100, 0, 50, 0, 0);
    ppath1.closeSubpath();
    ppath1.moveTo(100, 0);
    ppath1.lineTo(100, 100);

    QPainterPath ppath1mod = path1->outline();
    QVERIFY(ppath1 == ppath1mod);

    QPainterPath ppath3(QPointF(0, 0));
    ppath3.lineTo(100, 50);

    QPainterPath ppath3mod = path3->outline();
    QVERIFY(ppath3 == ppath3mod);

    QList<KoPathPointData> pd2;
    pd2.append(KoPathPointData(path1, path1->pathPointIndex(point16)));
    pd2.append(KoPathPointData(path1, path1->pathPointIndex(point17)));
    pd2.append(KoPathPointData(path1, path1->pathPointIndex(point18)));
    pd2.append(KoPathPointData(path1, path1->pathPointIndex(point19)));
    pd2.append(KoPathPointData(path3, path3->pathPointIndex(point32)));
    pd2.append(KoPathPointData(path3, path3->pathPointIndex(point33)));

    KUndo2Command *cmd2 = KoPathPointRemoveCommand::createCommand(pd2, &shapeController);
    cmd2->redo();

    QVERIFY(!rootContainer->contains(path1));
    QVERIFY(!rootContainer->contains(path2));
    QVERIFY(!rootContainer->contains(path3));

    cmd2->undo();

    QVERIFY(rootContainer->contains(path1));
    QVERIFY(!rootContainer->contains(path2));
    QVERIFY(rootContainer->contains(path3));
    QVERIFY(ppath1 == ppath1mod);
    QVERIFY(ppath3 == ppath3mod);

    cmd1->undo();

    QVERIFY(ppath1Org == path1->outline());
    QVERIFY(ppath2Org == path2->outline());
    QVERIFY(ppath3Org == path3->outline());

    cmd1->redo();
    cmd2->redo();

    delete cmd2;
    delete cmd1;
}

KISTEST_MAIN(TestPointRemoveCommand)
