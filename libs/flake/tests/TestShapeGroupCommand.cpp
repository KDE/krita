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
#include <MockShapes.h>
#include <KoShapeGroup.h>
#include <KoShapeGroupCommand.h>
#include <KoLineBorder.h>
#include <KoShapeShadow.h>
#include <QtGui/QUndoCommand>

TestShapeGroupCommand::TestShapeGroupCommand()
        : toplevelGroup(0), sublevelGroup(0), strokeGroup(0)
        , cmd1(0), cmd2(0), strokeCmd(0)
        , toplevelShape1(0), toplevelShape2(0)
        , sublevelShape1(0), sublevelShape2(0)
        , extraShape1(0), extraShape2(0)
        , strokeShape1(0), strokeShape2(0)
{
}

TestShapeGroupCommand::~TestShapeGroupCommand()
{
}

void TestShapeGroupCommand::init()
{
    toplevelShape1 = new MockShape();
    toplevelShape1->setPosition(QPointF(50, 50));
    toplevelShape1->setSize(QSize(50, 50));

    toplevelShape2 = new MockShape();
    toplevelShape2->setPosition(QPointF(50, 150));
    toplevelShape2->setSize(QSize(50, 50));

    sublevelShape1 = new MockShape();
    sublevelShape1->setPosition(QPointF(150, 150));
    sublevelShape1->setSize(QSize(50, 50));

    sublevelShape2 = new MockShape();
    sublevelShape2->setPosition(QPointF(250, 150));
    sublevelShape2->setSize(QSize(50, 50));

    extraShape1 = new MockShape();
    extraShape1->setPosition(QPointF(150, 50));
    extraShape1->setSize(QSize(50, 50));

    extraShape2 = new MockShape();
    extraShape2->setPosition(QPointF(250, 50));
    extraShape2->setSize(QSize(50, 50));

    toplevelGroup = new KoShapeGroup();
    sublevelGroup = new KoShapeGroup();

    strokeShape1 = new MockShape();
    strokeShape1->setSize( QSizeF(50,50) );
    strokeShape1->setPosition( QPointF(0,0) );

    strokeShape2 = new MockShape();
    strokeShape2->setSize( QSizeF(50,50) );
    strokeShape2->setPosition( QPointF(25,25) );

    strokeGroup = new KoShapeGroup();
    strokeGroup->setBorder( new KoLineBorder( 2.0f ) );
    strokeGroup->setShadow( new KoShapeShadow() );
}

void TestShapeGroupCommand::cleanup()
{
    delete toplevelGroup;
    toplevelGroup = 0;
    delete sublevelGroup;
    sublevelGroup = 0;
    delete strokeGroup;
    strokeGroup = 0;
    delete toplevelShape1;
    toplevelShape1 = 0;
    delete toplevelShape2;
    toplevelShape2 = 0;
    delete sublevelShape1;
    sublevelShape1 = 0;
    delete sublevelShape2;
    sublevelShape2 = 0;
    delete extraShape1;
    extraShape1 = 0;
    delete extraShape2;
    extraShape2 = 0;
    delete strokeShape1;
    strokeShape1 = 0;
    delete strokeShape2;
    strokeShape2 = 0;
    delete cmd1;
    cmd1 = 0;
    delete cmd2;
    cmd2 = 0;
    delete strokeCmd;
    strokeCmd = 0;
}

void TestShapeGroupCommand::testToplevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;
    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes);

    cmd1->redo();
    QCOMPARE(toplevelShape1->parent(), toplevelGroup);
    QCOMPARE(toplevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(50, 50));
    QCOMPARE(toplevelShape1->position(), QPointF(0, 0));
    QCOMPARE(toplevelShape2->parent(), toplevelGroup);
    QCOMPARE(toplevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(50, 150));
    QCOMPARE(toplevelShape2->position(), QPointF(0, 100));
    QCOMPARE(toplevelGroup->position(), QPointF(50, 50));
    QCOMPARE(toplevelGroup->size(), QSizeF(50, 150));

    cmd1->undo();
    QVERIFY(toplevelShape1->parent() == 0);
    QCOMPARE(toplevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(50, 50));
    QCOMPARE(toplevelShape1->position(), QPointF(50, 50));
    QVERIFY(toplevelShape2->parent() == 0);
    QCOMPARE(toplevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(50, 150));
    QCOMPARE(toplevelShape2->position(), QPointF(50, 150));
}

