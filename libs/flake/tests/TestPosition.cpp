#include "TestPosition.h"

#include <MockShapes.h>
#include <QPointF>

TestPosition::TestPosition()
        : shape1(0),
        shape2(0),
        childShape1(0),
        childShape2(0),
        container(0),
        container2(0)
{
}

void TestPosition::init()
{
    shape1 = new MockShape();
    shape1->setPosition(QPointF(50, 50));
    shape1->setSize(QSize(50, 50));
    shape2 = new MockShape();
    shape2->setPosition(QPointF(20, 20));
    shape2->setSize(QSize(50, 50));

    childShape1 = new MockShape();
    childShape1->setPosition(QPointF(20, 20));
    childShape1->setSize(QSize(50, 50));
    container = new MockContainer();
    container->setPosition(QPointF(100, 100));
    container->addShape(childShape1);
    container->setClipped(childShape1, false);

    childShape2 = new MockShape();
    childShape2->setPosition(QPointF(25, 25));
    childShape2->setSize(QSizeF(10, 15));
    container2 = new MockContainer();
    container2->setPosition(QPointF(100, 200));
    container2->setSize(QSizeF(100, 100));
    container2->rotate(90);
    container2->addShape(childShape2);
}

void TestPosition::cleanup()
{
    delete container;
    delete container2;
    delete shape1;
    delete shape2;
    delete childShape1;
    delete childShape2;
}

void TestPosition::testBasePosition()
{
    // internal consistency tests.
    QCOMPARE(shape1->position(), QPointF(50, 50));
    QCOMPARE(shape2->position(), QPointF(20, 20));
    QCOMPARE(childShape1->position(), QPointF(20, 20));
    QCOMPARE(container->position(), QPointF(100, 100));
}

void TestPosition::testAbsolutePosition()
{
    QCOMPARE(shape1->absolutePosition(), QPointF(75, 75));
    QCOMPARE(shape2->absolutePosition(), QPointF(45, 45));

    // translated
    QCOMPARE(childShape1->absolutePosition(), QPointF(100 + 20 + 25, 100 + 20 + 25));

    // rotated
    container2->setClipped(childShape2, false);
    QCOMPARE(container2->absolutePosition(), QPointF(150, 250));
    QCOMPARE(childShape2->absolutePosition(), QPointF(130, 232.5));
    container2->setClipped(childShape2, true);
    QCOMPARE(childShape2->absolutePosition(), QPointF(167.5, 230));

    shape1->rotate(90);
    shape1->setPosition(QPointF(10, 10));

    QCOMPARE(shape1->absolutePosition(), QPointF(10 + 25, 10 + 25));
    QCOMPARE(shape1->absolutePosition(KoFlake::CenteredPosition), QPointF(10 + 25, 10 + 25));
    QCOMPARE(shape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(10 + 50, 10));
    QCOMPARE(shape1->absolutePosition(KoFlake::BottomRightCorner), QPointF(10, 10 + 50));

    QCOMPARE(container2->absolutePosition(KoFlake::TopLeftCorner), QPointF(200, 200));
}

void TestPosition::testSetAbsolutePosition()
{
    shape1->rotate(-90); // rotate back as we are rotating relative
    shape1->setPosition(QPointF(10, 10));
    QCOMPARE(shape1->absolutePosition(), QPointF(10 + 25, 10 + 25));
    shape1->setAbsolutePosition(QPointF(10, 10));
    QCOMPARE(shape1->absolutePosition(), QPointF(10, 10));
    shape1->rotate(45);
    QCOMPARE(shape1->absolutePosition(), QPointF(10, 10));

    childShape1->setAbsolutePosition(QPointF(0, 0));
    QCOMPARE(childShape1->position(), QPointF(-125, -125));
    QCOMPARE(childShape1->absolutePosition(), QPointF(0, 0));

    QCOMPARE(container2->position(), QPointF(100, 200));  // make sure nobody changed it
    container2->setClipped(childShape2, false);
    childShape2->setAbsolutePosition(QPointF(0, 0));
    QCOMPARE(childShape2->position(), QPointF(-100 - 5, -200 - 7.5));
    QCOMPARE(childShape2->absolutePosition(), QPointF(0, 0));

    container2->setClipped(childShape2, true);
    childShape2->setAbsolutePosition(QPointF(0, 0));
    QCOMPARE(childShape2->absolutePosition(), QPointF(0, 0));
    QCOMPARE(childShape2->position(), QPointF(-200 - 5, 200 - 7.5));
}

void TestPosition::testSetAbsolutePosition2()
{
    shape1->rotate(90);
    shape1->setAbsolutePosition(QPointF(100, 100));
    QCOMPARE(shape1->absolutePosition(), QPointF(100, 100));

    shape1->setAbsolutePosition(QPointF(100, 100), KoFlake::TopLeftCorner);
    QCOMPARE(shape1->absolutePosition(KoFlake::TopLeftCorner), QPointF(100, 100));

    childShape1->setAbsolutePosition(QPointF(0, 0), KoFlake::BottomRightCorner);
    QCOMPARE(childShape1->position(), QPointF(-150, -150));

    childShape1->setAbsolutePosition(QPointF(0, 0), KoFlake::BottomLeftCorner);
    QCOMPARE(childShape1->position(), QPointF(-100, -150));

    childShape1->setAbsolutePosition(QPointF(0, 0), KoFlake::TopRightCorner);
    QCOMPARE(childShape1->position(), QPointF(-150, -100));

    container2->setClipped(childShape2, true);
    childShape2->setAbsolutePosition(QPointF(0, 0), KoFlake::TopLeftCorner);
    QCOMPARE(childShape2->position(), QPointF(-200, 200));
}

void TestPosition::testSetAndGetRotation()
{
    shape1->rotate(180);
    QCOMPARE(shape1->rotation(), 180.0);
    shape1->rotate(2);
    QCOMPARE(shape1->rotation(), 182.0);
    shape1->rotate(4);
    QCOMPARE(shape1->rotation(), 186.0);
    shape1->rotate(358);
    QCOMPARE(shape1->rotation(), 184.0);
}

QTEST_MAIN(TestPosition)
#include "TestPosition.moc"
