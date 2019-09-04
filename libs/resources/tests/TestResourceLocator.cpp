/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestResourceLocator.h"

#include <QTest>
#include <QVersionNumber>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlQuery>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KritaVersionWrapper.h>
#include <KisResourceCacheDb.h>

#include <KisResourceLocator.h>
#include <KisResourceLoaderRegistry.h>

#include <DummyResource.h>
#include <ResourceTestHelper.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif

void TestResourceLocator::initTestCase()
{
    ResourceTestHelper::initTestDb();

    m_srcLocation = QString(FILES_DATA_DIR);
    QVERIFY2(QDir(m_srcLocation).exists(), m_srcLocation.toUtf8());

    m_dstLocation = QString(FILES_DEST_DIR);
    ResourceTestHelper::cleanDstLocation(m_dstLocation);

    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    cfg.writeEntry(KisResourceLocator::resourceLocationKey, m_dstLocation);

    m_locator = KisResourceLocator::instance();

    ResourceTestHelper::createDummyLoaderRegistry();
}

void TestResourceLocator::testLocatorInitalization()
{
    KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    KisResourceLocator::LocatorError r = m_locator->initialize(m_srcLocation);
    if (!m_locator->errorMessages().isEmpty()) qDebug() << m_locator->errorMessages();
    QVERIFY(r == KisResourceLocator::LocatorError::Ok);
    QVERIFY(QDir(m_dstLocation).exists());
    Q_FOREACH(const QString &folder, KisResourceLoaderRegistry::instance()->resourceTypes()) {
        QDir dstDir(m_dstLocation + '/' + folder + '/');
        QDir srcDir(m_srcLocation + '/' + folder + '/');

        QVERIFY(dstDir.exists());
        QVERIFY(dstDir.entryList(QDir::Files | QDir::NoDotAndDotDot) == srcDir.entryList(QDir::Files | QDir::NoDotAndDotDot));
    }

    QFile f(m_dstLocation + '/' + "KRITA_RESOURCE_VERSION");
    QVERIFY(f.exists());
    f.open(QFile::ReadOnly);
    QVersionNumber version = QVersionNumber::fromString(QString::fromUtf8(f.readAll()));
    QVERIFY(version == QVersionNumber::fromString(KritaVersionWrapper::versionString()));

}

void TestResourceLocator::testStorageInitialization()
{
    Q_FOREACH(KisResourceStorageSP storage, m_locator->storages()) {
        QVERIFY(KisResourceCacheDb::addStorage(storage, true));
    }
    QSqlQuery query;
    bool r = query.exec("SELECT COUNT(*) FROM storages");
    QVERIFY(r);
    QVERIFY(query.lastError() == QSqlError());
    query.first();

    Q_FOREACH(KisResourceStorageSP storage, m_locator->storages()) {
        qDebug() << storage;
    }

    QCOMPARE(query.value(0).toInt(), m_locator->storages().count());
}

void TestResourceLocator::testLocatorSynchronization()
{
    QVERIFY(m_locator->synchronizeDb());
}

void TestResourceLocator::testResources()
{
    QSqlQuery query;
    bool r = query.exec("SELECT COUNT(*) FROM resources");
    QVERIFY(r);
    QVERIFY(query.lastError() == QSqlError());
    query.first();
    QCOMPARE(query.value(0).toInt(), 7);
}

void TestResourceLocator::testTags()
{
    QSqlQuery query;
    bool r = query.exec("SELECT COUNT(*) FROM tags");
    QVERIFY(r);
    QVERIFY(query.lastError() == QSqlError());
    query.first();
    QCOMPARE(query.value(0).toInt(), 1);
}


void TestResourceLocator::testResourceLocationBase()
{
    QCOMPARE(m_locator->resourceLocationBase(), QString(FILES_DEST_DIR));
}

void TestResourceLocator::testResource()
{
    KoResourceSP res = m_locator->resource("", "paintoppresets", "test0.kpp");
    QVERIFY(res);
}

void TestResourceLocator::testResourceForId()
{
    KoResourceSP res = m_locator->resource("", "paintoppresets", "test0.kpp");
    int resourceId = KisResourceCacheDb::resourceIdForResource("test0.kpp", "paintoppresets", "");
    QVERIFY(resourceId > -1);
    KoResourceSP res2 = m_locator->resourceForId(resourceId);
    QCOMPARE(res, res2);
}

void TestResourceLocator::testRemoveResource()
{

}

void TestResourceLocator::testImportResourceFromFile()
{

}

void TestResourceLocator::testAddResource()
{

}

void TestResourceLocator::testUpdateResource()
{

}



void TestResourceLocator::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}

QTEST_MAIN(TestResourceLocator)

