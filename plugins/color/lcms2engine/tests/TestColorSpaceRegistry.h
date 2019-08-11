#ifndef TESTKOCOLORSPACEREGISTRY_H
#define TESTKOCOLORSPACEREGISTRY_H

#include <QObject>

class TestColorSpaceRegistry : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testConstruction();
    void testRgbU8();
    void testRgbU16();
    void testLab();
};

#endif
