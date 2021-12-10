/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestResourceLocator.h"

#include <simpletest.h>
#include <QVersionNumber>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlQuery>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KritaVersionWrapper.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>
#include <KisResourceLoaderRegistry.h>
#include <KisMemoryStorage.h>
#include <KisResourceModel.h>
#include <KisResourceTypes.h>

#include <DummyResource.h>
#include <ResourceTestHelper.h>

#include <kis_debug.h>
#include <KisResourceModelProvider.h>
#include <KoMD5Generator.h>


#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif

void TestResourceLocator::initTestCase()
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
}

void TestResourceLocator::testLocatorInitialization()
{
    KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    KisResourceLocator::LocatorError r = m_locator->initialize(m_srcLocation);
    if (!m_locator->errorMessages().isEmpty()) qDebug() << m_locator->errorMessages();
    QVERIFY(r == KisResourceLocator::LocatorError::Ok);
    QVERIFY(QDir(m_dstLocation).exists());
    Q_FOREACH(const QString &folder, KisResourceLoaderRegistry::instance()->resourceTypes()) {
        QDir dstDir(m_dstLocation + '/' + folder + '/');
        QDir srcDir(m_srcLocation + '/' + folder + '/');

        QVERIFY(dstDir.exists());
        QVERIFY(dstDir.entryList(QDir::Files | QDir::NoDotAndDotDot) == srcDir.entryList(QDir::Files | QDir::NoDotAndDotDot));
    }

    QFile f(m_dstLocation + '/' + "KRITA_RESOURCE_VERSION");
    QVERIFY(f.exists());
    f.open(QFile::ReadOnly);
    QVersionNumber version = QVersionNumber::fromString(QString::fromUtf8(f.readAll()));
    QVERIFY(version == QVersionNumber::fromString(KritaVersionWrapper::versionString()));

}

void TestResourceLocator::testStorageInitialization()
{
    Q_FOREACH(KisResourceStorageSP storage, m_locator->storages()) {
        QVERIFY(KisResourceCacheDb::addStorage(storage, true));
    }
    QSqlQuery query;
    bool r = query.exec("SELECT COUNT(*) FROM storages");
    QVERIFY(r);
    QVERIFY(query.lastError() == QSqlError());
    query.first();
    QCOMPARE(query.value(0).toInt(), m_locator->storages().count());
}

void TestResourceLocator::testLocatorSynchronization()
{
    QVERIFY(m_locator->synchronizeDb());
    {
        QSqlQuery query;
        bool r = query.exec("SELECT COUNT(*) FROM resources");
        QVERIFY(r);
        QVERIFY(query.lastError() == QSqlError());
        query.first();
        QCOMPARE(query.value(0).toInt(), 7);
    }

    {
        QSqlQuery query;
        bool r = query.exec("SELECT COUNT(*) FROM tags");
        QVERIFY(r);
        QVERIFY(query.lastError() == QSqlError());
        query.first();
        QCOMPARE(query.value(0).toInt(), 1);
    }
}

void TestResourceLocator::testResourceLocationBase()
{
    QCOMPARE(m_locator->resourceLocationBase(), QString(FILES_DEST_DIR));
}

void TestResourceLocator::testResource()
{
    KoResourceSP res = m_locator->resource("", ResourceType::PaintOpPresets, "test0.kpp");
    QVERIFY(res);
}

void TestResourceLocator::testResourceForId()
{
    KoResourceSP res = m_locator->resource("", ResourceType::PaintOpPresets, "test0.kpp");
    int resourceId = KisResourceCacheDb::resourceIdForResource("test0.kpp", ResourceType::PaintOpPresets, "");
    QVERIFY(resourceId > -1);
    KoResourceSP res2 = m_locator->resourceForId(resourceId);
    QCOMPARE(res, res2);
}

