#ifndef TESTSHAPEPAINT_H
#define TESTSHAPEPAINT_H

#include <QtTest/QtTest>

class TestShapePainting : public QObject
{
    Q_OBJECT
private slots:

    void testPaintShape();
    void testPaintHiddenShape();
    void testPaintOrder();
};

#endif
