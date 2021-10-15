/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestResourceStorage.h"

#include <simpletest.h>

#include <QImage>
#include <QPainter>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <KoMD5Generator.h>
#include <KoPattern.h>

#include "KisResourceLoaderRegistry.h"
#include "DummyResource.h"
#include "ResourceTestHelper.h"
#include "KisResourceStorage.h"
#include "KisResourceLocator.h"
#include "KisResourceTypes.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestResourceStorage::initTestCase()
{
    m_dstLocation = QString(FILES_DEST_DIR);
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
    QDir().mkpath(m_dstLocation);
    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    cfg.writeEntry(KisResourceLocator::resourceLocationKey, m_dstLocation);
    m_locator = KisResourceLocator::instance();
    ResourceTestHelper::createDummyLoaderRegistry();
}

void TestResourceStorage ::testStorage()
{
    {
        KisResourceStorage storage(QString(FILES_DATA_DIR));
        QVERIFY(storage.type() == KisResourceStorage::StorageType::Folder);
        QVERIFY(storage.valid());
    }

    {
        KisResourceStorage storage(QString(FILES_DATA_DIR) + "/bundles/test1.bundle");
        QVERIFY(storage.type() == KisResourceStorage::StorageType::Bundle);
        QVERIFY(storage.valid());
    }

    {
        KisResourceStorage storage(QString(FILES_DATA_DIR) + "/bundles/test2.bundle");
        QVERIFY(storage.type() == KisResourceStorage::StorageType::Bundle);
        QVERIFY(storage.valid());
    }

    {
        KisResourceStorage storage("");
        QVERIFY(storage.type() == KisResourceStorage::StorageType::Unknown);
        QVERIFY(!storage.valid());
    }
}

void TestResourceStorage::testImportExportResource()
{
    QImage img(256, 256, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(0, 0, 256, 256, Qt::red);
    img.save("testpattern.png");

    QByteArray ba;

    {
        QFile f("testpattern.png");
        f.open(QFile::ReadOnly);
        ba = f.readAll();
        f.close();
    }

    const QString md5 = KoMD5Generator::generateHash(ba);

    {
        QDir().mkpath(m_dstLocation + "/" + ResourceType::Patterns);
        KisResourceStorage folderStorage(m_dstLocation);

        QFile f("testpattern.png");
        f.open(QFile::ReadOnly);
        bool r = folderStorage.importResource("patterns/testpattern.png", &f);
        QVERIFY(r);
        QCOMPARE(md5, folderStorage.resourceMd5("patterns/testpattern.png"));

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        r = folderStorage.exportResource("patterns/testpattern.png", &buffer);
        QVERIFY(r);
        buffer.close();
        QCOMPARE(KoMD5Generator::generateHash(buffer.data()), md5);
    }

    {
        QFile f("testpattern.png");
        f.open(QFile::ReadOnly);
        KisResourceStorage memoryStorage("memory");
        bool r = memoryStorage.importResource("patterns/testpattern.png", &f);
        QVERIFY(r);
        QCOMPARE(md5, memoryStorage.resourceMd5("patterns/testpattern.png"));

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        r = memoryStorage.exportResource("patterns/testpattern.png", &buffer);
        QVERIFY(r);
        buffer.close();
        QCOMPARE(KoMD5Generator::generateHash(buffer.data()), md5);
    }

    {
        QFile f("testpattern.png");
        f.open(QFile::ReadOnly);
        KisResourceStorage bundleStorage(QString(FILES_DATA_DIR) + "/bundles/test1.bundle");
        bool r = bundleStorage.importResource("patterns/testpattern.png", &f);
        QVERIFY(!r);

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        r = bundleStorage.exportResource("patterns/testpattern.png", &buffer);
        QVERIFY(!r);
    }
}

void TestResourceStorage::testAddResource()
{
    QDir().mkpath(m_dstLocation + "/" + ResourceType::Patterns);
    KisResourceStorage folderStorage(m_dstLocation);

    QImage img(256, 256, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(0, 0, 256, 256, Qt::red);

    KoResourceSP res(new KoPattern(img, "testpattern2", "testpattern2.png"));
    Q_ASSERT(res->resourceType().first == ResourceType::Patterns);
    bool r =  folderStorage.addResource(res);
    QVERIFY(r);
}

void TestResourceStorage::testStorageVersioningHelperCounting()
{
    // create the resource
    QDir().mkpath(m_dstLocation + "/" + ResourceType::Patterns);

    QImage img(256, 256, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(0, 0, 256, 256, Qt::red);
    KoResourceSP res(new KoPattern(img, "testpattern", "testpattern.png"));
    Q_ASSERT(res->resourceType().first == ResourceType::Patterns);

    // function that returns false for everything
    auto noResourcesExisting = [] (QString a) {Q_UNUSED(a); return false;};
    QString resNewFilename = KisStorageVersioningHelper::chooseUniqueName(res, 0, noResourcesExisting);
    //QCOMPARE(resNewFilename, "testpattern.png");
    QCOMPARE(resNewFilename, "testpattern.0000.png");

    // function that returns true for the same resource but false for everything else
    auto onlyFirstVersionExists = [] (QString a) {
        return a == "testpattern.png" || a == "testpattern.0000.png";
    };
    resNewFilename = KisStorageVersioningHelper::chooseUniqueName(res, 0, onlyFirstVersionExists);
    QCOMPARE(resNewFilename, "testpattern.0001.png");

    // function that returns true for first 10 versions of the resource but false for everything else
    auto firstTenVersionExists = [] (QString a) {
        if (a == "testpattern.png") return true;
        if (a == "testpattern.0000.png") return true;
        if (a == "testpattern.0001.png") return true;
        if (a == "testpattern.0002.png") return true;
        if (a == "testpattern.0003.png") return true;
        if (a == "testpattern.0004.png") return true;
        if (a == "testpattern.0005.png") return true;
        if (a == "testpattern.0006.png") return true;
        if (a == "testpattern.0007.png") return true;
        if (a == "testpattern.0008.png") return true;
        if (a == "testpattern.0009.png") return true;
        if (a == "testpattern.0010.png") return true;
        return false;
    };
    resNewFilename = KisStorageVersioningHelper::chooseUniqueName(res, 0, firstTenVersionExists);
    QCOMPARE(resNewFilename, "testpattern.0011.png");

}

void TestResourceStorage::cleanupTestCase()
{
    ResourceTestHelper::cleanDstLocation(FILES_DEST_DIR);
}

SIMPLE_TEST_MAIN(TestResourceStorage)

