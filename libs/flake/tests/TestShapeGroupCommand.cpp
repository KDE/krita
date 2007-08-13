/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "TestShapeGroupCommand.h"
#include "MockShapes.h"
#include <KoShapeGroup.h>
#include <KoShapeGroupCommand.h>
#include <QtGui/QUndoCommand>

TestShapeGroupCommand::TestShapeGroupCommand()
: toplevelGroup(0), sublevelGroup(0)
, cmd1(0), cmd2(0)
, toplevelShape1(0), toplevelShape2(0)
, sublevelShape1(0), sublevelShape2(0)
, extraShape1(0), extraShape2(0)
{
}

TestShapeGroupCommand::~TestShapeGroupCommand()
{
}

void TestShapeGroupCommand::init()
{
    toplevelShape1 = new MockShape();
    toplevelShape1->setPosition( QPointF(50,50) );
    toplevelShape1->setSize( QSize(50,50) );

    toplevelShape2 = new MockShape();
    toplevelShape2->setPosition( QPointF(50,150) );
    toplevelShape2->setSize( QSize(50,50) );

    sublevelShape1 = new MockShape();
    sublevelShape1->setPosition( QPointF(150,150) );
    sublevelShape1->setSize( QSize(50,50) );

    sublevelShape2 = new MockShape();
    sublevelShape2->setPosition( QPointF(250,150) );
    sublevelShape2->setSize( QSize(50,50) );

    extraShape1 = new MockShape();
    extraShape1->setPosition( QPointF(150,50) );
    extraShape1->setSize( QSize(50,50) );

    extraShape2 = new MockShape();
    extraShape2->setPosition( QPointF(250,50) );
    extraShape2->setSize( QSize(50,50) );

    toplevelGroup = new KoShapeGroup();
    sublevelGroup = new KoShapeGroup();
}

void TestShapeGroupCommand::cleanup()
{
    delete toplevelGroup;
    delete sublevelGroup;
    delete toplevelShape1;
    delete toplevelShape2;
    delete sublevelShape1;
    delete sublevelShape2;
    delete extraShape1;
    delete extraShape2;
    delete cmd1;
    delete cmd2;
}

void TestShapeGroupCommand::testToplevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;
    cmd1 = new KoShapeGroupCommand( toplevelGroup, toplevelShapes );

    cmd1->redo();
    QCOMPARE( toplevelShape1->parent(), toplevelGroup );
    QCOMPARE( toplevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(50,50) );
    QCOMPARE( toplevelShape1->position(), QPointF(0,0) );
    QCOMPARE( toplevelShape2->parent(), toplevelGroup );
    QCOMPARE( toplevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(50,150) );
    QCOMPARE( toplevelShape2->position(), QPointF(0,100) );
    QCOMPARE( toplevelGroup->position(), QPointF(50,50) );
    QCOMPARE( toplevelGroup->size(), QSizeF(50,150) );

    cmd1->undo();
    QVERIFY( toplevelShape1->parent() == 0 );
    QCOMPARE( toplevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(50,50) );
    QCOMPARE( toplevelShape1->position(), QPointF(50,50) );
    QVERIFY( toplevelShape2->parent() == 0 );
    QCOMPARE( toplevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(50,150) );
    QCOMPARE( toplevelShape2->position(), QPointF(50,150) );
}

