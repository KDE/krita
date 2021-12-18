/*
 * SPDX-FileCopyrightText: 2020 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestTagResourceModel.h"

#include <simpletest.h>
#include <QStandardPaths>
#include <QDir>
#include <QVersionNumber>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlQuery>
#include <QAbstractItemModelTester>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>
#include <KisResourceModel.h>
#include <KisTagModel.h>
#include <KisTagResourceModel.h>
#include <KisResourceTypes.h>

#include <DummyResource.h>
#include <ResourceTestHelper.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestTagResourceModel::initTestCase()
{
    ResourceTestHelper::initTestDb();
    ResourceTestHelper::createDummyLoaderRegistry();

    m_srcLocation = QString(FILES_DATA_DIR);
    QVERIFY2(QDir(m_srcLocation).exists(), m_srcLocation.toUtf8());

    m_dstLocation = QString(FILES_DEST_DIR);
    ResourceTestHelper::cleanDstLocation(m_dstLocation);

    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    cfg.writeEntry(KisResourceLocator::resourceLocationKey, m_dstLocation);

    m_locator = KisResourceLocator::instance();

    if (!KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))) {
        qDebug() << "Could not initialize KisResourceCacheDb on" << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    QVERIFY(KisResourceCacheDb::isValid());

    KisResourceLocator::LocatorError r = m_locator->initialize(m_srcLocation);
    if (!m_locator->errorMessages().isEmpty()) qDebug() << m_locator->errorMessages();

    QVERIFY(r == KisResourceLocator::LocatorError::Ok);
    QVERIFY(QDir(m_dstLocation).exists());
}

void TestTagResourceModel::testWithTagModelTester()
{
    KisAllTagResourceModel model(ResourceType::PaintOpPresets);
    auto tester = new QAbstractItemModelTester(&model);
    Q_UNUSED(tester);
}


void TestTagResourceModel::testRowCount()
{
    QSqlQuery q;
    QVERIFY(q.prepare("SELECT count(*)\n"
                      "FROM   resource_tags\n"
                      "WHERE  resource_tags.active = 1\n"));
    QVERIFY(q.exec());
    q.first();
    int rowCount = q.value(0).toInt();
    QCOMPARE(rowCount, 2);

    KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
    QCOMPARE(tagResourceModel.rowCount(), rowCount);
}

bool testDataInColumnAndRole(KisAllTagResourceModel &tagResourceModel, int columnRole)
{
    QModelIndex idx = tagResourceModel.index(0, columnRole);
    QVariant data1 = tagResourceModel.data(idx, Qt::DisplayRole);

    idx = tagResourceModel.index(0, 0);
    QVariant data2 = tagResourceModel.data(idx, Qt::UserRole + columnRole);
    bool success = data1 == data2;
    if (!success) {
        qInfo() << "Column: " << columnRole << "Data from column: " << data1 << "Data from role: " << data2;
    }
    return success;
}

bool testDataInColumnAndRole(KisAllTagResourceModel &tagResourceModel, int columnRole, QVariant expected)
{
    QModelIndex idx = tagResourceModel.index(0, columnRole);
    QVariant data = tagResourceModel.data(idx, Qt::DisplayRole);
    bool columnSuccess = data == expected;
    if (!columnSuccess) {
        qInfo() << "Column: " << columnRole << "Expected: " << expected << "Actual: " << data;
    }

    idx = tagResourceModel.index(0, 0);
    data = tagResourceModel.data(idx, Qt::UserRole + columnRole);
    bool roleSuccess = data == expected;
    if (!roleSuccess) {
        qInfo() << "Column: " << columnRole << "Expected: " << expected << "Actual: " << data;
    }
    return columnSuccess && roleSuccess;

}

void TestTagResourceModel::testData()
{
     KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
     QModelIndex idx = tagResourceModel.index(0, 0);
     QVERIFY(idx.isValid());

     // *** columns from ResourceModel, via ResourceQueryMapper *** //
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::Id, 3));
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::StorageId, 1));
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::Name, "test0.kpp"));
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::Filename, "test0.kpp"));
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::Tooltip, "test0.kpp"));

    // Thumbnail -- without checking the value, just if both are the same
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::Thumbnail));
    // Status -- skip

    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::Location, ""));
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::ResourceType, ResourceType::PaintOpPresets));

    // Tags -- only in role; there is no column like that, the result in QVariant()
    QStringList tagList = tagResourceModel.data(idx, Qt::UserRole + KisAbstractResourceModel::Tags).value<QStringList>();
    QCOMPARE(tagList.count(), 1);
    QCOMPARE(tagList[0], "* Favorites");

    // Large Thumbnail -- without checking the value, just if both are the same
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::LargeThumbnail));

    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::Dirty, false));

    // MetaData -- only in role; there is no column like that, the result in QVariant()
    QMap<QString, QVariant> metadata = tagResourceModel.data(idx, Qt::UserRole + KisAbstractResourceModel::MetaData).value<QMap<QString, QVariant>>();
    QCOMPARE(metadata.count(), 0);

    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::ResourceActive, true));
    QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAbstractResourceModel::StorageActive, true));

     // *** TagResourceModel own columns *** //
     QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAllTagResourceModel::TagId, 1));
     QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAllTagResourceModel::ResourceId, 3));

     KisTagSP tag = tagResourceModel.data(idx, Qt::UserRole + KisAllTagResourceModel::Tag).value<KisTagSP>();
     QVERIFY(tag);
     QVERIFY(tag->valid());
     QCOMPARE(tag->name(), "* Favorites");
     QCOMPARE(tag->id(), 1);

     KoResourceSP resource = tagResourceModel.data(idx, Qt::UserRole + KisAllTagResourceModel::Resource).value<KoResourceSP>();
     QVERIFY(resource);
     QVERIFY(resource->valid());
     QCOMPARE(resource->name(), "test0.kpp");
     QCOMPARE(resource->resourceId(), 3);

     QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAllTagResourceModel::TagActive, true));
     QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAllTagResourceModel::ResourceActive, true));
     QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAllTagResourceModel::ResourceStorageActive, true));

     QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAllTagResourceModel::ResourceName, "test0.kpp"));
     QVERIFY(testDataInColumnAndRole(tagResourceModel, KisAllTagResourceModel::TagName, "* Favorites"));


}

void TestTagResourceModel::testTagResource()
{
    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    KoResourceSP resource = resourceModel.resourcesForName("test2.kpp").first();
    Q_ASSERT(resource);

    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForIndex(tagModel.index(2, 0));
    Q_ASSERT(tag);

    KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
    if (tagResourceModel.isResourceTagged(tag, resource->resourceId())) {
        tagResourceModel.untagResource(tag, resource->resourceId());
    }

    int rowCount = tagResourceModel.rowCount();

    QVERIFY(tagResourceModel.tagResource(tag, resource->resourceId()));

    QCOMPARE(tagResourceModel.rowCount(), rowCount + 1);
}

void TestTagResourceModel::testUntagResource()
{
    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    KoResourceSP resource = resourceModel.resourcesForName("test1.kpp").first();
    QVERIFY(resource);

    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForIndex(tagModel.index(2, 0));
    QVERIFY(tag);

    KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);

    if (!tagResourceModel.isResourceTagged(tag, resource->resourceId())) {
        tagResourceModel.tagResource(tag, resource->resourceId());
    }

    int rowCount = tagResourceModel.rowCount();
    tagResourceModel.untagResource(tag, resource->resourceId());

    QCOMPARE(tagResourceModel.rowCount(), rowCount - 1);
}

void TestTagResourceModel::testIsResourceTagged()
{
    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    KoResourceSP resource = resourceModel.resourcesForName("test2.kpp").first();
    Q_ASSERT(resource);

    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForIndex(tagModel.index(2, 0));
    Q_ASSERT(tag);

    KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);

    QVERIFY(tagResourceModel.tagResource(tag, resource->resourceId()));
    QCOMPARE(tagResourceModel.isResourceTagged(tag, resource->resourceId()), true);

    resource = resourceModel.resourcesForName("test1.kpp").first();
    QVERIFY(resource);

    tag = tagModel.tagForIndex(tagModel.index(2, 0));
    QVERIFY(tag);

    tagResourceModel.untagResource(tag, resource->resourceId());
    QCOMPARE(tagResourceModel.isResourceTagged(tag, resource->resourceId()), false);
}

void TestTagResourceModel::testFilterTagResource()
{
    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    KoResourceSP resource = resourceModel.resourcesForName("test2.kpp").first();
    Q_ASSERT(resource);

    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForIndex(tagModel.index(2, 0));
    Q_ASSERT(tag);

    KisTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
    QCOMPARE(tagResourceModel.rowCount(), 2);

    QVector<int> tagIds;
    tagIds << tag->id();
    tagResourceModel.setTagsFilter(tagIds);

    QVector<int> resourceIds;
    resourceIds << resource->resourceId();
    tagResourceModel.setResourcesFilter(resourceIds);

    QCOMPARE(tagResourceModel.rowCount(), 1);
}

void TestTagResourceModel::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}

#include <sdk/tests/kistest.h>
KISTEST_MAIN(TestTagResourceModel)

