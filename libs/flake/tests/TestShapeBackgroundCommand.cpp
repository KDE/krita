/* This file is part of the KDE project
* SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
*
* SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestShapeBackgroundCommand.h"

#include <MockShapes.h>
#include "KoShapeBackgroundCommand.h"
#include "KoColorBackground.h"
#include "KoShapePaintingContext.h"

#include <QPainterPath>
#include <simpletest.h>

void TestShapeBackgroundCommand::refCounting()
{
    MockShape * shape1 = new MockShape();
    QSharedPointer<KoShapeBackground> whiteFill(new KoColorBackground(QColor(Qt::white)));
    QSharedPointer<KoShapeBackground> blackFill(new KoColorBackground(QColor(Qt::black)));
    QSharedPointer<KoShapeBackground> redFill  (new KoColorBackground(QColor(Qt::red)));

    shape1->setBackground(whiteFill);
    QVERIFY(shape1->background() == whiteFill);

    // old fill is white, new fill is black
    KUndo2Command *cmd1 = new KoShapeBackgroundCommand(shape1, blackFill);
    cmd1->redo();
    QVERIFY(shape1->background() == blackFill);

    // change fill back to white fill
    cmd1->undo();
    QVERIFY(shape1->background() == whiteFill);

    // old fill is white, new fill is red
    KUndo2Command *cmd2 = new KoShapeBackgroundCommand(shape1, redFill);
    cmd2->redo();
    QVERIFY(shape1->background() == redFill);

    // this command has the white fill as the old fill
    delete cmd1;

    // set fill back to white fill
    cmd2->undo();
    QVERIFY(shape1->background() == whiteFill);

    // if white is deleted when deleting cmd1 this will crash
    QPainter p;
    QPainterPath path;
    path.addRect( QRectF(0,0,100,100) );
    KoShapePaintingContext context;
    whiteFill->paint( p, context, path );

    delete cmd2;
    delete shape1;
}

SIMPLE_TEST_MAIN(TestShapeBackgroundCommand)
