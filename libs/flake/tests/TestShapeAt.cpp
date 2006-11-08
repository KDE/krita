#include "TestShapeAt.h"
#include "MockShapes.h"

#include <kdebug.h>
#include <KoShapeManager.h>
#include <KoSelection.h>

void TestShapeAt::test() {
    MockShape shape1;
    MockShape shape2;
    MockShape shape3;

    MockCanvas canvas;
    KoShapeManager manager(&canvas);
    shape1.setPosition(QPointF(100, 100));
    shape1.resize(QSizeF(50, 50));
    shape1.setZIndex(0);
    manager.add(&shape1);

    QVERIFY(manager.shapeAt(QPointF(90, 90)) == 0);
    QVERIFY(manager.shapeAt(QPointF(110, 140)) != 0);
    QVERIFY(manager.shapeAt(QPointF(100, 100)) != 0);
    QVERIFY(manager.shapeAt(QPointF(100, 100), KoFlake::Selected) == 0);
    QVERIFY(manager.shapeAt(QPointF(100, 100), KoFlake::Unselected) != 0 );
    QVERIFY(manager.shapeAt(QPointF(100, 100), KoFlake::NextUnselected) != 0 );

    shape2.setPosition(QPointF(80, 80));
    shape2.resize(QSizeF(50, 50));
    shape2.setZIndex(1);
    manager.add(&shape2);

    QVERIFY(manager.shapeAt(QPointF(200, 200)) == 0);
    QCOMPARE(manager.shapeAt(QPointF(90, 90)), &shape2);
    QCOMPARE(manager.shapeAt(QPointF(105, 105)), &shape2); // the one on top
    KoShape *dummy = 0;
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::Selected), dummy);
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::Unselected), &shape2); // the one on top
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::NextUnselected), &shape2);

    manager.selection()->select(&shape2);

    QVERIFY(manager.shapeAt(QPointF(200, 200)) == 0);
    QCOMPARE(manager.shapeAt(QPointF(90, 90)), &shape2);
    QCOMPARE(manager.shapeAt(QPointF(105, 105)), &shape2); // the one on top
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::Selected), &shape2);
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::Unselected), &shape1);
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::NextUnselected), &shape1);

    shape3.setPosition(QPointF(120, 80));
    shape3.resize(QSizeF(50, 50));
    shape3.setZIndex(2);
    manager.add(&shape3);

    QVERIFY(manager.shapeAt(QPointF(200, 200)) == 0);
    QCOMPARE(manager.shapeAt(QPointF(90, 90)), &shape2);

    QVERIFY(manager.shapeAt(QPointF(200, 200)) == 0);
    QCOMPARE(manager.shapeAt(QPointF(90, 90)), &shape2);
    QCOMPARE(manager.shapeAt(QPointF(105, 145)), &shape1);
    QCOMPARE(manager.shapeAt(QPointF(165, 90)), &shape3);

    QCOMPARE(manager.shapeAt(QPointF(125, 105)), &shape3); // the one on top
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::Selected), &shape2);
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::Unselected), &shape1);
    QCOMPARE(manager.shapeAt(QPointF(105, 105), KoFlake::NextUnselected), &shape1);

    QCOMPARE(manager.shapeAt(QPointF(125, 105), KoFlake::Selected), &shape2);
    QCOMPARE(manager.shapeAt(QPointF(125, 105), KoFlake::Unselected), &shape3);
    QCOMPARE(manager.shapeAt(QPointF(125, 105), KoFlake::NextUnselected), &shape1);
}

QTEST_MAIN(TestShapeAt)
#include "TestShapeAt.moc"
