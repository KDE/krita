/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestControlPointMoveCommand.h"

#include <QPainterPath>
#include "KoPathShape.h"
#include "KoPathControlPointMoveCommand.h"

#include <QTest>

void TestControlPointMoveCommand::redoUndoControlPoint1()
{
    KoPathShape path;
    path.moveTo(QPointF(0, 0));
    path.lineTo(QPointF(0, 100));
    KoPathPoint *point1 = path.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    path.curveTo(QPointF(100, 150), QPointF(200, 150), QPointF(200, 100));
    KoPathPoint *point2 = path.moveTo(QPointF(0, 300));
    path.lineTo(QPointF(100, 400));
    path.curveTo(QPointF(50, 400), QPointF(0, 350), QPointF(0, 300));
    path.closeMerge();

    QPainterPath ppathOrg = path.outline();
    KoPathControlPointMoveCommand cmd1(KoPathPointData(&path, path.pathPointIndex(point1)), QPointF(10, 10), KoPathPoint::ControlPoint1);
    cmd1.redo();

    QPainterPath ppathNew1(QPointF(0, 0));
    ppathNew1.lineTo(0, 100);
    ppathNew1.cubicTo(0, 50, 110, 60, 100, 100);
    ppathNew1.cubicTo(100, 150, 200, 150, 200, 100);
    ppathNew1.moveTo(0, 300);
    ppathNew1.lineTo(100, 400);
    ppathNew1.cubicTo(50, 400, 0, 350, 0, 300);
    ppathNew1.closeSubpath();

    QVERIFY(ppathNew1 == path.outline());

    KoPathControlPointMoveCommand cmd2(KoPathPointData(&path, path.pathPointIndex(point2)), QPointF(10, -10), KoPathPoint::ControlPoint1);
    cmd2.redo();

    QPainterPath ppathNew2(QPointF(0, 0));
    ppathNew2.lineTo(0, 100);
    ppathNew2.cubicTo(0, 50, 110, 60, 100, 100);
    ppathNew2.cubicTo(100, 150, 200, 150, 200, 100);
    ppathNew2.moveTo(0, 300);
    ppathNew2.lineTo(100, 400);
    ppathNew2.cubicTo(50, 400, 10, 340, 0, 300);
    ppathNew2.closeSubpath();

    cmd2.undo();

    QVERIFY(ppathNew1 == path.outline());

    cmd1.undo();

    QVERIFY(ppathOrg == path.outline());
}

void TestControlPointMoveCommand::redoUndoControlPoint1Smooth()
{
    KoPathShape path;
    path.moveTo(QPointF(0, 0));
    path.lineTo(QPointF(0, 100));
    KoPathPoint *point1 = path.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    path.curveTo(QPointF(100, 150), QPointF(200, 150), QPointF(200, 100));
    path.moveTo(QPointF(0, 300));
    path.lineTo(QPointF(100, 400));
    path.curveTo(QPointF(50, 400), QPointF(0, 350), QPointF(0, 300));
    path.closeMerge();

    point1->setProperties(point1->properties() | KoPathPoint::IsSmooth);

    QPainterPath ppathOrg = path.outline();
    KoPathControlPointMoveCommand cmd1(KoPathPointData(&path, path.pathPointIndex(point1)), QPointF(-25, 50), KoPathPoint::ControlPoint1);
    cmd1.redo();

    QPainterPath ppathNew1(QPointF(0, 0));
    ppathNew1.lineTo(0, 100);
    ppathNew1.cubicTo(0, 50, 75, 100, 100, 100);
    ppathNew1.cubicTo(150, 100, 200, 150, 200, 100);
    ppathNew1.moveTo(0, 300);
    ppathNew1.lineTo(100, 400);
    ppathNew1.cubicTo(50, 400, 0, 350, 0, 300);
    ppathNew1.closeSubpath();

    QVERIFY(ppathNew1 == path.outline());

    cmd1.undo();

    QVERIFY(ppathOrg == path.outline());
}

void TestControlPointMoveCommand::redoUndoControlPoint1Symmetric()
{
    KoPathShape path;
    path.moveTo(QPointF(0, 0));
    path.lineTo(QPointF(0, 100));
    KoPathPoint *point1 = path.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    path.curveTo(QPointF(100, 150), QPointF(200, 150), QPointF(200, 100));
    path.moveTo(QPointF(0, 300));
    path.lineTo(QPointF(100, 400));
    path.curveTo(QPointF(50, 400), QPointF(0, 350), QPointF(0, 300));
    path.closeMerge();

    point1->setProperties(point1->properties() | KoPathPoint::IsSymmetric);

    QPainterPath ppathOrg = path.outline();
    KoPathControlPointMoveCommand cmd1(KoPathPointData(&path, path.pathPointIndex(point1)), QPointF(-25, 50), KoPathPoint::ControlPoint1);
    cmd1.redo();

    QPainterPath ppathNew1(QPointF(0, 0));
    ppathNew1.lineTo(0, 100);
    ppathNew1.cubicTo(0, 50, 75, 100, 100, 100);
    ppathNew1.cubicTo(125, 100, 200, 150, 200, 100);
    ppathNew1.moveTo(0, 300);
    ppathNew1.lineTo(100, 400);
    ppathNew1.cubicTo(50, 400, 0, 350, 0, 300);
    ppathNew1.closeSubpath();

    QVERIFY(ppathNew1 == path.outline());

    cmd1.undo();

    QVERIFY(ppathOrg == path.outline());
}

