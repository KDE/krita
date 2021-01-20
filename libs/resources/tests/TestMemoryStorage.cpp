/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestMemoryStorage.h"
#include <QTest>

#include <KisMemoryStorage.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisResourceLoaderRegistry.h>

#include "DummyResource.h"
#include "ResourceTestHelper.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif

void TestMemoryStorage::initTestCase()
{
    ResourceTestHelper::createDummyLoaderRegistry();
}

void TestMemoryStorage ::testStorage()
{
    KisMemoryStorage memoryStorage;
    KoResourceSP resource(new DummyResource("test.gbr", "brushes"));
    memoryStorage.addResource("brushes", resource);

    QSharedPointer<KisResourceStorage::ResourceIterator> iter = memoryStorage.resources(ResourceType::Brushes);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        QVERIFY(iter->resource());
        count++;
    }
    QCOMPARE(count, 1);
}

void TestMemoryStorage ::testStorageRetrieval()
{
    KisMemoryStorage memoryStorage;
    KoResourceSP resource1(new DummyResource("test1.gbr", "brushes"));
    memoryStorage.addResource("brushes", resource1);
    KoResourceSP resource2(new DummyResource("test2.gbr", "brushes"));
    memoryStorage.addResource("brushes", resource2);

    QString url = QString("brushes/test1.0000.gbr");
    KoResourceSP resource = memoryStorage.resource(url);
    QVERIFY(resource);
    QCOMPARE(resource->filename(), "test1.0000.gbr");
}

void TestMemoryStorage::testTagIterator()
{
    KisMemoryStorage memoryStorage;
    KisTagSP tag(new KisTag());
    tag->setComment("comment");
    tag->setUrl("url");
    tag->setName("name");
    tag->setResourceType("paintoppresets");
    memoryStorage.addTag("paintoppresets", tag);
    QSharedPointer<KisResourceStorage::TagIterator> iter = memoryStorage.tags(ResourceType::PaintOpPresets);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        count++;
    }
    QVERIFY(count == 1);
}

void TestMemoryStorage::testAddResource()
{
    KisMemoryStorage memoryStorage;
    KoResourceSP res1(new DummyResource("test1.gbr", "brushes"));
    memoryStorage.addResource("brushes", res1);

    ResourceTestHelper::testVersionedStorage(memoryStorage, "brushes", "brushes/test1.0000.gbr");
}

QTEST_MAIN(TestMemoryStorage)

