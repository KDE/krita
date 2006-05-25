#ifndef TestPosition_H
#define TestPosition_H

#include <QtTest/QtTest>

#include <KoShape.h>
#include <KoShapeContainer.h>

class QPainter;
class KoViewConverter;

class TestPosition : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    // tests
    void testBasePosition();
    void testAbsolutePosition();
    void testSetAbsolutePosition();

private:
    void resetValues();
    void resetValues(KoShape *shape);
    class MockShape : public KoShape {
        void paint(QPainter &painter, KoViewConverter &converter) {
            Q_UNUSED(painter);
            Q_UNUSED(converter);
        }
    };

    class MockContainer : public KoShapeContainer {
        void paintComponent(QPainter &painter, KoViewConverter &converter) {
            Q_UNUSED(painter);
            Q_UNUSED(converter);
        }
    };

    // vars
    KoShape *shape1, *shape2, *childShape1, *childShape2;
    KoShapeContainer *container, *container2;
};

#endif
