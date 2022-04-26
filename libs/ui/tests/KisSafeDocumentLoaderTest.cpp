#include "KisSafeDocumentLoaderTest.h"

#include <testui.h>

#include "config-limit-long-tests.h"
#include "kis_safe_document_loader.h"
#include "kis_debug.h"

namespace {
void writeToFile(QFile &file, QColor color)
{
    file.reset();
    file.resize(0);
    QImage img(10,10,QImage::Format_ARGB32);
    img.fill(Qt::black);
    img.save(&file, "PNG");
    file.flush();
}

}

void KisSafeDocumentLoaderTest::test()
{
    QTemporaryFile file("safe_loader_test_XXXXXX.png");

    file.open();
    writeToFile(file, Qt::black);

    KisSafeDocumentLoader loader(file.fileName());

    QSignalSpy spy(&loader, &KisSafeDocumentLoader::loadingFinished);
    QSignalSpy spyFailed(&loader, &KisSafeDocumentLoader::loadingFailed);

    // reloadImage() is synchronous
    loader.reloadImage();
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spyFailed.size(), 0);

    writeToFile(file, Qt::white);

    spy.wait(1500);
    QCOMPARE(spy.size(), 2);
    QCOMPARE(spyFailed.size(), 0);

    spy.clear();

    file.reset();
    file.resize(0);
    file.write("blah-blah-try-read-me");
    file.flush();

    spyFailed.wait(6000);

    QCOMPARE(spy.size(), 0);
    QCOMPARE(spyFailed.size(), 1);

}

void KisSafeDocumentLoaderTest::testFileLost()
{
    QTemporaryFile file("safe_loader_test_XXXXXX.png");

    file.open();
    writeToFile(file, Qt::black);

    KisSafeDocumentLoader loader(file.fileName());

    QSignalSpy spy(&loader, &KisSafeDocumentLoader::loadingFinished);
    QSignalSpy spyFailed(&loader, &KisSafeDocumentLoader::loadingFailed);
    QSignalSpy spyExistsState(&loader, &KisSafeDocumentLoader::fileExistsStateChanged);

    // reloadImage() is synchronous
    loader.reloadImage();
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spyFailed.size(), 0);
    QCOMPARE(spyExistsState.size(), 0);

    spy.clear();

    file.close();
    file.remove();

    spyExistsState.wait(15000);
    QCOMPARE(spyExistsState.size(), 1);
    QCOMPARE(spyExistsState[0][0].toBool(), false);
    spyExistsState.clear();

    file.open();
    writeToFile(file, Qt::white);

    spyExistsState.wait(3500);
    QCOMPARE(spy.size(), 0);
    QCOMPARE(spyFailed.size(), 0);
    QCOMPARE(spyExistsState.size(), 1);
    QCOMPARE(spyExistsState[0][0].toBool(), true);
    spyExistsState.clear();

    spy.wait(1500);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spyFailed.size(), 0);
    QCOMPARE(spyExistsState.size(), 0);
    spy.clear();


    writeToFile(file, Qt::yellow);
    spy.wait(1500);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spyFailed.size(), 0);
    QCOMPARE(spyExistsState.size(), 0);
}

KISTEST_MAIN(KisSafeDocumentLoaderTest)