void TestShapeGroupCommand::testSublevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;
    cmd1 = new KoShapeGroupCommand( toplevelGroup, toplevelShapes );

    QList<KoShape*> sublevelShapes;
    sublevelShapes << sublevelShape1 << sublevelShape2;
    new KoShapeGroupCommand( sublevelGroup, sublevelShapes, cmd1 );
    new KoShapeGroupCommand( toplevelGroup, QList<KoShape*>()<<sublevelGroup, cmd1 );

    cmd1->redo();

    QCOMPARE( toplevelShape1->parent(), toplevelGroup );
    QCOMPARE( toplevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(50,50) );
    QCOMPARE( toplevelShape1->position(), QPointF(0,0) );
    QCOMPARE( toplevelShape2->parent(), toplevelGroup );
    QCOMPARE( toplevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(50,150) );
    QCOMPARE( toplevelShape2->position(), QPointF(0,100) );
    QCOMPARE( toplevelGroup->position(), QPointF(50,50) );
    QCOMPARE( toplevelGroup->size(), QSizeF(250,150) );

    QCOMPARE( sublevelShape1->parent(), sublevelGroup );
    QCOMPARE( sublevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150,150) );
    QCOMPARE( sublevelShape1->position(), QPointF(0,0) );
    QCOMPARE( sublevelShape2->parent(), sublevelGroup );
    QCOMPARE( sublevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250,150) );
    QCOMPARE( sublevelShape2->position(), QPointF(100,0) );
    QCOMPARE( sublevelGroup->absolutePosition(KoFlake::TopLeftCorner), QPointF(150,150) );
    QCOMPARE( sublevelGroup->position(), QPointF(100,100) );
    QCOMPARE( sublevelGroup->size(), QSizeF(150,50) );
}

void TestShapeGroupCommand::testAddToToplevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;
    cmd1 = new KoShapeGroupCommand( toplevelGroup, toplevelShapes );
    cmd1->redo();

    cmd2 = new KoShapeGroupCommand( toplevelGroup, QList<KoShape*>()<<extraShape1 );
    cmd2->redo();

    QVERIFY( extraShape1->parent() == toplevelGroup );
    QCOMPARE( extraShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150,50) );
    QCOMPARE( extraShape1->position(), QPointF(100,0) );
    QCOMPARE( toplevelGroup->position(), QPointF(50,50) );
    QCOMPARE( toplevelGroup->size(), QSizeF(150,150) );

    cmd2->undo();

    QVERIFY( extraShape1->parent() == 0 );
    QCOMPARE( extraShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150,50) );
    QCOMPARE( extraShape1->position(), QPointF(150,50) );
    QCOMPARE( toplevelGroup->position(), QPointF(50,50) );
    QCOMPARE( toplevelGroup->size(), QSizeF(50,150) );
}

void TestShapeGroupCommand::testAddToSublevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;
    cmd1 = new KoShapeGroupCommand( toplevelGroup, toplevelShapes );

    QList<KoShape*> sublevelShapes;
    sublevelShapes << sublevelShape1 << sublevelShape2;
    new KoShapeGroupCommand( sublevelGroup, sublevelShapes, cmd1 );
    new KoShapeGroupCommand( toplevelGroup, QList<KoShape*>()<<sublevelGroup, cmd1 );
    cmd1->redo();

    cmd2 = new KoShapeGroupCommand( sublevelGroup, QList<KoShape*>()<<extraShape2 );
    cmd2->redo();

    QVERIFY( extraShape2->parent() == sublevelGroup );
    QCOMPARE( extraShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250,50) );
    QCOMPARE( extraShape2->position(), QPointF(100,0) );
    QCOMPARE( sublevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150,150) );
    QCOMPARE( sublevelShape1->position(), QPointF(0,100) );
    QCOMPARE( sublevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250,150) );
    QCOMPARE( sublevelShape2->position(), QPointF(100,100) );
    QCOMPARE( sublevelGroup->absolutePosition(KoFlake::TopLeftCorner), QPointF(150,50) );
    QCOMPARE( sublevelGroup->position(), QPointF(100,0) );
    QCOMPARE( sublevelGroup->size(), QSizeF(150,150) );

    cmd2->undo();

    QVERIFY( extraShape2->parent() == 0 );
    QCOMPARE( extraShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250,50) );
    QCOMPARE( extraShape2->position(), QPointF(250,50) );
    QCOMPARE( sublevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150,150) );
    QCOMPARE( sublevelShape1->position(), QPointF(0,0) );
    QCOMPARE( sublevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250,150) );
    QCOMPARE( sublevelShape2->position(), QPointF(100,0) );
    QCOMPARE( sublevelGroup->absolutePosition(KoFlake::TopLeftCorner), QPointF(150,150) );
    QCOMPARE( sublevelGroup->position(), QPointF(100,100) );
    QCOMPARE( sublevelGroup->size(), QSizeF(150,50) );
}

QTEST_MAIN(TestShapeGroupCommand)

#include "TestShapeGroupCommand.moc"