void TestResourceLocator::testDocumentStorage()
{
    const QString &documentName("document");

    KisResourceModel model(ResourceType::PaintOpPresets);
    int rowcount = model.rowCount();

    KisResourceStorageSP documentStorage = QSharedPointer<KisResourceStorage>::create(documentName);
    QVERIFY(documentStorage->valid());
    KoResourceSP resource(new DummyResource("test.kpp", ResourceType::PaintOpPresets));
    documentStorage->saveAsNewVersion(resource);

    m_locator->addStorage(documentName, documentStorage);

    QVERIFY(m_locator->hasStorage(documentName));
    QVERIFY(model.rowCount() > rowcount);

    m_locator->removeStorage(documentName);
    QVERIFY(!m_locator->hasStorage(documentName));

    QVERIFY(model.rowCount() == rowcount);
}

void TestResourceLocator::testSyncVersions()
{
    int resourceId = -1;
    QString storageLocation;

    {
        KisResourceModel model(ResourceType::PaintOpPresets);
        model.setResourceFilter(KisResourceModel::ShowAllResources);

        KoResourceSP res = m_locator->resource("", ResourceType::PaintOpPresets, "test0.kpp");
        resourceId = KisResourceCacheDb::resourceIdForResource("test0.kpp", ResourceType::PaintOpPresets, "");
        storageLocation = res->storageLocation();

        // ENTER_FUNCTION() << ppVar(model.rowCount());
        // for (int i = 0; i < model.rowCount(); i++) {
        //     qDebug() << ppVar(model.data(model.index(i, KisResourceModel::Filename))) << model.data(model.index(i, KisResourceModel::Id));
        // }

        {
            bool result = m_locator->updateResource(res->resourceType().first, res);
            QVERIFY(result);
            QCOMPARE(res->version(), 1);
        }

        {
            bool result = m_locator->updateResource(res->resourceType().first, res);
            QVERIFY(result);
            QCOMPARE(res->version(), 2);
        }

        // ENTER_FUNCTION() << ppVar(model.rowCount());
        // for (int i = 0; i < model.rowCount(); i++) {
        //     qDebug() << ppVar(model.data(model.index(i, KisResourceModel::Filename))) << model.data(model.index(i, KisResourceModel::Id));
        // }

        QCOMPARE(model.rowCount(), 3);

        {
            KoResourceSP res1 = model.resourceForIndex(model.index(0, 0));
            QVERIFY(res1);
            QCOMPARE(res1->resourceId(), resourceId);
            QCOMPARE(res1->version(), 2);
        }

    }

    // test removing one version of the resource
    {
        bool result = QFile::remove(storageLocation + "/paintoppresets/test0.0002.kpp");
        QVERIFY(result);

        m_locator->synchronizeDb();
        KisResourceModelProvider::testingResetAllModels();

        KisResourceModel model(ResourceType::PaintOpPresets);
        model.setResourceFilter(KisResourceModel::ShowAllResources);

        // ENTER_FUNCTION() << ppVar(model.rowCount());
        // for (int i = 0; i < model.rowCount(); i++) {
        //     qDebug() << ppVar(model.data(model.index(i, KisResourceModel::Filename))) << model.data(model.index(i, KisResourceModel::Id));
        // }

        QCOMPARE(model.rowCount(), 3);

        KoResourceSP res1 = model.resourceForIndex(model.index(0, 0));
        QVERIFY(res1);
        QCOMPARE(res1->resourceId(), resourceId);
        QCOMPARE(res1->version(), 1);

    }

    // test adding one more version of the resource
    {
        bool result = QFile::copy(storageLocation + "/paintoppresets/test0.0001.kpp",
                                  storageLocation + "/paintoppresets/test0.0006.kpp");
        QVERIFY(result);

        m_locator->synchronizeDb();
        KisResourceModelProvider::testingResetAllModels();

        KisResourceModel model(ResourceType::PaintOpPresets);
        model.setResourceFilter(KisResourceModel::ShowAllResources);

        // ENTER_FUNCTION() << ppVar(model.rowCount());
        // for (int i = 0; i < model.rowCount(); i++) {
        //     qDebug() << ppVar(model.data(model.index(i, KisResourceModel::Filename))) << model.data(model.index(i, KisResourceModel::Id));
        // }

        QCOMPARE(model.rowCount(), 3);

        KoResourceSP res1 = model.resourceForIndex(model.index(0, 0));
        QVERIFY(res1);
        QVERIFY(res1->filename().startsWith("test0"));
        QCOMPARE(res1->resourceId(), resourceId);
        QCOMPARE(res1->version(), 6);
    }

    // test adding a completely new resource
    {
        bool result = QFile::copy(storageLocation + "/paintoppresets/test0.0001.kpp",
                                  storageLocation + "/paintoppresets/test5.0004.kpp");
        QVERIFY(result);

        result = QFile::copy(storageLocation + "/paintoppresets/test0.0001.kpp",
                             storageLocation + "/paintoppresets/test6.0003.kpp");
        QVERIFY(result);

        m_locator->synchronizeDb();
        KisResourceModelProvider::testingResetAllModels();

        KisResourceModel model(ResourceType::PaintOpPresets);
        model.setResourceFilter(KisResourceModel::ShowAllResources);

        // ENTER_FUNCTION() << ppVar(model.rowCount());
        // for (int i = 0; i < model.rowCount(); i++) {
        //     qDebug() << ppVar(model.data(model.index(i, KisResourceModel::Filename))) << model.data(model.index(i, KisResourceModel::Id));
        // }

        QCOMPARE(model.rowCount(), 5);

        {
            KoResourceSP res1 = model.resourceForIndex(model.index(3, 0));
            QVERIFY(res1->filename().startsWith("test5"));
            QCOMPARE(res1->version(), 4);
        }

        {
            KoResourceSP res1 = model.resourceForIndex(model.index(4, 0));
            QVERIFY(res1->filename().startsWith("test6"));
            QCOMPARE(res1->version(), 3);
        }
    }

    // test complete removal of all version of the resource
    {
        bool result = QFile::remove(storageLocation + "/paintoppresets/test5.0004.kpp");
        QVERIFY(result);

        m_locator->synchronizeDb();
        KisResourceModelProvider::testingResetAllModels();

        KisResourceModel model(ResourceType::PaintOpPresets);
        model.setResourceFilter(KisResourceModel::ShowAllResources);

        // ENTER_FUNCTION() << ppVar(model.rowCount());
        // for (int i = 0; i < model.rowCount(); i++) {
        //     qDebug() << ppVar(model.data(model.index(i, KisResourceModel::Filename))) << model.data(model.index(i, KisResourceModel::Id));
        // }

        QCOMPARE(model.rowCount(), 4);

        KoResourceSP res1 = model.resourceForIndex(model.index(3, 0));
        QVERIFY(res1->filename().startsWith("test6"));
        QCOMPARE(res1->version(), 3);
    }
}

