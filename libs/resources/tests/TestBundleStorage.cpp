/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestBundleStorage.h"

#include <simpletest.h>
#include <QImageReader>

#include <KoConfig.h>

#include <KisBundleStorage.h>
#include <KoResource.h>
#include "DummyResource.h"
#include "ResourceTestHelper.h"

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <KisResourceLocator.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif


void TestBundleStorage::initTestCase()
{
    m_dstLocation = ResourceTestHelper::filesDestDir();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);

    QDir().mkpath(m_dstLocation + QString("/bundles"));

    const bool copyResult =
        QFile::copy(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"),
                    m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));

    QVERIFY(copyResult);

    ResourceTestHelper::createDummyLoaderRegistry();
}

void TestBundleStorage::testMetaData()
{
    KisBundleStorage storage(m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));
    QVERIFY(storage.location() == m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_generator).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_author).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_description).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_initial_creator).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_dc_date).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_version).isNull());
}

void TestBundleStorage::testResourceIterator()
{
    KisBundleStorage storage(m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));
    QSharedPointer<KisResourceStorage::ResourceIterator> iter = storage.resources(ResourceType::Brushes);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        KoResourceSP res = iter->resource();
        QVERIFY(res);
        count++;
    }
    QVERIFY(count > 0);
}

void TestBundleStorage::testTagIterator()
{
    KisBundleStorage storage(m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));
    QSharedPointer<KisResourceStorage::TagIterator> iter = storage.tags(ResourceType::PaintOpPresets);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        //qDebug() << iter->url() << iter->name() << iter->tag();
        count++;
    }
    QVERIFY(count > 0);
}

void TestBundleStorage::testResourceItem()
{
    KisBundleStorage storage(m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));
    KisResourceStorage::ResourceItem item = storage.resourceItem("paintoppresets/g)_Dry_Brushing.kpp");
    QVERIFY(!item.url.isEmpty());
}

void TestBundleStorage::testResource()
{
    KisBundleStorage storage(m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));
    KoResourceSP res = storage.resource("paintoppresets/g)_Dry_Brushing.kpp");
    QVERIFY(res);
    QVERIFY(res->filename() == "g)_Dry_Brushing.kpp");
}

void TestBundleStorage::testAddResource()
{

    KisBundleStorage storage(m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));

    const QString resourceUrl = "paintoppresets/g)_Dry_Brushing.kpp";
    const QString resourceType = ResourceType::PaintOpPresets;

    ResourceTestHelper::testVersionedStorage(storage, resourceType, resourceUrl,
                                            m_dstLocation +
                                             QString("/bundles/Krita_4_Default_Resources.bundle_modified"));

    ResourceTestHelper::testVersionedStorageIterator(storage, resourceType, resourceUrl);
}

void TestBundleStorage::testResourceCaseSensitivity()
{
    KisBundleStorage storage(m_dstLocation + QString("/bundles/Krita_4_Default_Resources.bundle"));

    const QString resourceUrl = "paintoppresets/g)_Dry_Brushing.kpp";
    const QString resourceType = ResourceType::PaintOpPresets;

    KoResourceSP resource = storage.resource(resourceUrl);
    QVERIFY(resource);

    resource->setVersion(1);

    bool result = storage.saveAsNewVersion(resourceType, resource);
    QVERIFY(result);

    /**
     * In the bundle storage we expect the new versioned resources to have
     * filesystem-dependent name resolution. That is, if we request a resource
     * with wrong casing, it should still fetch the resource. But the filename
     * field of the resource must contain the correct casing
     */

#if defined Q_OS_WIN || defined Q_OS_MACOS
    KoResourceSP res2 = storage.resource("paintoppresets/" + resource->filename().toUpper());
    QVERIFY(res2);
    QCOMPARE(res2->filename(), resource->filename());
#endif

}

void TestBundleStorage::cleanupTestCase()
{
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}



SIMPLE_TEST_MAIN(TestBundleStorage)

