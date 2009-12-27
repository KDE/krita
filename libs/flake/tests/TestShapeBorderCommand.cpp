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
#include "TestShapeBorderCommand.h"

#include <MockShapes.h>
#include "KoShapeBorderModel.h"
#include "KoLineBorder.h"
#include "KoShapeBorderCommand.h"

void TestShapeBorderCommand::refCounting()
{
    MockShape * shape1 = new MockShape();
    KoShapeBorderModel * whiteBorder = new KoLineBorder(1.0, QColor(Qt::white));
    KoShapeBorderModel * blackBorder = new KoLineBorder(1.0, QColor(Qt::black));
    KoShapeBorderModel * redBorder = new KoLineBorder(1.0, QColor(Qt::red));

    shape1->setBorder(whiteBorder);
    QVERIFY(shape1->border() == whiteBorder);
    QCOMPARE(whiteBorder->useCount(), 1);

    // old border is white, new border is black
    QUndoCommand *cmd1 = new KoShapeBorderCommand(shape1, blackBorder);
    cmd1->redo();
    QVERIFY(shape1->border() == blackBorder);

    // change border back to white border
    cmd1->undo();
    QVERIFY(shape1->border() == whiteBorder);

    // old border is white, new border is red
    QUndoCommand *cmd2 = new KoShapeBorderCommand(shape1, redBorder);
    cmd2->redo();
    QVERIFY(shape1->border() == redBorder);

    // this command has the white border as the old border
    delete cmd1;

    // set border back to white border
    cmd2->undo();
    QVERIFY(shape1->border() == whiteBorder);

    // if white is deleted when deleting cmd1 this will crash
    whiteBorder->borderInsets(shape1);

    delete cmd2;
    delete shape1;
}

QTEST_MAIN(TestShapeBorderCommand)
#include "TestShapeBorderCommand.moc"
