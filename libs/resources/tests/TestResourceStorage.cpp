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
    KisResourceStorage storage(m_dstLocation);

    QImage img(256, 256, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(0, 0, 256, 256, Qt::red);
    img.save("testpattern.png");

    QFile f("testpattern.png");
    f.open(QFile::ReadOnly);
    QByteArray ba = f.readAll();
    f.close();

    QByteArray md5 = KoMD5Generator::generateHash(ba);

    bool r = storage.importResourceFile(ResourceType::Patterns, "testpattern.png");
    QVERIFY(r);
    QCOMPARE(md5.toHex(), storage.resourceMd5("patterns/testpattern.png").toHex());

}

void TestResourceStorage::cleanupTestCase()
{
    ResourceTestHelper::cleanDstLocation(FILES_DEST_DIR);
}

SIMPLE_TEST_MAIN(TestResourceStorage)

