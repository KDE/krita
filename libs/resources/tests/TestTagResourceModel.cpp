/*
 * SPDX-FileCopyrightText: 2020 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestTagResourceModel.h"

#include <QTest>
#include <QStandardPaths>
#include <QDir>
#include <QVersionNumber>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlQuery>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>
#include <KisResourceLoaderRegistry.h>
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


void TestTagResourceModel::testRowCount()
{
    QSqlQuery q;
    QVERIFY(q.prepare("SELECT count(*)\n"
                      "FROM   resource_tags"));
    QVERIFY(q.exec());
    q.first();
    int rowCount = q.value(0).toInt();
    QCOMPARE(rowCount, 2);

    KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
    QCOMPARE(tagResourceModel.rowCount(), rowCount);
}

void TestTagResourceModel::testData()
{
     KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
     QModelIndex idx = tagResourceModel.index(0, 0);
     QVERIFY(idx.isValid());

     int tagId = tagResourceModel.data(idx, Qt::UserRole + KisAllTagResourceModel::TagId).toInt();
     QCOMPARE(tagId, 1);

     int resourceId = tagResourceModel.data(idx, Qt::UserRole + KisAllTagResourceModel::ResourceId).toInt();
     QCOMPARE(resourceId, 3);

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

     bool tagActive = tagResourceModel.data(idx, Qt::UserRole + KisAllTagResourceModel::TagActive).toBool();
     QVERIFY(tagActive);

     bool resourceActive = tagResourceModel.data(idx, Qt::UserRole + KisAllTagResourceModel::ResourceActive).toBool();
     QVERIFY(resourceActive);

     bool resourceStorageActive = tagResourceModel.data(idx, Qt::UserRole + KisAllTagResourceModel::ResourceStorageActive).toBool();
     QVERIFY(resourceStorageActive);
}

void TestTagResourceModel::testTagResource()
{
    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    KoResourceSP resource = resourceModel.resourceForName("test2.kpp");
    Q_ASSERT(resource);

    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForIndex(tagModel.index(2, 0));
    Q_ASSERT(tag);

    KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
    int rowCount = tagResourceModel.rowCount();

    QVERIFY(tagResourceModel.tagResource(tag, resource->resourceId()));

    QCOMPARE(tagResourceModel.rowCount(), rowCount + 1);
}

void TestTagResourceModel::testUntagResource()
{
    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    KoResourceSP resource = resourceModel.resourceForName("test1.kpp");
    QVERIFY(resource);

    KisTagModel tagModel(ResourceType::PaintOpPresets);
    KisTagSP tag = tagModel.tagForIndex(tagModel.index(2, 0));
    QVERIFY(tag);

    KisAllTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
    int rowCount = tagResourceModel.rowCount();
    tagResourceModel.untagResource(tag, resource->resourceId());

    QCOMPARE(tagResourceModel.rowCount(), rowCount - 1);
}

void TestTagResourceModel::testFilterTagResource()
{
    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    KoResourceSP resource = resourceModel.resourceForName("test2.kpp");
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


QTEST_MAIN(TestTagResourceModel)

