/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestFolderStorage.h"
#include <simpletest.h>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KritaVersionWrapper.h>

#include <KisFolderStorage.h>
#include <KoResource.h>
#include <KisResourceCacheDb.h>

#include <KisResourceLocator.h>

#include "DummyResource.h"
#include "ResourceTestHelper.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestFolderStorage::initTestCase()
{
    ResourceTestHelper::initTestDb();

    m_srcLocation = QString(FILES_DATA_DIR);
    QVERIFY2(QDir(m_srcLocation).exists(), m_srcLocation.toUtf8());

    m_dstLocation = QString(FILES_DEST_DIR);
    ResourceTestHelper::cleanDstLocation(m_dstLocation);

    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    cfg.writeEntry(KisResourceLocator::resourceLocationKey, m_dstLocation);

    m_locator = KisResourceLocator::instance();

    ResourceTestHelper::createDummyLoaderRegistry();

    KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    m_locator->initialize(m_srcLocation);

    if (!m_locator->errorMessages().isEmpty()) qDebug() << m_locator->errorMessages();
}

void TestFolderStorage ::testStorage()
{

    KisFolderStorage folderStorage(m_dstLocation);
    QSharedPointer<KisResourceStorage::ResourceIterator> iter = folderStorage.resources(ResourceType::Brushes);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        qDebug() << iter->url() << iter->type() << iter->lastModified();
        count++;
    }
    QVERIFY(count == 1);
}

void TestFolderStorage::testTagIterator()
{
    KisFolderStorage folderStorage(m_dstLocation);
    QSharedPointer<KisResourceStorage::TagIterator> iter = folderStorage.tags(ResourceType::PaintOpPresets);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        qDebug() << iter->tag();
        count++;
    }
    QVERIFY(count == 1);
}

void TestFolderStorage::testAddResource()
{
    KoResourceSP resource(new DummyResource("anewresource.kpp", ResourceType::PaintOpPresets));
    resource->setValid(true);
    resource->setVersion(0);

    KisFolderStorage folderStorage(QString(FILES_DEST_DIR));
    bool r = folderStorage.saveAsNewVersion(ResourceType::PaintOpPresets, resource);
    QVERIFY(r);

    ResourceTestHelper::testVersionedStorage(folderStorage,
                                             ResourceType::PaintOpPresets,
                                             "paintoppresets/anewresource.0000.kpp",
                                             QString(FILES_DEST_DIR));
    ResourceTestHelper::testVersionedStorageIterator(folderStorage,
                                                     ResourceType::PaintOpPresets,
                                                     "paintoppresets/anewresource.0000.kpp");
}

void TestFolderStorage::testResourceCaseSensitivity()
{
    KoResourceSP resource(new DummyResource("resourcecasetest.kpp", ResourceType::PaintOpPresets));
    resource->setValid(true);
    resource->setVersion(0);

    KisFolderStorage folderStorage(QString(FILES_DEST_DIR));
    bool r = folderStorage.addResource(ResourceType::PaintOpPresets, resource);
    QVERIFY(r);

    KoResourceSP res1 = folderStorage.resource("paintoppresets/resourcecasetest.kpp");
    QVERIFY(res1);
    QCOMPARE(res1->filename(), "resourcecasetest.kpp");

    /**
     * In the folder storage we expect all resources to have
     * filesystem-dependent name resolution. That is, if we request a resource
     * with wrong casing, it should still fetch the resource. But the filename
     * field of the resource must contain the correct casing
     */

#if defined Q_OS_WIN || defined Q_OS_MACOS
    KoResourceSP res2 = folderStorage.resource("paintoppresets/ReSoUrCeCaSeTeSt.kpp");
    QVERIFY(res2);
    QCOMPARE(res2->filename(), "resourcecasetest.kpp");
#endif
}

void TestFolderStorage::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}

SIMPLE_TEST_MAIN(TestFolderStorage)

