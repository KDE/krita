#ifndef TESTKOLCMSCOLORPROFILE_H
#define TESTKOLCMSCOLORPROFILE_H

#include <QtTest>

class TestKoLcmsColorProfile : public QObject
{
    Q_OBJECT
private slots:
    void testChromaticitiesFromProfile();
    void testProfileCreationFromChromaticities();
};

#endif
