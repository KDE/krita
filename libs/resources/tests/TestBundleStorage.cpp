/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
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
#include "TestBundleStorage.h"

#include <QTest>
#include <QImageReader>

#include <KoConfig.h>

#include <KisBundleStorage.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisResourceLoaderRegistry.h>
#include "DummyResource.h"

void TestBundleStorage::initTestCase()
{
    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();
    reg->add(new KisResourceLoader<DummyResource>("gbr_brushes", "brushes", i18n("Brush tips"), QStringList() << "image/x-gimp-brush"));
    reg->add(new KisResourceLoader<DummyResource>("gih_brushes", "brushes", i18n("Brush tips"), QStringList() << "image/x-gimp-brush-animated"));
    reg->add(new KisResourceLoader<DummyResource>("svg_brushes", "brushes", i18n("Brush tips"), QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<DummyResource>("png_brushes", "brushes", i18n("Brush tips"), QStringList() << "image/png"));
    reg->add(new KisResourceLoader<DummyResource>("paintoppresets", "paintoppresets",  i18n("Brush presets"), QStringList() << "application/x-krita-paintoppreset"));
    QList<QByteArray> src = QImageReader::supportedMimeTypes();
    QStringList allImageMimes;
    Q_FOREACH(const QByteArray ba, src) {
        allImageMimes << QString::fromUtf8(ba);
    }
    reg->add(new KisResourceLoader<DummyResource>("patterns", "patterns", i18n("Patterns"), allImageMimes));
}

void TestBundleStorage::testMetaData()
{
    KisBundleStorage storage(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    QVERIFY(storage.location() == KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    qDebug() << storage.metaData(KisStoragePlugin::s_meta_generator);
    QVERIFY(!storage.metaData(KisStoragePlugin::s_meta_generator).isEmpty());
    QVERIFY(!storage.metaData(KisStoragePlugin::s_meta_author).isEmpty());
    QVERIFY(!storage.metaData(KisStoragePlugin::s_meta_description).isEmpty());
    QVERIFY(!storage.metaData(KisStoragePlugin::s_meta_initial_creator).isEmpty());
    QVERIFY(!storage.metaData(KisStoragePlugin::s_meta_dc_date).isEmpty());
    QVERIFY(!storage.metaData(KisStoragePlugin::s_meta_version).isEmpty());
}

void TestBundleStorage::testResourceIterator()
{
    KisBundleStorage storage(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    QSharedPointer<KisResourceStorage::ResourceIterator> iter = storage.resources("brushes");
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
    KisBundleStorage storage(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    QSharedPointer<KisResourceStorage::TagIterator> iter = storage.tags("paintoppresets");
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        count++;
    }
    QVERIFY(count > 0);
}

void TestBundleStorage::testResourceItem()
{
    KisBundleStorage storage(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    KisResourceStorage::ResourceItem item = storage.resourceItem("paintoppresets/g)_Dry_Brushing.kpp");
    QVERIFY(!item.url.isEmpty());
}

void TestBundleStorage::testResource()
{
    KisBundleStorage storage(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    KoResourceSP res = storage.resource("paintoppresets/g)_Dry_Brushing.kpp");
    QVERIFY(res);
    QVERIFY(res->filename() == "paintoppresets/g)_Dry_Brushing.kpp");
}


QTEST_MAIN(TestBundleStorage)

