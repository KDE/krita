/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
 * Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "TestAbrStorage.h"


#include <QTest>
#include <QImageReader>

#include <KoConfig.h>

#include <KisAbrStorage.h>
#include <KisBundleStorage.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisResourceLoaderRegistry.h>
#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

void TestAbrStorage::initTestCase()
{
    // nothing to do
}

void TestAbrStorage::testResourceIterator()
{
    QString filename = "brushes_by_mar_ka_d338ela.abr";
    KisAbrStorage storage(FILES_DATA_DIR + QDir::separator() + filename);

    QSharedPointer<KisResourceStorage::ResourceIterator> iter(storage.resources(ResourceType::Brushes));
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        KoResourceSP res(iter->resource());
        QVERIFY(res);
        count++;
    }
    QVERIFY(count > 0);
}

void TestAbrStorage::testTagIterator()
{
    QString filename = "brushes_by_mar_ka_d338ela.abr";
    KisAbrStorage storage(FILES_DATA_DIR + QDir::separator() + filename);

    QSharedPointer<KisResourceStorage::TagIterator> iter = storage.tags(ResourceType::Brushes);
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        count++;
    }
    QVERIFY(count == 0);
}

void TestAbrStorage::testResourceItem()
{
    QString name = "brushes_by_mar_ka_d338ela";
    QString filename = name + ".abr";
    QString resourceName = name + "_2"; // "1" seem to be invalid or something; it isn't not loaded in any case.
    KisAbrStorage storage(FILES_DATA_DIR + QDir::separator() + filename);

    KisResourceStorage::ResourceItem item = storage.resourceItem(resourceName);
    QVERIFY(!item.url.isEmpty());
}

void TestAbrStorage::testResource()
{
    QString name = "brushes_by_mar_ka_d338ela";
    QString filename = name + ".abr";
    QString resourceName = name + "_2"; // "1" seem to be invalid or something; it isn't not loaded in any case.
    KisAbrStorage storage(FILES_DATA_DIR + QDir::separator() + filename);
    KoResourceSP res = storage.resource(resourceName);
    QVERIFY(res);
    QVERIFY(res->filename() == resourceName);
}


QTEST_MAIN(TestAbrStorage)

