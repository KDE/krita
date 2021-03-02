/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestRemoveSubpathCommand.h"

#include <QPainterPath>
#include "KoPathShape.h"
#include "KoSubpathRemoveCommand.h"
#include <simpletest.h>

void TestRemoveSubpathCommand::redoUndo()
{
    KoPathShape path;
    path.moveTo(QPointF(0, 0));
    path.lineTo(QPointF(0, 100));
    path.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    path.lineTo(QPointF(200, 100));
    path.curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    path.moveTo(QPointF(100, 0));
    path.curveTo(QPointF(150, 0), QPointF(200, 50), QPointF(200, 100));
    path.curveTo(QPointF(150, 100), QPointF(100, 50), QPointF(100, 0));
    path.closeMerge();
    path.moveTo(QPointF(200, 0));
    path.lineTo(QPointF(200, 100));

    QPainterPath ppathOrg = path.outline();

    KUndo2Command *cmd1 = new KoSubpathRemoveCommand(&path, 0);
    cmd1->redo();

    QPainterPath ppath1(QPointF(0, 0));
    ppath1.cubicTo(50, 0, 100, 50, 100, 100);
    ppath1.cubicTo(50, 100, 0, 50, 0, 0);
    ppath1.closeSubpath();
    ppath1.moveTo(100, 0);
    ppath1.lineTo(100, 100);

    QVERIFY(ppath1 == path.outline());

    cmd1->undo();

    QVERIFY(ppathOrg == path.outline());

    KUndo2Command *cmd2 = new KoSubpathRemoveCommand(&path, 1);
    cmd2->redo();

    QPainterPath ppath2(QPointF(0, 0));
    ppath2.lineTo(0, 100);
    ppath2.cubicTo(0, 50, 100, 50, 100, 100);
    ppath2.lineTo(200, 100);
    ppath2.cubicTo(200, 50, 300, 50, 300, 100);
    ppath2.moveTo(200, 0);
    ppath2.lineTo(200, 100);

    QVERIFY(ppath2 == path.outline());

    cmd2->undo();

    QVERIFY(ppathOrg == path.outline());

    KUndo2Command *cmd3 = new KoSubpathRemoveCommand(&path, 2);
    cmd3->redo();

    QPainterPath ppath3(QPointF(0, 0));
    ppath3.lineTo(0, 100);
    ppath3.cubicTo(0, 50, 100, 50, 100, 100);
    ppath3.lineTo(200, 100);
    ppath3.cubicTo(200, 50, 300, 50, 300, 100);
    ppath3.moveTo(100, 0);
    ppath3.cubicTo(150, 0, 200, 50, 200, 100);
    ppath3.cubicTo(150, 100, 100, 50, 100, 0);
    ppath3.closeSubpath();

    QVERIFY(ppath3 == path.outline());

    cmd3->undo();

    QVERIFY(ppathOrg == path.outline());

    cmd1->redo();

    QVERIFY(ppath1 == path.outline());

    delete cmd3;
    delete cmd2;
    delete cmd1;
}

SIMPLE_TEST_MAIN(TestRemoveSubpathCommand)