void TestResourceLocator::testImportExportResource()
{
    QTemporaryFile f(QDir::tempPath() + "/testresourcemodel-testimportresourcefile-XXXXXX.kpp");
    f.open();
    f.write("mysimpletestresource");
    f.close();

    const QString dataMd5 = KoMD5Generator::generateHash(f.fileName());

    KoResourceSP resource = KisResourceLocator::instance()->importResourceFromFile(ResourceType::PaintOpPresets, f.fileName(), false);
    QVERIFY(resource);

    QCOMPARE(resource->md5Sum(false), dataMd5);

    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        resource->saveToDevice(&buffer);
        buffer.close();

        QCOMPARE(KoMD5Generator::generateHash(buffer.data()), dataMd5);
    }


    QSharedPointer<DummyResource> dummyResource = resource.dynamicCast<DummyResource>();
    KIS_ASSERT(dummyResource);

    dummyResource->setSomething("some weird data to change MD5");

    // md5 sum of the resource is updated only after it is serialized
    // into the storage
    QCOMPARE(resource->md5Sum(false), dataMd5);

    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        resource->saveToDevice(&buffer);
        buffer.close();

        // mere saving the resource will obviously change the resource's MD5
        QVERIFY(KoMD5Generator::generateHash(buffer.data()) != dataMd5);
    }

    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);

        // but exporting **not**, because it export the latest serialized
        // version
        KisResourceLocator::instance()->exportResource(resource, &buffer);
        QCOMPARE(KoMD5Generator::generateHash(buffer.data()), dataMd5);
    }

}

