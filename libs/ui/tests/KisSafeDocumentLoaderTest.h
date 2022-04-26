#ifndef KISSAFEDOCUMENTLOADERTEST_H
#define KISSAFEDOCUMENTLOADERTEST_H

#include <QObject>

class KisSafeDocumentLoaderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test();
    void testFileLost();
};

#endif // KISSAFEDOCUMENTLOADERTEST_H
