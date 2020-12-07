/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
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

void TestBundleStorage::initTestCase()
{
    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();
    reg->add(new KisResourceLoader<DummyResource>(ResourceSubType::GbrBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush"));
    reg->add(new KisResourceLoader<DummyResource>(ResourceSubType::GihBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush-animated"));
    reg->add(new KisResourceLoader<DummyResource>(ResourceSubType::SvgBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<DummyResource>(ResourceSubType::PngBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/png"));
    reg->add(new KisResourceLoader<DummyResource>(ResourceType::PaintOpPresets, ResourceType::PaintOpPresets,  i18n("Brush presets"), QStringList() << "application/x-krita-paintoppreset"));
    QList<QByteArray> src = QImageReader::supportedMimeTypes();
    QStringList allImageMimes;
    Q_FOREACH(const QByteArray ba, src) {
        allImageMimes << QString::fromUtf8(ba);
    }
    reg->add(new KisResourceLoader<DummyResource>(ResourceType::Patterns, ResourceType::Patterns, i18n("Patterns"), allImageMimes));
}

void TestBundleStorage::testMetaData()
{
    KisBundleStorage storage(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    QVERIFY(storage.location() == KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_generator).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_author).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_description).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_initial_creator).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_dc_date).isNull());
    QVERIFY(!storage.metaData(KisResourceStorage::s_meta_version).isNull());
}

void TestBundleStorage::testResourceIterator()
{
    KisBundleStorage storage(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
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
    KisBundleStorage storage(KRITA_SOURCE_DIR + QString("/krita/data/bundles/Krita_4_Default_Resources.bundle"));
    QSharedPointer<KisResourceStorage::TagIterator> iter = storage.tags(ResourceType::PaintOpPresets);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        //qDebug() << iter->url() << iter->name() << iter->tag();
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
    QVERIFY(res->filename() == "g)_Dry_Brushing.kpp");
}


QTEST_MAIN(TestBundleStorage)

