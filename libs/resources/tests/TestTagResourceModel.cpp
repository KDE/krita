/*
 * Copyright (c) 2020 boud <boud@valdyas.org>
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
#include <KisTagModelProvider.h>
#include <KisTagResourceModel.h>
#include <KisResourceModelProvider.h>

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

    KisAllTagResourceModel *tagResourceModel = new KisAllTagResourceModel(0);
    QCOMPARE(tagResourceModel->rowCount(), rowCount);
    delete tagResourceModel;
}

void TestTagResourceModel::testData()
{
     KisAllTagResourceModel *tagResourceModel = new KisAllTagResourceModel(0);
     QModelIndex idx = tagResourceModel->index(0, 0);
     QVERIFY(idx.isValid());

     int tagId = tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::TagId).toInt();
     QCOMPARE(tagId, 1);

     int resourceId = tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::ResourceId).toInt();
     QCOMPARE(resourceId, 4);

     KisTagSP tag = tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::Tag).value<KisTagSP>();
     QVERIFY(tag);
     QVERIFY(tag->valid());
     QCOMPARE(tag->name(), "* Favorites");
     QCOMPARE(tag->id(), 1);

     KoResourceSP resource = tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::Resource).value<KoResourceSP>();
     QVERIFY(resource);
     QVERIFY(resource->valid());
     QCOMPARE(resource->name(), "test0.kpp");
     QCOMPARE(resource->resourceId(), 4);

     bool tagActive = tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::TagActive).toBool();
     QVERIFY(tagActive);

     bool resourceActive = tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::ResourceActive).toBool();
     QVERIFY(resourceActive);

     bool resourceStorageActive = tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::ResourceStorageActive).toBool();
     QVERIFY(resourceStorageActive);

     delete tagResourceModel;
}

void TestTagResourceModel::testTagResource()
{
    KisResourceModel *resourceModel = KisResourceModelProvider::resourceModel("paintoppresets");
    KoResourceSP resource = resourceModel->resourceForName("test2.kpp");
    Q_ASSERT(resource);

    KisTagModel *tagModel = KisTagModelProvider::tagModel("paintoppresets");
    KisTagSP tag = tagModel->tagForIndex(tagModel->index(2, 0));
    Q_ASSERT(tag);

    KisAllTagResourceModel *tagResourceModel = new KisAllTagResourceModel(0);
    int rowCount = tagResourceModel->rowCount();

    QVERIFY(tagResourceModel->tagResource(tag, resource));

    QCOMPARE(tagResourceModel->rowCount(), rowCount + 1);

    delete tagResourceModel;
}

void TestTagResourceModel::testUntagResource()
{
    KisResourceModel *resourceModel = KisResourceModelProvider::resourceModel("paintoppresets");
    KoResourceSP resource = resourceModel->resourceForName("test1.kpp");
    Q_ASSERT(resource);

    KisTagModel *tagModel = KisTagModelProvider::tagModel("paintoppresets");
    KisTagSP tag = tagModel->tagForIndex(tagModel->index(2, 0));
    Q_ASSERT(tag);

    KisAllTagResourceModel *tagResourceModel = new KisAllTagResourceModel(0);
    int rowCount = tagResourceModel->rowCount();
    tagResourceModel->untagResource(tag, resource);

    QCOMPARE(tagResourceModel->rowCount(), rowCount - 1);
}

void TestTagResourceModel::testFilterTagResource()
{
    KisResourceModel *resourceModel = KisResourceModelProvider::resourceModel("paintoppresets");
    KoResourceSP resource = resourceModel->resourceForName("test2.kpp");
    Q_ASSERT(resource);

    KisTagModel *tagModel = KisTagModelProvider::tagModel("paintoppresets");
    KisTagSP tag = tagModel->tagForIndex(tagModel->index(2, 0));
    Q_ASSERT(tag);

    KisTagResourceModel *tagResourceModel = new KisTagResourceModel;
    QCOMPARE(tagResourceModel->rowCount(), 0);

    QVector<int> tagIds;
    tagIds << tag->id();
    tagResourceModel->setTagsFilter(tagIds);

    QVector<int> resourceIds;
    resourceIds << resource->resourceId();
    tagResourceModel->setResourcesFilter(resourceIds);

    QCOMPARE(tagResourceModel->rowCount(), 1);

    delete tagResourceModel;

}


void TestTagResourceModel::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}


QTEST_MAIN(TestTagResourceModel)