void TestControlPointMoveCommand::redoUndoControlPoint2()
{
    KoPathShape path;
    path.moveTo(QPointF(0, 0));
    path.lineTo(QPointF(0, 100));
    KoPathPoint *point1 = path.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    path.curveTo(QPointF(100, 150), QPointF(200, 150), QPointF(200, 100));
    path.moveTo(QPointF(0, 300));
    KoPathPoint *point2 = path.lineTo(QPointF(100, 400));
    path.curveTo(QPointF(50, 400), QPointF(0, 350), QPointF(0, 300));
    path.closeMerge();

    QPainterPath ppathOrg = path.outline();
    KoPathControlPointMoveCommand cmd1(KoPathPointData(&path, path.pathPointIndex(point1)), QPointF(10, 10), KoPathPoint::ControlPoint2);
    cmd1.redo();

    QPainterPath ppathNew1(QPointF(0, 0));
    ppathNew1.lineTo(0, 100);
    ppathNew1.cubicTo(0, 50, 100, 50, 100, 100);
    ppathNew1.cubicTo(110, 160, 200, 150, 200, 100);
    ppathNew1.moveTo(0, 300);
    ppathNew1.lineTo(100, 400);
    ppathNew1.cubicTo(50, 400, 0, 350, 0, 300);
    ppathNew1.closeSubpath();

    QVERIFY(ppathNew1 == path.outline());

    KoPathControlPointMoveCommand cmd2(KoPathPointData(&path, path.pathPointIndex(point2)), QPointF(-10, -10), KoPathPoint::ControlPoint2);
    cmd2.redo();

    QPainterPath ppathNew2(QPointF(0, 0));
    ppathNew2.lineTo(0, 100);
    ppathNew2.cubicTo(0, 50, 100, 50, 100, 100);
    ppathNew2.cubicTo(110, 160, 200, 150, 200, 100);
    ppathNew2.moveTo(0, 300);
    ppathNew2.lineTo(100, 400);
    ppathNew2.cubicTo(40, 390, 0, 350, 0, 300);
    ppathNew2.closeSubpath();

    cmd2.undo();

    QVERIFY(ppathNew1 == path.outline());

    cmd1.undo();

    QVERIFY(ppathOrg == path.outline());
}

void TestControlPointMoveCommand::redoUndoControlPoint2Smooth()
{
    KoPathShape path;
    path.moveTo(QPointF(0, 0));
    path.lineTo(QPointF(0, 100));
    KoPathPoint *point1 = path.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    path.curveTo(QPointF(100, 150), QPointF(200, 150), QPointF(200, 100));
    path.moveTo(QPointF(0, 300));
    path.lineTo(QPointF(100, 400));
    path.curveTo(QPointF(50, 400), QPointF(0, 350), QPointF(0, 300));
    path.closeMerge();

    point1->setProperties(point1->properties() | KoPathPoint::IsSmooth);

    QPainterPath ppathOrg = path.outline();
    KoPathControlPointMoveCommand cmd1(KoPathPointData(&path, path.pathPointIndex(point1)), QPointF(25, -50), KoPathPoint::ControlPoint2);
    cmd1.redo();

    QPainterPath ppathNew1(QPointF(0, 0));
    ppathNew1.lineTo(0, 100);
    ppathNew1.cubicTo(0, 50, 50, 100, 100, 100);
    ppathNew1.cubicTo(125, 100, 200, 150, 200, 100);
    ppathNew1.moveTo(0, 300);
    ppathNew1.lineTo(100, 400);
    ppathNew1.cubicTo(50, 400, 0, 350, 0, 300);
    ppathNew1.closeSubpath();

    QVERIFY(ppathNew1 == path.outline());

    cmd1.undo();

    QVERIFY(ppathOrg == path.outline());
}

void TestControlPointMoveCommand::redoUndoControlPoint2Symmetric()
{
    KoPathShape path;
    path.moveTo(QPointF(0, 0));
    path.lineTo(QPointF(0, 100));
    KoPathPoint *point1 = path.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    path.curveTo(QPointF(100, 150), QPointF(200, 150), QPointF(200, 100));
    path.moveTo(QPointF(0, 300));
    path.lineTo(QPointF(100, 400));
    path.curveTo(QPointF(50, 400), QPointF(0, 350), QPointF(0, 300));
    path.closeMerge();

    point1->setProperties(point1->properties() | KoPathPoint::IsSymmetric);

    QPainterPath ppathOrg = path.outline();
    KoPathControlPointMoveCommand cmd1(KoPathPointData(&path, path.pathPointIndex(point1)), QPointF(25, -50), KoPathPoint::ControlPoint2);
    cmd1.redo();

    QPainterPath ppathNew1(QPointF(0, 0));
    ppathNew1.lineTo(0, 100);
    ppathNew1.cubicTo(0, 50, 75, 100, 100, 100);
    ppathNew1.cubicTo(125, 100, 200, 150, 200, 100);
    ppathNew1.moveTo(0, 300);
    ppathNew1.lineTo(100, 400);
    ppathNew1.cubicTo(50, 400, 0, 350, 0, 300);
    ppathNew1.closeSubpath();

    QVERIFY(ppathNew1 == path.outline());

    cmd1.undo();

    QVERIFY(ppathOrg == path.outline());
}

QTEST_MAIN(TestControlPointMoveCommand)
