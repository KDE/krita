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
        tagResourceModel.untagResources(tag, { resource->resourceId() });
    }

    int rowCount = tagResourceModel.rowCount();

    QVERIFY(tagResourceModel.tagResources(tag, { resource->resourceId() }));

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
        tagResourceModel.tagResources(tag, { resource->resourceId() });
    }

    int rowCount = tagResourceModel.rowCount();
    tagResourceModel.untagResources(tag, { resource->resourceId() });

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

    QVERIFY(tagResourceModel.tagResources(tag, { resource->resourceId() }));
    QCOMPARE(tagResourceModel.isResourceTagged(tag, resource->resourceId()), true);

    resource = resourceModel.resourcesForName("test1.kpp").first();
    QVERIFY(resource);

    tag = tagModel.tagForIndex(tagModel.index(2, 0));
    QVERIFY(tag);

    tagResourceModel.untagResources(tag, { resource->resourceId() });
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


void printOutModel(QAbstractItemModel* model)
{
    if (model->inherits("KisTagResourceModel")) {
        KisTagResourceModel* tagResourceModel = dynamic_cast<KisTagResourceModel*>(model);
        if (!tagResourceModel) {
            return;
        }
        for (int i = 0; i < tagResourceModel->rowCount(); i++) {
            QModelIndex idx = tagResourceModel->index(i, 0);
            qCritical() << "Tag Resource | i: " << i << "| resource id: " << tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::ResourceId).toInt()
                        << "| tag id: " << tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::TagId).toInt()
                        << "(" << tagResourceModel->data(idx, Qt::UserRole + KisAllTagResourceModel::TagName).toString() << ")";
        }
        return;
    }

    if (model->inherits("KisTagModel")) {
        KisTagModel* tagModel = dynamic_cast<KisTagModel*>(model);
        if (!tagModel) {
            return;
        }
        for (int i = 0; i < tagModel->rowCount(); i++) {
            QModelIndex idx = tagModel->index(i, 0);
            qCritical() << "Tag | i: " << i << "| real id: " << tagModel->data(idx, Qt::UserRole + KisAllTagsModel::Id).toInt()
                        << "| name: " << tagModel->data(idx, Qt::UserRole + KisAllTagsModel::Name).toString();
        }
        return;
    }

    if (model->inherits("KisResourceModel")) {
        KisResourceModel* resourceModel = dynamic_cast<KisResourceModel*>(model);
        if (!resourceModel) {
            return;
        }
        for (int i = 0; i < resourceModel->rowCount(); i++) {
            QModelIndex idx = resourceModel->index(i, 0);
            qCritical() << "Resource | i: " << i << "| real id: " << resourceModel->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toInt()
                        << "| name: " << resourceModel->data(idx, Qt::UserRole + KisAllResourcesModel::Name).toString();
        }
        return;
    }

}


