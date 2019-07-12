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
#include "TestResourceTypeModel.h"

#include <QTest>
#include <QStandardPaths>
#include <QDir>
#include <QVersionNumber>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlQuery>
#include <QtSql>
#include <QModelIndex>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>
#include <KisResourceLoaderRegistry.h>
#include <KisResourceTypeModel.h>

#include <DummyResource.h>
#include <ResourceTestHelper.h>


#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestResourceTypeModel::initTestCase()
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


void TestResourceTypeModel::testRowCount()
{
    QSqlQuery q;
    QVERIFY(q.prepare("SELECT count(*)\n"
                      "FROM   resource_types"));
    QVERIFY(q.exec());
    q.first();
    int rowCount = q.value(0).toInt();
    QCOMPARE(rowCount, KisResourceLoaderRegistry::instance()->resourceTypes().count());

    KisResourceTypeModel typeModel;
    QCOMPARE(typeModel.rowCount(), rowCount);
}

void TestResourceTypeModel::testData()
{
    KisResourceTypeModel typeModel;
    for(int i = 0; i < typeModel.rowCount(); ++i) {

        QModelIndex idx = typeModel.index(0, KisResourceTypeModel::ResourceType);
//        qDebug() << "test" << idx.data(Qt::DisplayRole)
//                 << idx.data(Qt::UserRole + KisResourceTypeModel::Id)
//                 << idx.data(Qt::UserRole + KisResourceTypeModel::ResourceType)
//                 << idx.data(Qt::UserRole + KisResourceTypeModel::Name);

        QVERIFY(KisResourceLoaderRegistry::instance()->resourceTypeLoaders(idx.data(Qt::DisplayRole).toString()).size() > 0);

        QVector<KisResourceLoaderBase *> loaders = KisResourceLoaderRegistry::instance()->resourceTypeLoaders(idx.data(Qt::UserRole + KisResourceTypeModel::ResourceType).toString());

        QVERIFY(loaders.size() > 0);

        auto loader = loaders.first();
        QCOMPARE(loader->name(), idx.data(Qt::UserRole + KisResourceTypeModel::Name).toString());
    }
}


void TestResourceTypeModel::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}




QTEST_MAIN(TestResourceTypeModel)

