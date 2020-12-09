/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

void TestMemoryStorage ::testStorage()
{
    KisMemoryStorage memoryStorage;
    KoResourceSP resource(new DummyResource("test"));
    memoryStorage.addResource("brushes", resource);

    KisResourceLoaderRegistry::instance()->add(ResourceType::Brushes, new KisResourceLoader<DummyResource>("dummy", ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush"));

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

QTEST_MAIN(TestMemoryStorage)

