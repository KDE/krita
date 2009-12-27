/* This file is part of the KDE project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2008 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QFile>
#include <QDir>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include <KoStore.h>
#include <KoEncryptionChecker.h>
#include <kdebug.h>
#include <stdlib.h>

#include <string.h>

#include <qtest_kde.h>

class TestStorage : public QObject
{
    Q_OBJECT
private slots:
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

    QTest::newRow("tar") << (int) KoStore::Tar << "test.tgz";
    QTest::newRow("directory") << (int) KoStore::Directory << "testdir/maindoc.xml";
    QTest::newRow("zip") << (int) KoStore::Zip <<"test.zip";
#ifdef QCA2
    if (KoEncryptionChecker::isEncryptionSupported()) {
        QTest::newRow("Encrypted") << (int) KoStore::Encrypted << "testEncrypted.zip";
    }
#endif
}

void TestStorage::storage()
{
    const char* const test1 = "This test checks whether we're able to write to some arbitrary directory.\n";
    const char* const testDir = "0";
    const char* const testDirResult = "0/";
    const char* const test2 = "This time we try to append the given relative path to the current dir.\n";
    const char* const test3 = "<xml>Hello World</xml>";
    const char* const testDir2 = "test2/with/a";
    const char* const testDir2Result = "0/test2/with/a/";
    const char* const test4 = "<xml>Heureka, it works</xml>";

    QFETCH(int, type);
    QFETCH(QString, testFile);
    KoStore::Backend backend = static_cast<KoStore::Backend>(type);

    if (QFile::exists(testFile))
        QFile::remove(testFile);

    QDir dirTest(testFile);
    if (dirTest.exists()) {
#ifdef Q_OS_UNIX
        system(QByteArray("rm -rf ") + QFile::encodeName(testFile));       // QDir::rmdir isn't recursive!
#else
        QFAIL("build dir not empty");
#endif
    }

    KoStore* store = KoStore::createStore(testFile, KoStore::Write, "", backend);
    QVERIFY(store);
    QVERIFY(store->bad() == false);

    if (store->isEncrypted())
        store->setPassword("password");

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

    if (store->isEncrypted())
        store->setPassword("password");

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

    QTest::newRow("tar") << (int) KoStore::Tar << "test.tgz";
    QTest::newRow("directory") << (int) KoStore::Directory << "testdir/maindoc.xml";
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
        system(QByteArray("rm -rf ") + QFile::encodeName(testFile));       // QDir::rmdir isn't recursive!
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

QTEST_KDEMAIN(TestStorage, NoGUI)
#include <TestStorage.moc>