void TestTagResourceModel::testBeginEndInsert()
{
    KisTagResourceModel tagResourceModel(ResourceType::PaintOpPresets);
    KisResourceModel resourceModel(ResourceType::PaintOpPresets);
    KisTagModel tagModel(ResourceType::PaintOpPresets);

    QList<KoResourceSP> resources;
    QList<int> resourceIds;

    // prepare signal checker
    typedef ModelSignalChecker SC;
    ModelSignalChecker checker({5, 5, 4, 4}, {5, 5, 5, 5}, {SC::AboutInsert, SC::Insert, SC::AboutInsert, SC::Insert});

    // debug tips:
    // to see the content of the signal checker, use:
    //checker.printOut();
    // to see the content of the tagResourceModel, use:
    //printOutModel(&tagResourceModel);

    connect(&tagResourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex, int, int)), &checker, SLOT(rowsAboutToBeInserted(const QModelIndex, int, int)));
    connect(&tagResourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex, int, int)), &checker, SLOT(rowsAboutToBeRemoved(const QModelIndex, int, int)));
    connect(&tagResourceModel, SIGNAL(rowsInserted(const QModelIndex, int, int)), &checker, SLOT(rowsInserted(const QModelIndex, int, int)));
    connect(&tagResourceModel, SIGNAL(rowsRemoved(const QModelIndex, int, int)), &checker, SLOT(rowsRemoved(const QModelIndex, int, int)));

    // this builds a list of resources
    for (int i = 0; i < resourceModel.rowCount(); i++) {
        QModelIndex idx = resourceModel.index(i, 0);
        if (resources.length() < 4) { // sanity check
            resources << resourceModel.resourceForIndex(idx);
            resourceIds << resourceModel.data(idx, Qt::UserRole + KisAllResourcesModel::Id).toInt();
        }
    }

    // ////
    tagModel.addTag("Tag1", true, {});
    tagModel.addTag("Tag2", true, {});
    tagModel.addTag("TagSpecial", true, {});


    QList<KisTagSP> tags;
    // this builds a list of tags

    for (int i = 0; i < tagModel.rowCount(); i++) {
        QModelIndex idx = tagModel.index(i, 0);
        int realId = tagModel.data(idx, Qt::UserRole + KisAllTagsModel::Id).toInt();
        if (tags.length() < 5 && realId >= 0) { // sanity check
            tags << tagModel.tagForIndex(idx);
        }
    }

    QCOMPARE(tags.count(), 4);
    QCOMPARE(resources.count(), 3);


    // tag assignment to resources:
    // other1 - res1
    // other2 - res2
    // special - res2
    // special - res3
    // other2 - res3
    // special - res1
    // other1 - res2
    //
    // this is done in order for tagging and untagging having to process
    // multiple batches of rows
    // and not just one consequent group of rows
    KisTagSP other1 = tags[1];
    KisTagSP other2 = tags[2];
    KisTagSP special = tags[3];

    // --- tagging
    tagResourceModel.tagResources(other1, { resources[0]->resourceId() });
    tagResourceModel.tagResources(other2, { resources[1]->resourceId() });

    tagResourceModel.tagResources(special, { resources[2]->resourceId() }); // <- special
    tagResourceModel.tagResources(special, { resources[1]->resourceId() }); // <- special

    tagResourceModel.tagResources(other2, { resources[2]->resourceId() });

    tagResourceModel.tagResources(special, { resources[0]->resourceId() }); // <- special

    tagResourceModel.tagResources(other1, { resources[1]->resourceId() });

    // --- untagging

    checker.reset();
    checker.setInfo({4, 4, 5, 5}, {5, 5, 5, 5}, {SC::AboutRemove, SC::Remove, SC::AboutRemove, SC::Remove});

    tagResourceModel.untagResources(special, QVector<int>() << resourceIds[2] << resourceIds[1] << resourceIds[0]);

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");



    // *** *** //
    checker.reset();
    checker.setInfo({5, 5, 4, 4}, {5, 5, 5, 5}, {SC::AboutInsert, SC::Insert, SC::AboutInsert, SC::Insert});

    tagResourceModel.tagResources(special, { resourceIds[2], resourceIds[1], resourceIds[0] });

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");

    // *** *** //

    checker.reset();
    checker.setInfo({4, 4, 5, 5}, {5, 5, 5, 5}, {SC::AboutRemove, SC::Remove, SC::AboutRemove, SC::Remove});

    tagResourceModel.untagResources(special, QVector<int>() << resourceIds[2] << resourceIds[1] << resourceIds[0]);

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");

    // *** *** //

    checker.reset();
    checker.setInfo({5, 5, 4, 4}, {5, 5, 5, 5}, {SC::AboutInsert, SC::Insert, SC::AboutInsert, SC::Insert});

    tagResourceModel.tagResources(special, { resourceIds[0], resourceIds[1], resourceIds[2] });

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");

    // *** *** //

    checker.reset();
    checker.setInfo({4, 4, 5, 5}, {5, 5, 5, 5}, {SC::AboutRemove, SC::Remove, SC::AboutRemove, SC::Remove});

    tagResourceModel.untagResources(special, { resourceIds[2], resourceIds[1], resourceIds[0] });

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");

    // *** *** //

    checker.reset();
    checker.setInfo({}, {}, {}); // no signals expected, because all of them are untagged now

    tagResourceModel.untagResources(special, { resourceIds[2], resourceIds[1], resourceIds[0] });

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");

    // *** *** //

    checker.reset();
    checker.setInfo({5, 5, 4, 4}, {5, 5, 4, 4}, {SC::AboutInsert, SC::Insert, SC::AboutInsert, SC::Insert});

    tagResourceModel.tagResources(special, { resourceIds[2], resourceIds[0] });

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");

    // just to clean up space (to have all resources tagged)
    checker.reset();

    tagResourceModel.tagResources(special, { resourceIds[2], resourceIds[1], resourceIds[0] });

    // *** *** //

    checker.reset();
    checker.setInfo({4, 4, 6, 6}, {4, 4, 6, 6}, {SC::AboutRemove, SC::Remove, SC::AboutRemove, SC::Remove});

    tagResourceModel.untagResources(special, { resourceIds[2], resourceIds[0] });

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");

    // *** *** //

    checker.reset();
    checker.setInfo({4, 4}, {4, 4}, {SC::AboutRemove, SC::Remove});

    tagResourceModel.untagResources(special, { resourceIds[1], resourceIds[0]});

    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");

    // *** *** //
    // check adding multiple new tag-resource assignments at once

    checker.reset();
    checker.setInfo({6, 6, 9, 9}, {8, 8, 11, 11}, {SC::AboutInsert, SC::Insert, SC::AboutInsert, SC::Insert});

    tagModel.addTag("tagExtra1", true, { resources[2], resources[1], resources[0]});
    tagModel.addTag("tagExtra2", true, {});
    KisTagSP tagExtra1 = tagModel.tagForUrl("tagExtra2");
    tagResourceModel.tagResources(tagExtra1, { resourceIds[2], resourceIds[1], resourceIds[0]});


    QVERIFY2(checker.isCorrect(), "Detected incorrect/unexpected signals sent to views");
}

