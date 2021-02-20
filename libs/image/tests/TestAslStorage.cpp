/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestAslStorage.h"


#include <QTest>
#include <QImageReader>

#include <KoConfig.h>

#include <KisAslStorage.h>

#include <testutil.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

void TestAslStorage::initTestCase()
{
    // nothing to do
}


void TestAslStorage::testResourceIterator_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("patternsCount");
    QTest::addColumn<int>("stylesCount");

    QTest::newRow("asl/test_all_style.asl") << TestUtil::fetchDataFileLazy("asl/test_all_style.asl") << 0 << 1;
    //QTest::newRow("asl/test_all_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/test_all_with_pattern.asl") << 1 << 1;
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << 1 << 4;
    QTest::newRow("asl/freebie_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/testset/freebie_with_pattern.asl") << 1 << 1;


}

void TestAslStorage::testResourceIterator()
{
    QFETCH(QString, filename);
    QFETCH(int, patternsCount);
    QFETCH(int, stylesCount);

    KisAslStorage storage(filename);

    QSharedPointer<KisResourceStorage::ResourceIterator> iter(storage.resources(ResourceType::Patterns));

    QVERIFY(iter->hasNext() || patternsCount == 0);
    int count = 0;

    while (iter->hasNext()) {
        iter->next();
        KoResourceSP res(iter->resource());
        QVERIFY(res);
        count++;
    }
    QCOMPARE(count, patternsCount);


    QSharedPointer<KisResourceStorage::ResourceIterator> iter2(storage.resources(ResourceType::LayerStyles));

    QVERIFY(iter2->hasNext());
    count = 0;

    while (iter2->hasNext()) {
        iter2->next();
        KoResourceSP res(iter2->resource());
        QVERIFY(res);
        count++;
    }
    QCOMPARE(count, stylesCount);

    QSharedPointer<KisResourceStorage::ResourceIterator> iter3(storage.resources(ResourceType::Brushes));

    count = 0;

    while (iter3->hasNext()) {
        iter3->next();
        KoResourceSP res(iter3->resource());
        QVERIFY(res);
        count++;
    }
    QCOMPARE(count, 0);

}

void TestAslStorage::testTagIterator_data()
{
    QTest::addColumn<QString>("filename");

    QTest::newRow("asl/test_all_style.asl") << TestUtil::fetchDataFileLazy("asl/test_all_style.asl");
    //QTest::newRow("asl/test_all_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/test_all_with_pattern.asl") << 1 << 1;
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl");
    QTest::newRow("asl/freebie_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/testset/freebie_with_pattern.asl");


}

void TestAslStorage::testTagIterator()
{
    QFETCH(QString, filename);

    KisAslStorage storage(filename);

    QSharedPointer<KisResourceStorage::TagIterator> iter = storage.tags(ResourceType::LayerStyles);
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        count++;
    }
    QVERIFY(count == 0);

}

void TestAslStorage::testResourceItem_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("resource");

    QTest::newRow("asl/test_all_style.asl") << TestUtil::fetchDataFileLazy("asl/test_all_style.asl") << "0701cdb9-df8a-11e4-adaf-ce8e6f81a66e_style";

    //QTest::newRow("asl/test_all_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/test_all_with_pattern.asl") << 1 << 1;

    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "8122fc0c-58b9-11d4-b895-a898787104c1_pattern";
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "81a5a778-bd9f-11d5-b8ba-b73f8571793d_style";
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "81a5a779-bd9f-11d5-b8ba-b73f8571793d_style";
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "81a5a77a-bd9f-11d5-b8ba-b73f8571793d_style";
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "4ae237f2-bda0-11d5-b8ba-b73f8571793d_style";


    QTest::newRow("asl/freebie_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/testset/freebie_with_pattern.asl")  << "47c8b792-b27f-11e1-a082-d6e8ee17595d_style";
    QTest::newRow("asl/freebie_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/testset/freebie_with_pattern.asl")  << "47c8b78c-b27f-11e1-a082-d6e8ee17595d_pattern";


}

void TestAslStorage::testResourceItem()
{

    QFETCH(QString, filename);
    QFETCH(QString, resource);
    KisAslStorage storage(filename);

    KisResourceStorage::ResourceItem item = storage.resourceItem(resource);
    QVERIFY(!item.url.isEmpty());

}

void TestAslStorage::testResource_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("resource");

    QTest::newRow("asl/test_all_style.asl") << TestUtil::fetchDataFileLazy("asl/test_all_style.asl") << "0701cdb9-df8a-11e4-adaf-ce8e6f81a66e_style";

    //QTest::newRow("asl/test_all_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/test_all_with_pattern.asl") << 1 << 1;

    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "8122fc0c-58b9-11d4-b895-a898787104c1_pattern";
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "81a5a778-bd9f-11d5-b8ba-b73f8571793d_style";
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "81a5a779-bd9f-11d5-b8ba-b73f8571793d_style";
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "81a5a77a-bd9f-11d5-b8ba-b73f8571793d_style";
    QTest::newRow("asl/multiple_styles.asl") << TestUtil::fetchDataFileLazy("asl/multiple_styles.asl") << "4ae237f2-bda0-11d5-b8ba-b73f8571793d_style";


    QTest::newRow("asl/freebie_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/testset/freebie_with_pattern.asl")  << "47c8b792-b27f-11e1-a082-d6e8ee17595d_style";
    QTest::newRow("asl/freebie_with_pattern.asl") << TestUtil::fetchDataFileLazy("asl/testset/freebie_with_pattern.asl")  << "47c8b78c-b27f-11e1-a082-d6e8ee17595d_pattern";


}

void TestAslStorage::testResource()
{
    QFETCH(QString, filename);
    QFETCH(QString, resource);


    KisAslStorage storage(filename);
    KoResourceSP res = storage.resource(resource);
    QVERIFY(res);
    QCOMPARE(res->filename(), resource);
}


QTEST_MAIN(TestAslStorage)

