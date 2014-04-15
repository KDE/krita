#ifndef KOXMLGENERATORTEST_H
#define KOXMLGENERATORTEST_H
#include <QtTest/QTest>
#include <KoConfig.h>

class KoXmlGeneratorTest: public QObject
{
    Q_OBJECT
private slots:
    void ctorTest();
    void getValueTest();
    void addTagTest();
    void removeFirstTagTest();
    void removeTagTest();	
    void searchValueTest();
    void toFileTest();
private:
 QString env;

};
#endif // KOXMLGENERATORTEST_H
