/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestBundleStorage.h"

#include <QTest>
#include <QImageReader>

#include <KoConfig.h>

#include <KisBundleStorage.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisResourceLoaderRegistry.h>
#include "DummyResource.h"
#include "ResourceTestHelper.h"

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <KisResourceLocator.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestBundleStorage::initTestCase()
{
    ResourceTestHelper::cleanDstLocation(FILES_DEST_DIR);

    QDir().mkpath(FILES_DEST_DIR + QString("/bundles"));

    const bool copyResult =
        QFile::copy(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"),
                    FILES_DEST_DIR + QString("/bundles/Krita_4_Default_Resources.bundle"));

    QVERIFY(copyResult);

    ResourceTestHelper::createDummyLoaderRegistry();
}

void TestBundleStorage::cleanupTestCase()
{
    ResourceTestHelper::cleanDstLocation(FILES_DEST_DIR);
}

void TestBundleStorage::testMetaData()
{
    KisBundleStorage storage(FILES_DEST_DIR + QString("/bundles/Krita_4_Default_Resources.bundle"));
    QVERIFY(storage.location() == FILES_DEST_DIR + QString("/bundles/Krita_4_Default_Resources.bundle"));
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_generator).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_author).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_description).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_initial_creator).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_dc_date).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_version).isNull());
}

void TestBundleStorage::testResourceIterator()
{
    KisBundleStorage storage(FILES_DEST_DIR + QString("/bundles/Krita_4_Default_Resources.bundle"));
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
    KisBundleStorage storage(FILES_DEST_DIR + QString("/bundles/Krita_4_Default_Resources.bundle"));
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
    KisBundleStorage storage(FILES_DEST_DIR + QString("/bundles/Krita_4_Default_Resources.bundle"));
    KisResourceStorage::ResourceItem item = storage.resourceItem("paintoppresets/g)_Dry_Brushing.kpp");
    QVERIFY(!item.url.isEmpty());
}

void TestBundleStorage::testResource()
{
    KisBundleStorage storage(FILES_DEST_DIR + QString("/bundles/Krita_4_Default_Resources.bundle"));
    KoResourceSP res = storage.resource("paintoppresets/g)_Dry_Brushing.kpp");
    QVERIFY(res);
    QVERIFY(res->filename() == "g)_Dry_Brushing.kpp");
}

void TestBundleStorage::testAddResource()
{

    KisBundleStorage storage(FILES_DEST_DIR + QString("/bundles/Krita_4_Default_Resources.bundle"));

    const QString resourceUrl = "paintoppresets/g)_Dry_Brushing.kpp";
    const QString resourceType = ResourceType::PaintOpPresets;

    ResourceTestHelper::testVersionedStorage(storage, resourceType, resourceUrl,
                                            FILES_DEST_DIR +
                                             QString("/bundles/Krita_4_Default_Resources.bundle_modified"));

    ResourceTestHelper::testVersionedStorageIterator(storage, resourceType, resourceUrl);
}

QTEST_MAIN(TestBundleStorage)

