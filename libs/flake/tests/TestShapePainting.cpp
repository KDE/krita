#include "TestShapePainting.h"

#include <QtGui>
#include "KoShapeContainer.h"
#include "KoShapeManager.h"
#include "MockShapes.h"

#include <kcomponentdata.h>

void TestShapePainting::testPaintShape()
{
    MockShape shape1;
    MockShape shape2;
    MockContainer container;

    KComponentData componentData("TestShapePainting");    // we need an instance for that canvas

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

void TestShapePainting::testPaintHiddenShape()
{
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

void TestShapePainting::testPaintOrder()
{
    // the stacking order determines the painting order so things on top
    // get their paint called last.
    // Each shape has a zIndex and within the children a container has
    // it determines the stacking order. Its important to realize that
    // the zIndex is thus local to a container, if you have layer1 and layer2
    // with both various child shapes the stacking order of the layer shapes
    // is most important, then within this the child shape index is used.

    class OrderedMockShape : public MockShape {
    public:
        OrderedMockShape(QList<MockShape*> &list) : order(list) {}
        void paint(QPainter &painter, const KoViewConverter &converter) {
            order.append(this);
            MockShape::paint(painter, converter);
        }
        QList<MockShape*> &order;
    };

    QList<MockShape*> order;

    MockContainer top;
    top.setZIndex(2);
    OrderedMockShape shape1(order);
    shape1.setZIndex(5);
    OrderedMockShape shape2(order);
    shape2.setZIndex(0);
    top.addChild(&shape1);
    top.addChild(&shape2);

    MockContainer bottom;
    bottom.setZIndex(1);
    OrderedMockShape shape3(order);
    shape3.setZIndex(-1);
    OrderedMockShape shape4(order);
    shape4.setZIndex(9);
    bottom.addChild(&shape3);
    bottom.addChild(&shape4);

    MockCanvas canvas;
    KoShapeManager manager(&canvas);
    manager.add(&top);
    manager.add(&bottom);
    QCOMPARE(manager.shapes().count(), 6);

    QImage image(100, 100,  QImage::Format_Mono);
    QPainter painter(&image);
    MockViewConverter vc;
    manager.paint(painter, vc, false);
    QCOMPARE(top.paintedCount, 1);
    QCOMPARE(bottom.paintedCount, 1);
    QCOMPARE(shape1.paintedCount, 1);
    QCOMPARE(shape2.paintedCount, 1);
    QCOMPARE(shape3.paintedCount, 1);
    QCOMPARE(shape4.paintedCount, 1);

    QCOMPARE(order.count(), 4);
    QVERIFY(order[0] == &shape3); // lowest first
    QVERIFY(order[1] == &shape4);
    QVERIFY(order[2] == &shape2);
    QVERIFY(order[3] == &shape1);

    // again, with clipping.
    order.clear();
    painter.setClipRect(0, 0, 100, 100);
    manager.paint(painter, vc, false);
    QCOMPARE(top.paintedCount, 2);
    QCOMPARE(bottom.paintedCount, 2);
    QCOMPARE(shape1.paintedCount, 2);
    QCOMPARE(shape2.paintedCount, 2);
    QCOMPARE(shape3.paintedCount, 2);
    QCOMPARE(shape4.paintedCount, 2);

    QCOMPARE(order.count(), 4);
    QVERIFY(order[0] == &shape3); // lowest first
    QVERIFY(order[1] == &shape4);
    QVERIFY(order[2] == &shape2);
    QVERIFY(order[3] == &shape1);
}

QTEST_MAIN(TestShapePainting)
#include "TestShapePainting.moc"
