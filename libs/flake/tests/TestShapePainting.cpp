#include "TestShapePainting.h"

#include <QtGui>
#include "KoShapeContainer.h"
#include "KoShapeManager.h"
#include "MockShapes.h"

#include <kcomponentdata.h>

void TestShapePainting::testPaintShape() {
    MockShape shape1;
    MockShape shape2;
    MockContainer container;

    KComponentData componentData( "TestShapePainting" );  // we need an instance for that canvas

    container.addChild(&shape1);
    container.addChild(&shape2);
    QCOMPARE(shape1.parent(), &container);
    QCOMPARE(shape2.parent(), &container);
    container.setClipping(&shape1, false);
    container.setClipping(&shape2, false);
    QCOMPARE(container.childClipped(&shape1), false);
    QCOMPARE(container.childClipped(&shape2), false);

    MockCanvas canvas;
    KoShapeManager manager(&canvas);
    manager.add(&container);
    QCOMPARE(manager.shapes().count(), 3);

    QImage image(100, 100,  QImage::Format_Mono);
    QPainter painter(&image);
    MockViewConverter vc;
    manager.paint(painter, vc, false);

    // with the shape not being clipped, the shapeManager will paint it for us.
    QCOMPARE(shape1.paintedCount, 1);
    QCOMPARE(shape2.paintedCount, 1);
    QCOMPARE(container.paintedCount, 1);

    // the container should thus not paint the shape
    shape1.paintedCount = 0;
    shape2.paintedCount = 0;
    container.paintedCount = 0;
    container.paint(painter, vc);
    QCOMPARE(shape1.paintedCount, 0);
    QCOMPARE(shape2.paintedCount, 0);
    QCOMPARE(container.paintedCount, 1);


    container.setClipping(&shape1, false);
    container.setClipping(&shape2, true);
    QCOMPARE(container.childClipped(&shape1), false);
    QCOMPARE(container.childClipped(&shape2), true);

    shape1.paintedCount = 0;
    shape2.paintedCount = 0;
    container.paintedCount = 0;
    manager.paint(painter, vc, false);


    // with this shape not being clipped, the shapeManager will paint the container and this shape
    QCOMPARE(shape1.paintedCount, 1);
    // with this shape being clipped, the container will paint it for us.
    QCOMPARE(shape2.paintedCount, 1);
    QCOMPARE(container.paintedCount, 1);
}

void TestShapePainting::testPaintHiddenShape() {
    MockShape shape;
    MockContainer fourth;
    MockContainer thirth;
    MockContainer second;
    MockContainer top;

    top.addChild(&second);
    second.addChild(&thirth);
    thirth.addChild(&fourth);
    fourth.addChild(&shape);

    second.setVisible(false);

    MockCanvas canvas;
    KoShapeManager manager(&canvas);
    manager.add(&top);
    QCOMPARE(manager.shapes().count(), 5);

    QImage image(100, 100,  QImage::Format_Mono);
    QPainter painter(&image);
    MockViewConverter vc;
    manager.paint(painter, vc, false);

    QCOMPARE(top.paintedCount, 1);
    QCOMPARE(second.paintedCount, 0);
    QCOMPARE(thirth.paintedCount, 0);
    QCOMPARE(fourth.paintedCount, 0);
    QCOMPARE(shape.paintedCount, 0);
}

QTEST_MAIN(TestShapePainting)
#include "TestShapePainting.moc"
