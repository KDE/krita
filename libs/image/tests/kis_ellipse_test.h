#ifndef KRITA_KIS_BUGGY_TEST_H
#define KRITA_KIS_BUGGY_TEST_H

#include <simpletest.h>

class KisEllipseTest : public QObject
{
Q_OBJECT
private Q_SLOTS:

    void testDrawing();

    void testPrecision();
};

#endif //KRITA_KIS_BUGGY_TEST_H
