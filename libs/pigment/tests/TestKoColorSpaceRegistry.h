#ifndef TESTKOCOLORSPACEREGISTRY_H
#define TESTKOCOLORSPACEREGISTRY_H

#include <QtTest/QtTest>

class TestKoColorSpaceRegistry : public QObject
{
    Q_OBJECT
private slots:
    void testConstruction();
    void testRgbU8();
    void testRgbU16();
    void testLab();
};

#endif
