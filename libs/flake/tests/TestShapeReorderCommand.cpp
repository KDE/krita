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

#include "TestShapeReorderCommand.h"
#include "MockShapes.h"
#include <KoShapeReorderCommand.h>
#include <KoShapeManager.h>

#include <kcomponentdata.h>

TestShapeReorderCommand::TestShapeReorderCommand()
{
    KComponentData componentData( "TestShapeReorderCommand" );  // we need an instance for the canvas
}

TestShapeReorderCommand::~TestShapeReorderCommand()
{
}

void TestShapeReorderCommand::testBringToFront()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize( QSizeF(100,100) );
    shape1.setZIndex( 1 );
    shape2.setSize( QSizeF(100,100) );
    shape2.setZIndex( 2 );
    shape3.setSize( QSizeF(100,100) );
    shape3.setZIndex( 3 );
    QList<KoShape*> shapes;
    shapes.append( &shape1 );
    shapes.append( &shape2 );
    shapes.append( &shape3 );

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes );

    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE( shapes.indexOf( &shape1 ), 0 );
    QCOMPARE( shapes.indexOf( &shape2 ), 1 );
    QCOMPARE( shapes.indexOf( &shape3 ), 2 );

    QList<KoShape*> selectedShapes;
    selectedShapes.append( &shape1 );

    QUndoCommand * cmd = KoShapeReorderCommand::createCommand( selectedShapes, &manager, KoShapeReorderCommand::BringToFront );
    cmd->redo();

    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE( shapes.indexOf( &shape2 ), 0 );
    QCOMPARE( shapes.indexOf( &shape3 ), 1 );
    QCOMPARE( shapes.indexOf( &shape1 ), 2 );

    delete cmd;
}

void TestShapeReorderCommand::testSendToBack()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize( QSizeF(100,100) );
    shape1.setZIndex( 1 );
    shape2.setSize( QSizeF(100,100) );
    shape2.setZIndex( 2 );
    shape3.setSize( QSizeF(100,100) );
    shape3.setZIndex( 3 );
    QList<KoShape*> shapes;
    shapes.append( &shape1 );
    shapes.append( &shape2 );
    shapes.append( &shape3 );

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes );

    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE( shapes.indexOf( &shape1 ), 0 );
    QCOMPARE( shapes.indexOf( &shape2 ), 1 );
    QCOMPARE( shapes.indexOf( &shape3 ), 2 );

    QList<KoShape*> selectedShapes;
    selectedShapes.append( &shape3 );

    QUndoCommand * cmd = KoShapeReorderCommand::createCommand( selectedShapes, &manager, KoShapeReorderCommand::SendToBack );
    cmd->redo();

    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE( shapes.indexOf( &shape3 ), 0 );
    QCOMPARE( shapes.indexOf( &shape1 ), 1 );
    QCOMPARE( shapes.indexOf( &shape2 ), 2 );

    delete cmd;
}

void TestShapeReorderCommand::testMoveUp()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize( QSizeF(100,100) );
    shape1.setZIndex( 1 );
    shape2.setSize( QSizeF(100,100) );
    shape2.setZIndex( 2 );
    shape3.setSize( QSizeF(100,100) );
    shape3.setZIndex( 3 );
    QList<KoShape*> shapes;
    shapes.append( &shape1 );
    shapes.append( &shape2 );
    shapes.append( &shape3 );

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes );

    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE( shapes.indexOf( &shape1 ), 0 );
    QCOMPARE( shapes.indexOf( &shape2 ), 1 );
    QCOMPARE( shapes.indexOf( &shape3 ), 2 );

    QList<KoShape*> selectedShapes;
    selectedShapes.append( &shape1 );

    QUndoCommand * cmd = KoShapeReorderCommand::createCommand( selectedShapes, &manager, KoShapeReorderCommand::RaiseShape );
    cmd->redo();

    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE( shapes.indexOf( &shape2 ), 0 );
    QCOMPARE( shapes.indexOf( &shape1 ), 1 );
    QCOMPARE( shapes.indexOf( &shape3 ), 2 );

    delete cmd;
}

void TestShapeReorderCommand::testMoveDown()
{
    MockShape shape1, shape2, shape3;

    shape1.setSize( QSizeF(100,100) );
    shape1.setZIndex( 1 );
    shape2.setSize( QSizeF(100,100) );
    shape2.setZIndex( 2 );
    shape3.setSize( QSizeF(100,100) );
    shape3.setZIndex( 3 );
    QList<KoShape*> shapes;
    shapes.append( &shape1 );
    shapes.append( &shape2 );
    shapes.append( &shape3 );

    MockCanvas canvas;
    KoShapeManager manager(&canvas, shapes );

    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE( shapes.indexOf( &shape1 ), 0 );
    QCOMPARE( shapes.indexOf( &shape2 ), 1 );
    QCOMPARE( shapes.indexOf( &shape3 ), 2 );

    QList<KoShape*> selectedShapes;
    selectedShapes.append( &shape2 );

    QUndoCommand * cmd = KoShapeReorderCommand::createCommand( selectedShapes, &manager, KoShapeReorderCommand::LowerShape );
    cmd->redo();

    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    QCOMPARE( shapes.indexOf( &shape2 ), 0 );
    QCOMPARE( shapes.indexOf( &shape1 ), 1 );
    QCOMPARE( shapes.indexOf( &shape3 ), 2 );

    delete cmd;
}

QTEST_MAIN(TestShapeReorderCommand)
#include "TestShapeReorderCommand.moc"
