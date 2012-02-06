#ifndef TestColorConversion_H
#define TestColorConversion_H

#include <QtTest>

class TestColorConversion : public QObject
{
    Q_OBJECT
private slots:
    void testRGBHSV();
    void testRGBHSL();
};

#endif
