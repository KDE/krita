#ifndef TestPosition_H
#define TestPosition_H

#include <QtTest/QtTest>

class QPainter;
class KoViewConverter;
class KoShape;
class KoShapeContainer;

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

    // vars
    KoShape *shape1, *shape2, *childShape1, *childShape2;
    KoShapeContainer *container, *container2;
};

#endif
