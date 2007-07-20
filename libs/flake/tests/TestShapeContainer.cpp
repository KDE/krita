#include "TestShapeContainer.h"
#include "MockShapes.h"

void TestShapeContainer::testModel() {
    MockContainerModel *model = new MockContainerModel();
    MockContainer container(model);
    MockShape *shape1 = new MockShape();

    container.addChild(shape1);
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

    shape1->moveBy(10., 40.);
    QCOMPARE(model->containerChangedCalled(), 0);
    QCOMPARE(model->childChangedCalled(), 4);
    QCOMPARE(model->proposeMoveCalled(), 0);

    shape1->setTransformation(QMatrix());
    QCOMPARE(model->containerChangedCalled(), 0);
QEXPECT_FAIL("", "faulty", Continue); // TODO fix KoShape
    QCOMPARE(model->childChangedCalled(), 5);
    QCOMPARE(model->proposeMoveCalled(), 0);

    model->resetCounts();
    container.setPosition(QPointF(30, 30));
    QCOMPARE(model->containerChangedCalled(), 1);
    QCOMPARE(model->childChangedCalled(), 0);
    QCOMPARE(model->proposeMoveCalled(), 0);
}

QTEST_MAIN(TestShapeContainer)
#include "TestShapeContainer.moc"
