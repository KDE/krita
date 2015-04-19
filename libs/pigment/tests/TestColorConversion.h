#ifndef TestColorConversion_H
#define TestColorConversion_H

#include <QtTest>

class TestColorConversion : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRGBHSV();
    void testRGBHSL();
};

#endif
