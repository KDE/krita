/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestTagModel.h"

#include <simpletest.h>
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
#include <KisTagModel.h>
#include <KisResourceModel.h>
#include <DummyResource.h>
#include <ResourceTestHelper.h>


#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestTagModel::initTestCase()
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

    m_tag.reset(new KisTag());
    QFile f(QString(FILES_DATA_DIR) + "paintoppresets/test.tag");
    f.open(QFile::ReadOnly);
    m_tag->load(f);
}


void TestTagModel::testRowCount()
{
    QSqlQuery q;
    QVERIFY(q.prepare("SELECT count(*)\n"
                      "FROM   tags\n"
                      ",      resource_types\n"
                      "WHERE  tags.resource_type_id = resource_types.id\n"
                      "AND    resource_types.name = :resource_type"));
    q.bindValue(":resource_type", resourceType);
    QVERIFY(q.exec());
    q.first();
    int rowCount = q.value(0).toInt();
    QCOMPARE(rowCount, 1);

    KisTagModel tagModel(resourceType);
    // There is always an "All" tag in the first row
    QCOMPARE(tagModel.rowCount(), rowCount + 2);
}

void TestTagModel::testData()
{
    KisTagModel tagModel(resourceType);

    QVariant v = tagModel.data(tagModel.index(0, 0), Qt::DisplayRole);
    QCOMPARE(v.toString(), "All");

    v = tagModel.data(tagModel.index(0, 0), Qt::UserRole + KisAllTagsModel::Url);
    QCOMPARE(v.toString(), "All");

    v = tagModel.data(tagModel.index(1, 0), Qt::DisplayRole);
    QCOMPARE(v.toString(), "All Untagged");

    v = tagModel.data(tagModel.index(1, 0), Qt::UserRole + KisAllTagsModel::Url);
    QCOMPARE(v.toString(), "All Untagged");

    v = tagModel.data(tagModel.index(2, 0), Qt::DisplayRole);
    QCOMPARE(v.toString(), "* Favorites");

    v = tagModel.data(tagModel.index(2, 0), Qt::UserRole + KisAllTagsModel::Url);
    QCOMPARE(v.toString(), "* Favorites");

}

void TestTagModel::testIndexForTag()
{
    KisTagModel tagModel(resourceType);
    QModelIndex idx = tagModel.indexForTag(m_tag);
    QVERIFY(idx.isValid());
    QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Url).toString(), m_tag->url());
    QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Name).toString(), m_tag->name());
}

void TestTagModel::testTagForIndex()
{
    KisTagModel tagModel(resourceType);

    QModelIndex idx = tagModel.index(0, 0);
    KisTagSP tag = tagModel.tagForIndex(idx);
    QCOMPARE(tag->url(), "All");

    idx = tagModel.index(1, 0);
    tag = tagModel.tagForIndex(idx);
    QCOMPARE(tag->url(), "All Untagged");

    idx = tagModel.index(2, 0);
    tag = tagModel.tagForIndex(idx);
    QCOMPARE(tag->url(), m_tag->url());
}

void TestTagModel::testTagForUrl()
{
    KisTagModel tagModel(resourceType);

    KisTagSP tag = tagModel.tagForUrl("All");
    QVERIFY(tag);
    QCOMPARE(tag->url(), "All");

    tag = tagModel.tagForUrl("All Untagged");
    QVERIFY(tag);
    QCOMPARE(tag->url(), "All Untagged");

    tag = tagModel.tagForUrl(m_tag->url());
    QVERIFY(tag);
    QCOMPARE(tag->url(), m_tag->url());
}

void TestTagModel::testAddEmptyTag()
{
    KisTagModel tagModel(resourceType);

    QString tagName("A Brand New Tag");

    int rowCount = tagModel.rowCount();
    tagModel.addTag(tagName, false, {});

    QCOMPARE(tagModel.rowCount(), rowCount + 1);
    QModelIndex idx = tagModel.index(3, 0);
    QVERIFY(idx.isValid());

    KisTagSP tag = tagModel.tagForIndex(idx);
    QCOMPARE(tag->name(), tagName);
    QCOMPARE(tag->id(), 2);
}

