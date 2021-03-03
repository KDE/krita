/* This file is part of the KDE project
* SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
*
* SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestShapeShadowCommand.h"

#include <MockShapes.h>
#include "KoShapeShadow.h"
#include "KoShapeShadowCommand.h"
#include "KoInsets.h"

#include <simpletest.h>

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
    KUndo2Command *cmd1 = new KoShapeShadowCommand(shape1, shadow2);
    cmd1->redo();
    QVERIFY(shape1->shadow() == shadow2);

    // change back to shadow1
    cmd1->undo();
    QVERIFY(shape1->shadow() == shadow1);

    // old shadow1, new shadow3
    KUndo2Command *cmd2 = new KoShapeShadowCommand(shape1, shadow3);
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

SIMPLE_TEST_MAIN(TestShapeShadowCommand)
