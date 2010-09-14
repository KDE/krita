/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2010 Adam Celarek <kdedev@xibo.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TestShapeContainer.h"
#include <MockShapes.h>

#include <KoShape.h>
#include <QUndoCommand>
#include <KoShapeGroupCommand.h>
#include <KoShapeUngroupCommand.h>
#include <KoShapeTransformCommand.h>
#include <KoShapeGroup.h>
#include <KoSelection.h>


void TestShapeContainer::testModel()
{
    MockContainerModel *model = new MockContainerModel();
    MockContainer container(model);
    MockShape *shape1 = new MockShape();

    container.addShape(shape1);
    QCOMPARE(model->containerChangedCalled(), 0);
    QCOMPARE(model->childChangedCalled(), 1);
    QCOMPARE(model->proposeMoveCalled(), 0);

    shape1->setPosition(QPointF(300, 300));
    QCOMPARE(model->containerChangedCalled(), 0);
    QCOMPARE(model->childChangedCalled(), 2);
    QCOMPARE(model->proposeMoveCalled(), 0);

    shape1->rotate(10);
    QCOMPARE(model->containerChangedCalled(), 0);
    QCOMPARE(model->childChangedCalled(), 3);
    QCOMPARE(model->proposeMoveCalled(), 0);

    shape1->setAbsolutePosition(shape1->absolutePosition() + QPointF(10., 40.));
    QCOMPARE(model->containerChangedCalled(), 0);
    QCOMPARE(model->childChangedCalled(), 5); // we get a generic Matrix as well as a position change...
    QCOMPARE(model->proposeMoveCalled(), 0);

    shape1->setTransformation(QTransform());
    QCOMPARE(model->containerChangedCalled(), 0);
    QCOMPARE(model->childChangedCalled(), 6);
    QCOMPARE(model->proposeMoveCalled(), 0);

    model->resetCounts();
    container.setPosition(QPointF(30, 30));
    QCOMPARE(model->containerChangedCalled(), 1);
    QCOMPARE(model->childChangedCalled(), 0);
    QCOMPARE(model->proposeMoveCalled(), 0);
}

void TestShapeContainer::testSetParent()
{
    MockContainerModel *model1 = new MockContainerModel();
    MockContainer container1(model1);
    MockContainerModel *model2 = new MockContainerModel();
    MockContainer container2(model2);
    MockShape shape;
    // init test
    QCOMPARE(model1->shapes().count(), 0);
    QCOMPARE(model2->shapes().count(), 0);

    shape.setParent(&container1);
    QCOMPARE(model1->shapes().count(), 1);
    QCOMPARE(model2->shapes().count(), 0);
    QCOMPARE(shape.parent(), &container1);

    shape.setParent(&container2);
    QCOMPARE(model1->shapes().count(), 0);
    QCOMPARE(container1.shapes().count(), 0);
    QCOMPARE(model2->shapes().count(), 1);
    QCOMPARE(container2.shapes().count(), 1);
    QCOMPARE(shape.parent(), &container2);
}

void TestShapeContainer::testSetParent2()
{
    MockContainerModel *model = new MockContainerModel();
    MockContainer container(model);
    MockShape *shape = new MockShape();
    shape->setParent(&container);
    QCOMPARE(model->shapes().count(), 1);

    shape->setParent(0);
    QCOMPARE(model->shapes().count(), 0);
}

void TestShapeContainer::testScaling2()
{
    KoShape *shape1 = new MockShape();
    KoShape *shape2 = new MockShape();

    shape1->setPosition(QPointF(20., 20.));
    shape1->setSize(QSizeF(10., 10.));

    shape2->setPosition(QPointF(10., 40.));
    shape2->setSize(QSizeF(30., 10.));


    QList<KoShape*> groupedShapes;
    groupedShapes.append(shape1);
    groupedShapes.append(shape2);

    KoShapeGroup *group = new KoShapeGroup();
    KoShapeGroupCommand* groupCommand = KoShapeGroupCommand::createCommand(group, groupedShapes);
    groupCommand->redo();

    KoSelection* selection = new KoSelection();
    selection->select(shape1, true);

    QList<KoShape*> transformShapes;
    transformShapes.append(selection->selectedShapes());

    QTransform matrix;
    matrix.scale(0.5, 0.5);

    QList<QTransform> oldTransformations;
    QList<QTransform> newTransformations;
    foreach(const KoShape* shape, transformShapes) {
        QTransform oldTransform = shape->transformation();
        oldTransformations.append(oldTransform);
        newTransformations.append(oldTransform*matrix);
    }


    QList<QPointF> oldPositions;
    for(int i=0; i< transformShapes.size(); i++) {
//        kDebug() << "abs transform=" << transformShapes.at(i)->absoluteTransformation(0);
//        kDebug() << "absPos" << transformShapes.at(i)->absolutePosition(KoFlake::TopLeftCorner);
        oldPositions.append(transformShapes.at(i)->absolutePosition(KoFlake::TopLeftCorner));
    }

    KoShapeTransformCommand* transformCommand;
    transformCommand = new KoShapeTransformCommand(transformShapes, oldTransformations, newTransformations);
    transformCommand->redo();


    QRectF r1(shape1->absolutePosition(KoFlake::TopLeftCorner), shape1->absolutePosition(KoFlake::BottomRightCorner));
    QRectF r2(shape2->absolutePosition(KoFlake::TopLeftCorner), shape2->absolutePosition(KoFlake::BottomRightCorner));
//    kDebug() << "r1 u r2=" << r1.united(r2).size();
    QSizeF shapeSize=r1.united(r2).size();

    selection = new KoSelection();
    selection->select(shape1, true);
//    kDebug() << "selection=" << selection->size();
    QSizeF selecSize = selection->size();

    bool works=false;
    if(qFuzzyCompare(selecSize.width(), shapeSize.width()) && qFuzzyCompare(selecSize.height(), shapeSize.height()))
        works=true;
    QCOMPARE(works, true);

}

QTEST_MAIN(TestShapeContainer)
#include "TestShapeContainer.moc"
