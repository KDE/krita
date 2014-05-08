/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoResourceBundleManager_test.h"
#include "KoResourceBundleManager.h"
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"
#include <QTest>
#include <QFile>
#include <qtest_kde.h>


void KoResourceBundleManager_test::initTestCase()
{
    env = QProcessEnvironment::systemEnvironment().value("HOME");
}

void KoResourceBundleManager_test::ctorTest()
{
    //Without parameter

    man = new KoResourceBundleManager();
    QCOMPARE(man->getKritaPath(), QString(""));
    QCOMPARE(man->getPackName(), QString(""));
    QVERIFY(man->m_resourceStore == 0);

    //Empty parameters

    man = new KoResourceBundleManager();
    QCOMPARE(man->getKritaPath(), QString(""));
    QCOMPARE(man->getPackName(), QString(""));
    QVERIFY(man->m_resourceStore == 0);

    //KritaPath specified

    man = new KoResourceBundleManager(env);
    QCOMPARE(man->getKritaPath(), QString(env + "/"));
    QCOMPARE(man->getPackName(), QString(""));
    QVERIFY(man->m_resourceStore == 0);

    //PackName specified

    man = new KoResourceBundleManager(env, env + "/monPaquet.bundle");
    QCOMPARE(man->getKritaPath(), QString(env + "/"));
    QCOMPARE(man->getPackName(), QString(env + "/monPaquet.bundle"));
    QVERIFY(!man->m_resourceStore->bad());

    delete man;
}

void KoResourceBundleManager_test::setReadPackTest()
{
    QFile::remove(env + "/monPaquet.bundle");
    man = new KoResourceBundleManager();
    man->setReadPack(env + "/monPaquet.bundle");
    QVERIFY(man->m_resourceStore->bad());
    KoStore::createStore(env + "/monPaquet.bundle", KoStore::Write, "", KoStore::Zip)->finalize();
    man->setReadPack(env + "/monPaquet.bundle");
    QVERIFY(!man->m_resourceStore->bad());
    QFile::remove(env + "/monPaquet.bundle");
}

void KoResourceBundleManager_test::setWritePackTest()
{
    QFile::remove(env + "/monPaquet.bundle");
    man = new KoResourceBundleManager();
    man->setWritePack(env + "/monPaquet.bundle");
    QVERIFY(!man->m_resourceStore->bad());
    man->finalize();
    man->setReadPack(env + "/monPaquet.bundle");
    QVERIFY(!man->m_resourceStore->bad());
    man->setWritePack(env + "/monPaquet.bundle");
    QVERIFY(!man->m_resourceStore->bad());
    QVERIFY(QFile::remove(env + "/monPaquet.bundle"));
}

void KoResourceBundleManager_test::setKritaPathTest()
{
    man = new KoResourceBundleManager();
    man->setKritaPath(env + "/");
    QCOMPARE(man->getKritaPath(), QString(env + "/"));
    man->setKritaPath("");
    QCOMPARE(man->getKritaPath(), QString(""));
    man->setKritaPath(env);
    QCOMPARE(man->getKritaPath(), QString(env + "/"));
}

void KoResourceBundleManager_test::isPathSetTest()
{
    man = new KoResourceBundleManager();
    QVERIFY(!man->isPathSet());
    man->setKritaPath(env + "/");
    QVERIFY(man->isPathSet());
    man->setKritaPath("");
    QVERIFY(!man->isPathSet());
    man->setKritaPath(QString());
    QVERIFY(!man->isPathSet());
}

void KoResourceBundleManager_test::toRootTest()
{
    man = new KoResourceBundleManager(env, env + "/monPaquet.bundle");
    man->toRoot();
    QVERIFY(man->m_resourceStore->currentDirectory().isEmpty() || man->m_resourceStore->currentDirectory() == QString("./"));
    man->m_resourceStore->enterDirectory("Test_Dir_1");
    QVERIFY(!(man->m_resourceStore->currentDirectory().isEmpty() || man->m_resourceStore->currentDirectory() == QString("./")));
    man->toRoot();
    QVERIFY(man->m_resourceStore->currentDirectory().isEmpty() || man->m_resourceStore->currentDirectory() == QString("./"));

    QString prov = "test";
    for (int i = 0; i < 10; i++) {
        prov += "/test";
    }

    QFile fileTest(env + "/testFile.txt");
    fileTest.open(QIODevice::ReadWrite);
    man->m_resourceStore->addLocalFile(env + "/testFile.txt", prov + "/testFile.txt");

    man->toRoot();
    QVERIFY(man->m_resourceStore->currentDirectory().isEmpty() || man->m_resourceStore->currentDirectory() == QString("./"));

    QFile::remove(env + "/testFile.txt");
}

void KoResourceBundleManager_test::addKFileTest()
{
    QFile::remove(env + "/monPaquet.bundle");
    QDir dir;
    QFile::remove(env + "/brushes/example.txt");
    dir.rmdir(env + "/brushes");

    man = new KoResourceBundleManager(env, env + "/monPaquet.bundle");
    dir.mkdir(env + "/brushes");
    QFile file(env + "/brushes/example.txt");
    file.open(QIODevice::ReadWrite);

    man->addKFile(env + "/brushes/example.txt");
    QVERIFY(man->m_resourceStore->hasFile("/brushes/example.txt"));
    man->finalize();

    man->setReadPack(env + "/monPaquet.bundle");
    QVERIFY(man->open("/brushes/example.txt"));
    QCOMPARE(man->size(), file.size());
    man->close();
    file.close();

    QFile::remove(env + "/brushes/example.txt");
    dir.rmdir(env + "/brushes");
    QFile::remove(env + "/monPaquet.bundle");
}

