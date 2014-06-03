#ifndef TESTKOCOLORSPACEMATHS_H
#define TESTKOCOLORSPACEMATHS_H

#include <QtTest>

class TestKoColorSpaceMaths : public QObject
{
    Q_OBJECT
private slots:
    void testColorSpaceMathsTraits();
    void testScaleToA();
};

#endif