void TestResourceLocator::testImportDuplicatedResource()
{
    QTemporaryFile f(QDir::tempPath() + "/testresourcelocator-testimportduplicated-XXXXXX.kpp");
    f.open();
    f.write("mysimpletestresource_version1");
    f.close();

    const QString dataMd5 = KoMD5Generator::generateHash(f.fileName());
    const QString fileName = QFileInfo(f.fileName()).fileName();

    KoResourceSP resource = KisResourceLocator::instance()->importResourceFromFile(ResourceType::PaintOpPresets, f.fileName(), false);
    QVERIFY(resource);

    QCOMPARE(resource->md5Sum(false), dataMd5);
    QCOMPARE(resource->version(), 0);

    /**
     * Without override importing should fail, even though it is exactly the
     * same resource
     */
    KoResourceSP res0 = KisResourceLocator::instance()->importResourceFromFile(ResourceType::PaintOpPresets, f.fileName(), false);
    QVERIFY(!res0);

    /**
     * Since MD5 of the resource fully coincides with the existing resource, it
     * should return existing object while overriding
     */
    KoResourceSP res1 = KisResourceLocator::instance()->importResourceFromFile(ResourceType::PaintOpPresets, f.fileName(), true);

    /**
     * The returned resource must be exactly the same object from the cache
     */
    QCOMPARE(res1, resource);

    /**
     * Update the resource to make sure it is now different from the imported one
     */
    bool result = KisResourceLocator::instance()->updateResource(ResourceType::PaintOpPresets, resource);
    QVERIFY(result);
    QCOMPARE(resource->version(), 1);

    /**
     * Since the resource is updated now, then the shortcut for the existing
     * resource cannot work anymore, the resource will be wiped out and
     * created anew
     */
    KoResourceSP res3 = KisResourceLocator::instance()->importResourceFromFile(ResourceType::PaintOpPresets, f.fileName(), true);
    QVERIFY(res3);
    QVERIFY(res3 != resource);
    QCOMPARE(res3->md5Sum(false), dataMd5);
    QCOMPARE(res3->version(), 0);

    /**
     * The previous resource is not available anymore
     */
    int resourceId = -1;
    result = KisResourceCacheDb::getResourceIdFromVersionedFilename(resource->filename(), resource->resourceType().first, resource->storageLocation(), resourceId);
    QVERIFY(!result);
    QVERIFY(resourceId < 0);

#if defined Q_OS_WIN || defined Q_OS_MACOS
    /**
     * Try importing this resource with a different case on a case-insensitive
     * filesystem
     */

    f.open();
    KoResourceSP res4 = KisResourceLocator::instance()->importResource(ResourceType::PaintOpPresets, fileName.toUpper(), &f, false);
    f.close();

    /**
     * Without override flag it should fail
     */
    QVERIFY(!res4);

    f.open();
    KoResourceSP res5 = KisResourceLocator::instance()->importResource(ResourceType::PaintOpPresets, fileName.toUpper(), &f, true);
    f.close();

    /**
     * With override flag the resource should be overwritten even when its
     * md5 already present in the database
     */

    QVERIFY(res5);
    QVERIFY(res5 != res3);
    QCOMPARE(res5->md5Sum(false), dataMd5);
    QCOMPARE(res5->version(), 0);
    QCOMPARE(res5->filename(), fileName.toUpper());

#endif
}

void TestResourceLocator::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}

SIMPLE_TEST_MAIN(TestResourceLocator)