void TestTagResourceModel::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}

#include <sdk/tests/kistest.h>


// ***** MODEL SIGNAL CHECKER Function definitions *****


ModelSignalChecker::ModelSignalChecker(QList<int> _expectedFirsts, QList<int> _expectedLasts, QList<ModelSignalChecker::TYPE> _expectedTypes)
{
    setInfo(_expectedFirsts, _expectedLasts, _expectedTypes);
}

void ModelSignalChecker::setInfo(QList<int> _expectedFirsts, QList<int> _expectedLasts, QList<ModelSignalChecker::TYPE> _expectedTypes)
{
    KIS_ASSERT(_expectedFirsts.count() == _expectedLasts.count() && _expectedFirsts.count() == _expectedTypes.count() && "Incorrect data supplied to SignalChecker");
    expectedAmount = _expectedFirsts.count();

    expectedFirsts = _expectedFirsts;
    expectedLasts = _expectedLasts;
    expectedTypes = _expectedTypes;
    reset();
}

void ModelSignalChecker::reset()
{
    firsts = QList<int>();
    lasts = QList<int>();
    types = QList<TYPE>();
    amount = 0;
}

bool ModelSignalChecker::isCorrect() {
    if (expectedFirsts.count() != firsts.count()) return false;
    if (expectedLasts.count() != lasts.count()) return false;
    if (expectedTypes.count() != types.count()) return false;
    if (expectedAmount != amount) return false;

    for (int i = 0; i < firsts.count(); i++) {
        if (expectedFirsts[i] != firsts[i]) return false;
    }

    for (int i = 0; i < lasts.count(); i++) {
        if (expectedLasts[i] != lasts[i]) return false;
    }

    for (int i = 0; i < types.count(); i++) {
        if (expectedTypes[i] != types[i]) return false;
    }

    return true;
}

void ModelSignalChecker::printOut()
{

    qCritical() << "*** SignalChecker write out ***";

    int n = qMin(qMin(amount, firsts.count()), qMin(lasts.count(), types.count()));
    int nmax = qMax(qMax(amount, firsts.count()), qMax(lasts.count(), types.count()));
    if (n != nmax) {
        qCritical() << "The amounts differ!" << amount << firsts.count() << lasts.count() << types.count();
    }

    for (int i = 0; i < n; i++) {
        QString type = "[type]";
        switch(types[i]) {
        case AboutInsert:
            type = "About Insert";
            break;
        case AboutRemove:
            type = "About Remove";
            break;
        case Insert:
            type = "Insert";
            break;
        case Remove:
            type = "Remove";
            break;
        }
        qCritical() << i << type << firsts[i] << lasts[i];
    }

    qCritical() << "*** SignalChecker write out END ***";
}

void ModelSignalChecker::rowsAboutToBeInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    addSignalInfo(first, last, AboutInsert);
}

void ModelSignalChecker::rowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    addSignalInfo(first, last, Insert);
}

void ModelSignalChecker::rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    addSignalInfo(first, last, AboutRemove);
}

void ModelSignalChecker::rowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    addSignalInfo(first, last, Remove);
}

void ModelSignalChecker::addSignalInfo(int first, int last, ModelSignalChecker::TYPE type)
{
    amount++;
    firsts << first;
    lasts << last;
    types << type;
}

KISTEST_MAIN(TestTagResourceModel)