void TestTagModel::testAddTag()
{
    KisTagModel tagModel(resourceType);

    QString tagName("test1");

    KisTagSP tag(new KisTag);
    tag->setUrl(tagName);
    tag->setName(tagName);
    tag->setComment("A tag for testing");
    tag->setValid(true);
    tag->setActive(true);

    int rowCount = tagModel.rowCount();
    tagModel.addTag(tag, false, {});
    QCOMPARE(tagModel.rowCount(), rowCount + 1);
    QVERIFY(tag->id() >= 0);

    {
        QCOMPARE(tagModel.rowCount(), rowCount + 1);
        QModelIndex idx = tagModel.index(4, 0);
        QVERIFY(idx.isValid());
        QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Url).toString(), tag->url());
        QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Name).toString(), tag->name());

        KisTagSP tag = tagModel.tagForIndex(idx);
        QCOMPARE(tag->name(), tagName);
        QCOMPARE(tag->id(), 3);
    }

    {
        QModelIndex idx = tagModel.indexForTag(tag);
        QVERIFY(idx.isValid());
        QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Url).toString(), tag->url());
        QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Name).toString(), tag->name());
    }

}

void TestTagModel::testSetTagActiveInactive()
{
    KisTagModel tagModel(resourceType);

    int rowCount = tagModel.rowCount();

    tagModel.setTagInactive(m_tag);
    QVERIFY(!m_tag->active());
    QCOMPARE(tagModel.rowCount(), rowCount -1);
    QModelIndex idx = tagModel.indexForTag(m_tag);

    QCOMPARE(tagModel.data(idx, Qt::UserRole + KisAllTagsModel::Active).toBool(), false);


    tagModel.setTagActive(m_tag);
    QVERIFY(m_tag->active());
    QCOMPARE(tagModel.rowCount(), rowCount);

    idx = tagModel.indexForTag(m_tag);

    QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Url).toString(), m_tag->url());
    QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Name).toString(), m_tag->name());
    QCOMPARE(tagModel.data(idx, Qt::UserRole + KisAllTagsModel::Active).toBool(), true);
}

void TestTagModel::testRenameTag()
{
    KisTagModel tagModel(resourceType);
    KisTagSP tag = tagModel.tagForIndex(tagModel.index(2,0));
    QCOMPARE(tag->url(), m_tag->url());
    QCOMPARE(tag->name(), m_tag->name());

    tag->setName("Another name altogether");
    QVERIFY(tagModel.renameTag(tag));

    tag = tagModel.tagForIndex(tagModel.index(2,0));
    QCOMPARE(tag->url(), m_tag->url());
    QCOMPARE(tag->name(), "Another name altogether");
}

void TestTagModel::testChangeTagActive()
{
    KisTagModel tagModel(resourceType);

    int rowCount = tagModel.rowCount();

    tagModel.changeTagActive(m_tag, false);
    QVERIFY(!m_tag->active());
    QCOMPARE(tagModel.rowCount(), rowCount -1);
    QModelIndex idx = tagModel.indexForTag(m_tag);

    QCOMPARE(tagModel.data(idx, Qt::UserRole + KisAllTagsModel::Active).toBool(), false);

    tagModel.changeTagActive(m_tag, true);
    QVERIFY(m_tag->active());
    QCOMPARE(tagModel.rowCount(), rowCount);

    idx = tagModel.indexForTag(m_tag);

    QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Url).toString(), m_tag->url());
    QCOMPARE(idx.data(Qt::UserRole + KisAllTagsModel::Name).toString(), "Another name altogether");
    QCOMPARE(tagModel.data(idx, Qt::UserRole + KisAllTagsModel::Active).toBool(), true);

}

void TestTagModel::testAddEmptyTagWithResources()
{
    KisTagModel tagModel(resourceType);
    KisResourceModel resourceModel("paintoppresets");

    QString tagName("A Brand New Tag");
    QVector<KoResourceSP> resources;
    for (int i = 0; i < resourceModel.rowCount(); ++i)
    {
        resources << resourceModel.resourceForIndex(resourceModel.index(i, 0));
    }

    tagModel.addTag(tagName, false, resources);

    // XXX: check KisTagResourceModel
}

void TestTagModel::testAddTagWithResources()
{
    KisTagModel tagModel(resourceType);
    KisResourceModel resourceModel("paintoppresets");

    QString tagName("test1");

    KoResourceSP resource = resourceModel.resourceForIndex(resourceModel.index(0, 0));

    KisTagSP tag(new KisTag);
    tag->setUrl(tagName);
    tag->setName(tagName);
    tag->setComment("A tag for testing");
    tag->setValid(true);
    tag->setActive(true);
    tag->setResourceType("paintoppresets");

    tagModel.addTag(tag, false, {resource});
    QVERIFY(tag->id() >= 0);

    // XXX: check KisTagResourceModel

}

void TestTagModel::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}





SIMPLE_TEST_MAIN(TestTagModel)

