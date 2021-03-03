/* This file is part of the KDE project
* SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
*
* SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestShapeStrokeCommand.h"

#include <MockShapes.h>
#include "KoShapeStrokeModel.h"
#include "KoShapeStroke.h"
#include "KoShapeStrokeCommand.h"
#include <KoInsets.h>

#include <simpletest.h>

void TestShapeStrokeCommand::refCounting()
{
    MockShape * shape1 = new MockShape();
    KoShapeStrokeModelSP whiteStroke(new KoShapeStroke(1.0, QColor(Qt::white)));
    KoShapeStrokeModelSP blackStroke(new KoShapeStroke(1.0, QColor(Qt::black)));
    KoShapeStrokeModelSP redStroke(new KoShapeStroke(1.0, QColor(Qt::red)));

    shape1->setStroke(whiteStroke);
    QVERIFY(shape1->stroke() == whiteStroke);

    // old stroke is white, new stroke is black
    KUndo2Command *cmd1 = new KoShapeStrokeCommand(shape1, blackStroke);
    cmd1->redo();
    QVERIFY(shape1->stroke() == blackStroke);

    // change stroke back to white stroke
    cmd1->undo();
    QVERIFY(shape1->stroke() == whiteStroke);

    // old stroke is white, new stroke is red
    KUndo2Command *cmd2 = new KoShapeStrokeCommand(shape1, redStroke);
    cmd2->redo();
    QVERIFY(shape1->stroke() == redStroke);

    // this command has the white stroke as the old stroke
    delete cmd1;

    // set stroke back to white stroke
    cmd2->undo();
    QVERIFY(shape1->stroke() == whiteStroke);

    // if white is deleted when deleting cmd1 this will crash
    KoInsets insets;
    whiteStroke->strokeInsets(shape1, insets);

    delete cmd2;
    delete shape1;
}

SIMPLE_TEST_MAIN(TestShapeStrokeCommand)
