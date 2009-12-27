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
#include "TestShapeBackgroundCommand.h"

#include <MockShapes.h>
#include "KoShapeBackgroundCommand.h"
#include "KoColorBackground.h"

void TestShapeBackgroundCommand::refCounting()
{
    MockShape * shape1 = new MockShape();
    KoShapeBackground * whiteFill = new KoColorBackground(QColor(Qt::white));
    KoShapeBackground * blackFill = new KoColorBackground(QColor(Qt::black));
    KoShapeBackground * redFill = new KoColorBackground(QColor(Qt::red));

    shape1->setBackground(whiteFill);
    QVERIFY(shape1->background() == whiteFill);
    QCOMPARE(whiteFill->useCount(), 1);

    // old fill is white, new fill is black
    QUndoCommand *cmd1 = new KoShapeBackgroundCommand(shape1, blackFill);
    cmd1->redo();
    QVERIFY(shape1->background() == blackFill);

    // change fill back to white fill
    cmd1->undo();
    QVERIFY(shape1->background() == whiteFill);

    // old fill is white, new fill is red
    QUndoCommand *cmd2 = new KoShapeBackgroundCommand(shape1, redFill);
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
    whiteFill->paint( p, path );

    delete cmd2;
    delete shape1;
}

QTEST_MAIN(TestShapeBackgroundCommand)
#include "TestShapeBackgroundCommand.moc"
