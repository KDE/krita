/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2002 Werner Trobin <trobin@kde.org>
   SPDX-FileCopyrightText: 2008 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFile>
#include <QDir>

#include <KoStore.h>
#include <OdfDebug.h>
#include <stdlib.h>

#include <string.h>

#include <QTest>

class TestStorage : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void storage_data();
    void storage();
    void storage2_data();
    void storage2();

private:
    char getch(QIODevice * dev);
};


char TestStorage::getch(QIODevice * dev)
{
    char c = 0;
    dev->getChar(&c);
    return c;
}

void TestStorage::storage_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("testFile");

    QTest::newRow("directory") << (int) KoStore::Directory << "testdir";
    QTest::newRow("zip") << (int) KoStore::Zip <<"test.zip";
}

void TestStorage::storage()
{
    const char test1[] = "This test checks whether we're able to write to some arbitrary directory.\n";
    const char testDir[] = "0";
    const char testDirResult[] = "0/";
    const char test2[] = "This time we try to append the given relative path to the current dir.\n";
    const char test3[] = "<xml>Hello World</xml>";
    const char testDir2[] = "test2/with/a";
    const char testDir2Result[] = "0/test2/with/a/";
    const char test4[] = "<xml>Heureka, it works</xml>";

    QFETCH(int, type);
    QFETCH(QString, testFile);
    KoStore::Backend backend = static_cast<KoStore::Backend>(type);

    if (QFile::exists(testFile))
        QFile::remove(testFile);

    QDir dirTest(testFile);
    if (dirTest.exists()) {
#ifdef Q_OS_UNIX
        QByteArray ba = QByteArray("rm -rf ") + QFile::encodeName(testFile);
        Q_UNUSED(system(ba.constData()));       // QDir::rmdir isn't recursive!
#else
        QFAIL("build dir not empty");
#endif
    }

    KoStore* store = KoStore::createStore(testFile, KoStore::Write, "", backend);
    QVERIFY(store);
    QVERIFY(store->bad() == false);

    QVERIFY(store->open("test1/with/a/relative/dir.txt"));
    for (int i = 0; i < 100; ++i)
        store->write(test1, strlen(test1));
    store->close();

    store->enterDirectory(testDir);
    QCOMPARE(store->currentPath(), QString(testDirResult));

    QVERIFY(store->open("test2/with/a/relative/dir.txt"));
    for (int i = 0; i < 100; ++i)
        store->write(test2, strlen(test2));
    store->close();

    QVERIFY(store->open("root"));
    store->write(test3, strlen(test3));
    store->close();

    store->enterDirectory(testDir2);
    QCOMPARE(store->currentPath(), QString(testDir2Result));

    QVERIFY(store->open("root"));
    store->write(test4, strlen(test4));
    store->close();

    if (store->isOpen())
        store->close();
    delete store;

    store = KoStore::createStore(testFile, KoStore::Read, "", backend);
    QVERIFY(store->bad() == false);

    QVERIFY (store->open("test1/with/a/relative/dir.txt"));
    QIODevice* dev = store->device();
    int i = 0,  lim = strlen(test1),  count = 0;
    while (static_cast<char>(getch(dev)) == test1[i++]) {
        if (i == lim) {
            i = 0;
            ++count;
        }
    }
    store->close();
    QCOMPARE(count, 100);

    store->enterDirectory(testDir);
    QCOMPARE (store->currentPath(), QString(testDirResult));

    QVERIFY (store->open("test2/with/a/relative/dir.txt"));
    dev = store->device();
    i = 0;
    lim = strlen(test2);
    count = 0;
    while (static_cast<char>(getch(dev)) == test2[i++]) {
        if (i == lim) {
            i = 0;
            ++count;
        }
    }
    store->close();
    QCOMPARE(count, 100);

    store->enterDirectory(testDir2);
    store->pushDirectory();

    while (store->leaveDirectory()) {
        ;
    }
    store->enterDirectory(testDir);
    QCOMPARE (store->currentPath(), QString(testDirResult));

    QVERIFY (store->open("root"));
    QCOMPARE (store->size(), (qint64) 22);
    dev = store->device();
    QByteArray dataReadBack = dev->read(strlen(test3));
    store->close();
    QCOMPARE (dataReadBack, QByteArray(test3));

    store->popDirectory();
    QCOMPARE(store->currentPath(), QString(testDir2Result));

    QVERIFY (store->hasFile("relative/dir.txt"));
    
    QVERIFY (store->open("root"));
    char buf[29];
    store->read(buf, 28);
    buf[28] = '\0';
    store->close();
    QVERIFY(strncmp(buf, test4, 28) == 0);

    if (store->isOpen())
        store->close();
    delete store;
    QFile::remove(testFile);
}

#define DATALEN 64
void TestStorage::storage2_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("testFile");

    QTest::newRow("directory") << (int) KoStore::Directory << "testdir";
    QTest::newRow("zip") << (int) KoStore::Zip <<"test.zip";
}

void TestStorage::storage2()
{
    QFETCH(int, type);
    QFETCH(QString, testFile);
    KoStore::Backend backend = static_cast<KoStore::Backend>(type);

    if (QFile::exists(testFile))
        QFile::remove(testFile);

    QDir dirTest(testFile);
    if (dirTest.exists()) {
#ifdef Q_OS_UNIX
        QByteArray ba = QByteArray("rm -rf ") + QFile::encodeName(testFile);
        Q_UNUSED(system(ba.constData()));       // QDir::rmdir isn't recursive!
#else
        QFAIL("build dir not empty");
#endif
    }

    KoStore* store = KoStore::createStore(testFile, KoStore::Write, "", backend);
    QVERIFY(store->bad() == false);

    // Write
    QVERIFY (store->open("layer"));
    char str[DATALEN];

    sprintf(str, "1,2,3,4\n");
    store->write(str, strlen(str));
    memset(str, '\0', DATALEN);
    store->write(str, DATALEN);

    store->close();
    delete store;

    store = KoStore::createStore(testFile, KoStore::Read, "", backend);
    QVERIFY(store->bad() == false);
    // Read back
    QVERIFY (store->open("layer"));
    char str2[DATALEN];
    QIODevice *stream = store->device(); // << Possible suspect!

    stream->readLine(str2, DATALEN);      // << as is this
    qint64 len = store->read(str2, DATALEN);
    QCOMPARE(len, (qint64) DATALEN);
    store->close();
    delete store;

    QFile::remove(testFile);
}

QTEST_GUILESS_MAIN(TestStorage)
#include <TestStorage.moc>

