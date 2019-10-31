/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
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

