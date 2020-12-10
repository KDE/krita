/*
 * Copyright (c) 2019 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestResourceServer.h"

#include <QTest>
#include <QStandardPaths>
#include <QDir>
#include <QVersionNumber>
#include <QDirIterator>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryFile>
#include <QtSql>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>
#include <KisResourceLoaderRegistry.h>
#include <KisResourceModel.h>

#include <DummyResource.h>
#include <ResourceTestHelper.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestResourceServer::initTestCase()
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
        qWarning() << "Could not initialize KisResourceCacheDb on" << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    QVERIFY(KisResourceCacheDb::isValid());

    KisResourceLocator::LocatorError r = m_locator->initialize(m_srcLocation);
    if (!m_locator->errorMessages().isEmpty()) {
        qDebug() << m_locator->errorMessages();
    }

    QVERIFY(r == KisResourceLocator::LocatorError::Ok);
    QVERIFY(QDir(m_dstLocation).exists());
}

void TestResourceServer::testFirstResource()
{

}

void TestResourceServer::testResourceModel()
{

}

void TestResourceServer::testResourceCount()
{

}

void TestResourceServer::testRemoveResourceFromServer()
{

}

void TestResourceServer::testSaveLocation()
{

}

void TestResourceServer::testImportResourceFile()
{

}

void TestResourceServer::testRemoveResourceFile()
{

}

void TestResourceServer::testAddObserver()
{

}

void TestResourceServer::testRemoveObserver()
{

}

void TestResourceServer::testResourceByFileName()
{

}

void TestResourceServer::testResourceByName()
{

}

void TestResourceServer::testResourceByMD5()
{

}

void TestResourceServer::testUpdateResource()
{

}

void TestResourceServer::testAssignedTagsList()
{

}

void TestResourceServer::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}



QTEST_MAIN(TestResourceServer)