void KoResourceBundleManager_test::addKFileBundleTest()
{
    QFile::remove(env + "/monPaquet.bundle");
    QDir dir;
    QFile::remove(env + "/brushes/example.txt");
    dir.rmdir(env + "/brushes/pack");
    dir.rmdir(env + "/brushes");

    man = new KoResourceBundleManager(env, env + "/monPaquet.bundle");
    dir.mkpath(env + "/brushes/pack");
    QFile file(env + "/brushes/pack/example.txt");
    file.open(QIODevice::ReadWrite);

    man->addKFileBundle(env + "/brushes/pack/example.txt");
    QVERIFY(man->m_resourceStore->hasFile("/brushes/example.txt"));
    man->finalize();

    man->setReadPack(env + "/monPaquet.bundle");
    QVERIFY(man->open("/brushes/example.txt"));
    QCOMPARE(man->size(), file.size());
    man->close();
    file.close();

    QFile::remove(env + "/brushes/pack/example.txt");
    dir.rmdir(env + "/brushes/pack");
    dir.rmdir(env + "/brushes");
    QFile::remove(env + "/monPaquet.bundle");
}

void KoResourceBundleManager_test::addKFilesTest()
{
    QFile::remove(env + "/monPaquet.bundle");
    QDir dir;
    QList<QString> liste;
    QFile::remove(env + "/brushes/example.txt");
    QFile::remove(env + "/brushes/example2.txt");
    QFile::remove(env + "/brushes/example3.txt");
    dir.rmdir(env + "/brushes");

    man = new KoResourceBundleManager(env, env + "/monPaquet.bundle");
    dir.mkdir(env + "/brushes");
    QFile file(env + "/brushes/example.txt");
    file.open(QIODevice::ReadWrite);

    QFile file2(env + "/brushes/example2.txt");
    file2.open(QIODevice::ReadWrite);

    QFile file3(env + "/brushes/example3.txt");
    file3.open(QIODevice::ReadWrite);

    liste.push_back(file.fileName());
    liste.push_back(file2.fileName());
    liste.push_back(file3.fileName());

    man->addKFiles(liste);
    QVERIFY(man->m_resourceStore->hasFile("/brushes/example.txt")
            && man->m_resourceStore->hasFile("/brushes/example2.txt")
            && man->m_resourceStore->hasFile("/brushes/example3.txt"));
    man->finalize();

    man->setReadPack(env + "/monPaquet.bundle");

    QVERIFY(man->open("/brushes/example.txt"));
    QCOMPARE(man->size(), file.size());
    man->close();

    QVERIFY(man->open("/brushes/example2.txt"));
    QCOMPARE(man->size(), file2.size());
    man->close();

    QVERIFY(man->open("/brushes/example3.txt"));
    QCOMPARE(man->size(), file3.size());
    man->close();

    file.close();
    file2.close();
    file3.close();

    QFile::remove(env + "/brushes/example.txt");
    QFile::remove(env + "/brushes/example2.txt");
    QFile::remove(env + "/brushes/example3.txt");
    dir.rmdir(env + "/brushes");
    QFile::remove(env + "/monPaquet.bundle");
}

void KoResourceBundleManager_test::addManiMetaTest()
{
    KoXmlResourceBundleManifest* mani = new KoXmlResourceBundleManifest();
    KoXmlResourceBundleMeta* meta = new KoXmlResourceBundleMeta();

    meta->addTag("name", "NameExample", true);
    mani->install();

    man = new KoResourceBundleManager(env, env + "/monPaquet.bundle");
    man->addManiMeta(mani, meta);
    QVERIFY(man->m_resourceStore->hasFile("manifest.xml")
            && man->m_resourceStore->hasFile("meta.xml"));
    man->finalize();

    man->setReadPack(env + "/monPaquet.bundle");
    QVERIFY(man->open("/manifest.xml"));
    QCOMPARE((int)man->size(), mani->toByteArray().size());
    man->close();

    QVERIFY(man->open("/meta.xml"));
    QCOMPARE((int)man->size(), meta->toByteArray().size());
    man->close();

    QFile::remove(env + "/monPaquet.bundle");
}

void KoResourceBundleManager_test::getFileTest()
{
    QFile::remove(env + "/monPaquet.bundle");
    man = new KoResourceBundleManager(env, env + "/monPaquet.bundle");
    man->open("test.txt");
    man->write("Ceci est un fichier de test");
    man->close();
    man->finalize();
    man->setReadPack(env + "/monPaquet.bundle");
    man->open("test.txt");
    QByteArray val = man->read(man->size());
    man->close();
    QCOMPARE(man->getFile("test.txt")->readAll(), val);
    QFile::remove(env + "/monPaquet.bundle");
}

void KoResourceBundleManager_test::getFileDataTest()
{
    QFile::remove(env + "/monPaquet.bundle");
    man = new KoResourceBundleManager(env, env + "/monPaquet.bundle");
    man->open("test.txt");
    man->write("Ceci est un fichier de test");
    man->close();
    man->finalize();
    man->setReadPack(env + "/monPaquet.bundle");
    man->open("test.txt");
    QByteArray val = man->read(man->size());
    man->close();
    QCOMPARE(man->getFileData("test.txt"), val);
    QFile::remove(env + "/monPaquet.bundle");
}

void KoResourceBundleManager_test::cleanupTestCase()
{
    QStringList filters;
    filters << "monPaquet.*.new";

    QDir dir(env);
    dir.setNameFilters(filters);

    QStringList filesToBeRemoved = dir.entryList(dir.nameFilters(), QDir::Files);

    for (int i = 0; i < filesToBeRemoved.size(); i++) {
        dir.remove(filesToBeRemoved.at(i));
    }
}

QTEST_KDEMAIN(KoResourceBundleManager_test, GUI)
#include "KoResourceBundleManager_test.moc"
