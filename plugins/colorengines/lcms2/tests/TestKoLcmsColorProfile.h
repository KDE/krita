#ifndef TESTKOLCMSCOLORPROFILE_H
#define TESTKOLCMSCOLORPROFILE_H

#include <QObject>

class TestKoLcmsColorProfile : public QObject
{
    Q_OBJECT
    // commented out since I don't know when...
    void testChromaticitiesFromProfile();
    void testProfileCreationFromChromaticities();
private Q_SLOTS:
    void testConversion();
};

#endif
