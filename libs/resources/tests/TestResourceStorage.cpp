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

void TestResourceStorage::testImportResourceFile()
{
    QDir().mkpath(m_dstLocation + "/" + ResourceType::Patterns);
    KisResourceStorage folderStorage(m_dstLocation);

    QImage img(256, 256, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(0, 0, 256, 256, Qt::red);
    img.save("testpattern.png");

    QFile f("testpattern.png");
    f.open(QFile::ReadOnly);
    QByteArray ba = f.readAll();
    f.close();

    QString md5 = KoMD5Generator::generateHash(ba);

    bool r = folderStorage.importResourceFile(ResourceType::Patterns, "testpattern.png");
    QVERIFY(r);
    QCOMPARE(md5, folderStorage.resourceMd5("patterns/testpattern.png"));

    KisResourceStorage memoryStorage("memory");
    r = memoryStorage.importResourceFile(ResourceType::Patterns, "testpattern.png");
    QVERIFY(r);
    QCOMPARE(md5, memoryStorage.resourceMd5("patterns/testpattern.png"));

    KisResourceStorage bundleStorage(QString(FILES_DATA_DIR) + "/bundles/test1.bundle");
    r = bundleStorage.importResourceFile(ResourceType::Patterns, "testpattern.png");
    QVERIFY(!r);
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

void TestResourceStorage::cleanupTestCase()
{
    ResourceTestHelper::cleanDstLocation(FILES_DEST_DIR);
}

SIMPLE_TEST_MAIN(TestResourceStorage)

