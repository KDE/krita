#include "TestShapeContainer.h"
#include <MockShapes.h>

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

QTEST_MAIN(TestShapeContainer)
#include "TestShapeContainer.moc"
