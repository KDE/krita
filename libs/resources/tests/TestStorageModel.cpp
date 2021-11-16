/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestStorageModel.h"

#include <simpletest.h>
#include <QStandardPaths>
#include <QDir>
#include <QVersionNumber>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlQuery>
#include <QtSql>
#include <QModelIndex>
#include <QAbstractItemModelTester>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>
#include <KisStorageModel.h>

#include <DummyResource.h>
#include <ResourceTestHelper.h>


#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestStorageModel::initTestCase()
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

void TestStorageModel::testWithTagModelTester()
{
    KisStorageModel model;
    auto tester = new QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    Q_UNUSED(tester);
}


void TestStorageModel::testRowCount()
{
    QSqlQuery q;
    QVERIFY(q.prepare("SELECT count(*)\n"
                      "FROM   storages"));
    QVERIFY(q.exec());
    q.first();
    int rowCount = q.value(0).toInt();

    KisStorageModel storageModel;
    QCOMPARE(storageModel.rowCount(), rowCount);
}

void TestStorageModel::testSetActive()
{
    KisStorageModel storageModel;

    for (int i = 0; i < storageModel.rowCount(); ++i)  {

        QModelIndex idx = storageModel.index(i, 0);

        storageModel.setData(idx, QVariant(true), Qt::CheckStateRole);

        idx = storageModel.index(i, 0);
        QVERIFY(idx.data(Qt::UserRole + KisStorageModel::Active).toBool() == true);

        storageModel.setData(idx, QVariant(false), Qt::CheckStateRole);

        idx = storageModel.index(i, 0);
        QVERIFY(idx.data(Qt::UserRole + KisStorageModel::Active).toBool() == false);

    }
}


void TestStorageModel::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}

void TestStorageModel::testMetaData()
{
    KisStorageModel storageModel;
    int rowCount = storageModel.rowCount();

    KisResourceStorageSP storage {new KisResourceStorage("My Named Memory Storage")};
    KisResourceLocator::instance()->addStorage("My Named Memory Storage", storage);
    storage->setMetaData(KisResourceStorage::s_meta_name, "My Named Memory Storage");

    QVERIFY(storageModel.rowCount() > rowCount);

    QModelIndex idx;
    for (int row = 0; row < storageModel.rowCount(); ++row) {
        idx = storageModel.index(row, 7);
        KisResourceStorageSP st = storageModel.storageForIndex(idx);
        QVERIFY(st);
        if (st == storage) {
            break;
        }
    }

    QVERIFY(idx.isValid());

    QString displayName = storageModel.data(idx, Qt::DisplayRole).toString();
    QCOMPARE("My Named Memory Storage", displayName);

    idx = storageModel.index(idx.row(), 0);
    QMap<QString, QVariant> metadata = storageModel.data(idx, Qt::UserRole + KisStorageModel::MetaData).toMap();
    QVERIFY(metadata.contains(KisResourceStorage::s_meta_name));
    QVERIFY(metadata[KisResourceStorage::s_meta_name] == "My Named Memory Storage");
}

void TestStorageModel::testImportStorage()
{

}




SIMPLE_TEST_MAIN(TestStorageModel)

