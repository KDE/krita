/* This file is part of the KDE project
* Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/
#include "TestShapeShadowCommand.h"

#include <MockShapes.h>
#include "KoShapeShadow.h"
#include "KoShapeShadowCommand.h"

void TestShapeShadowCommand::refCounting()
{
    MockShape * shape1 = new MockShape();
    KoShapeShadow * shadow1 = new KoShapeShadow();
    KoShapeShadow * shadow2 = new KoShapeShadow();
    KoShapeShadow * shadow3 = new KoShapeShadow();

    shape1->setShadow(shadow1);
    QVERIFY(shape1->shadow() == shadow1);
    QCOMPARE(shadow1->useCount(), 1);

    // old shadow1, new shadow2
    QUndoCommand *cmd1 = new KoShapeShadowCommand(shape1, shadow2);
    cmd1->redo();
    QVERIFY(shape1->shadow() == shadow2);

    // change back to shadow1
    cmd1->undo();
    QVERIFY(shape1->shadow() == shadow1);

    // old shadow1, new shadow3
    QUndoCommand *cmd2 = new KoShapeShadowCommand(shape1, shadow3);
    cmd2->redo();
    QVERIFY(shape1->shadow() == shadow3);

    // this command has the shadow1 as the old one
    delete cmd1;

    // set back to shadow1
    cmd2->undo();
    QVERIFY(shape1->shadow() == shadow1);

    // if shadow1 is deleted when deleting cmd1 this will crash
    KoInsets insets;
    shadow1->insets(insets);

    delete cmd2;
    delete shape1;
}

QTEST_MAIN(TestShapeShadowCommand)
#include "TestShapeShadowCommand.moc"
