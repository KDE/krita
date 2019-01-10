#ifndef TESTLCMSRGBP2020PQCOLORSPACE_H
#define TESTLCMSRGBP2020PQCOLORSPACE_H
#include <QObject>

class TestLcmsRGBP2020PQColorSpace : public QObject
{
  Q_OBJECT
private Q_SLOTS:
    void test();
    void testInternalConversions();
    void testConvertToCmyk();
};

#endif // TESTLCMSRGBP2020PQCOLORSPACE_H