void TestShapeGroupCommand::testSublevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;
    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes);

    QList<KoShape*> sublevelShapes;
    sublevelShapes << sublevelShape1 << sublevelShape2;
    sublevelShape1->setZIndex(1);
    sublevelShape2->setZIndex(2);
    sublevelShape2->setParent(strokeGroup);
    strokeGroup->setZIndex(-1);

    KoShapeGroupCommand::createCommand(sublevelGroup, sublevelShapes, cmd1);
    KoShapeGroupCommand::createCommand(toplevelGroup, QList<KoShape*>() << sublevelGroup, cmd1);

    cmd1->redo();

    QCOMPARE(toplevelShape1->parent(), toplevelGroup);
    QCOMPARE(toplevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(50, 50));
    QCOMPARE(toplevelShape1->position(), QPointF(0, 0));
    QCOMPARE(toplevelShape2->parent(), toplevelGroup);
    QCOMPARE(toplevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(50, 150));
    QCOMPARE(toplevelShape2->position(), QPointF(0, 100));
    QCOMPARE(toplevelGroup->position(), QPointF(50, 50));
    QCOMPARE(toplevelGroup->size(), QSizeF(250, 150));

    QCOMPARE(sublevelShape1->parent(), sublevelGroup);
    QCOMPARE(sublevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150, 150));
    QCOMPARE(sublevelShape1->position(), QPointF(0, 0));
    QCOMPARE(sublevelShape2->parent(), sublevelGroup);
    QCOMPARE(sublevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250, 150));
    QCOMPARE(sublevelShape2->position(), QPointF(100, 0));
    QCOMPARE(sublevelGroup->absolutePosition(KoFlake::TopLeftCorner), QPointF(150, 150));
    QCOMPARE(sublevelGroup->position(), QPointF(100, 100));
    QCOMPARE(sublevelGroup->size(), QSizeF(150, 50));

    // check that the shapes are added in the correct order
    QList<KoShape*> childOrder(sublevelGroup->shapes());
    qSort(childOrder.begin(), childOrder.end(), KoShape::compareShapeZIndex);
    QList<KoShape*> expectedOrder;
    expectedOrder << sublevelShape2 << sublevelShape1;
    QCOMPARE(childOrder, expectedOrder); 
    // check that the group has the zIndex/parent of its added top shape 
    QCOMPARE(toplevelGroup->parent(), static_cast<KoShapeContainer*>(0));
    QCOMPARE(toplevelGroup->zIndex(), 1);
}

void TestShapeGroupCommand::testAddToToplevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;
    cmd1 = KoShapeGroupCommand::createCommand(toplevelGroup, toplevelShapes);
    cmd1->redo();

    cmd2 = KoShapeGroupCommand::createCommand(toplevelGroup, QList<KoShape*>() << extraShape1);
    cmd2->redo();

    QVERIFY(extraShape1->parent() == toplevelGroup);
    QCOMPARE(extraShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150, 50));
    QCOMPARE(extraShape1->position(), QPointF(100, 0));
    QCOMPARE(toplevelGroup->position(), QPointF(50, 50));
    QCOMPARE(toplevelGroup->size(), QSizeF(150, 150));

    cmd2->undo();

    QVERIFY(extraShape1->parent() == 0);
    QCOMPARE(extraShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150, 50));
    QCOMPARE(extraShape1->position(), QPointF(150, 50));
    QCOMPARE(toplevelGroup->position(), QPointF(50, 50));
    QCOMPARE(toplevelGroup->size(), QSizeF(50, 150));
}

void TestShapeGroupCommand::testAddToSublevelGroup()
{
    QList<KoShape*> toplevelShapes;
    toplevelShapes << toplevelShape1 << toplevelShape2;
    cmd1 = new KoShapeGroupCommand(toplevelGroup, toplevelShapes);

    QList<KoShape*> sublevelShapes;
    sublevelShapes << sublevelShape1 << sublevelShape2;
    new KoShapeGroupCommand(sublevelGroup, sublevelShapes, cmd1);
    new KoShapeGroupCommand(toplevelGroup, QList<KoShape*>() << sublevelGroup, cmd1);
    cmd1->redo();

    cmd2 = new KoShapeGroupCommand(sublevelGroup, QList<KoShape*>() << extraShape2);
    cmd2->redo();

    QVERIFY(extraShape2->parent() == sublevelGroup);
    QCOMPARE(extraShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250, 50));
    QCOMPARE(extraShape2->position(), QPointF(100, 0));
    QCOMPARE(sublevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150, 150));
    QCOMPARE(sublevelShape1->position(), QPointF(0, 100));
    QCOMPARE(sublevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250, 150));
    QCOMPARE(sublevelShape2->position(), QPointF(100, 100));
    QCOMPARE(sublevelGroup->absolutePosition(KoFlake::TopLeftCorner), QPointF(150, 50));
    QCOMPARE(sublevelGroup->position(), QPointF(100, 0));
    QCOMPARE(sublevelGroup->size(), QSizeF(150, 150));

    cmd2->undo();

    QVERIFY(extraShape2->parent() == 0);
    QCOMPARE(extraShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250, 50));
    QCOMPARE(extraShape2->position(), QPointF(250, 50));
    QCOMPARE(sublevelShape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(150, 150));
    QCOMPARE(sublevelShape1->position(), QPointF(0, 0));
    QCOMPARE(sublevelShape2->absolutePosition(KoFlake::TopLeftCorner), QPointF(250, 150));
    QCOMPARE(sublevelShape2->position(), QPointF(100, 0));
    QCOMPARE(sublevelGroup->absolutePosition(KoFlake::TopLeftCorner), QPointF(150, 150));
    QCOMPARE(sublevelGroup->position(), QPointF(100, 100));
    QCOMPARE(sublevelGroup->size(), QSizeF(150, 50));
}

void TestShapeGroupCommand::testGroupStrokeShapes()
{
    QRectF bound = strokeShape1->boundingRect().unite( strokeShape2->boundingRect() );
    if (strokeGroup->border()) {
        KoInsets insets;
        strokeGroup->border()->borderInsets(strokeGroup, insets);
        bound.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (strokeGroup->shadow()) {
        KoInsets insets;
        strokeGroup->shadow()->insets(insets);
        bound.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }

    QList<KoShape*> strokeShapes;
    strokeShapes << strokeShape2 << strokeShape1;
    strokeCmd = new KoShapeGroupCommand(strokeGroup, strokeShapes);
    strokeCmd->redo();
    QCOMPARE(strokeShape1->size(), QSizeF(50, 50));
    QCOMPARE(strokeShape2->size(), QSizeF(50, 50));
    QCOMPARE(bound, strokeGroup->boundingRect());
}

QTEST_MAIN(TestShapeGroupCommand)

#include "TestShapeGroupCommand.moc"
