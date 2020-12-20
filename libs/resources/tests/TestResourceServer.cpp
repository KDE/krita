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

#include <KoSegmentGradient.h>
#include <KoStopGradient.h>
#include <KoResourceServerProvider.h>
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

    // Replace the dummy loaders with real gradient loaders
    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();
    reg->add(new KisResourceLoader<KoSegmentGradient>(ResourceSubType::SegmentedGradients, ResourceType::Gradients, i18n("Gradients"), QStringList() << "application/x-gimp-gradient"));
    reg->add(new KisResourceLoader<KoStopGradient>(ResourceSubType::StopGradients, ResourceType::Gradients, i18n("Gradients"), QStringList() << "application/x-karbon-gradient" << "image/svg+xml"));

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
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
    KoAbstractGradientSP gradient = gradientServer->firstResource();
    QVERIFY(gradient);
}

void TestResourceServer::testResourceModel()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
    QVERIFY(gradientServer->resourceModel());
}

void TestResourceServer::testResourceCount()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
    QCOMPARE(gradientServer->resourceCount(), 3);
}

void TestResourceServer::testRemoveResourceFromServer()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testSaveLocation()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
    qDebug() << gradientServer->saveLocation();
}

void TestResourceServer::testImportResourceFile()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testRemoveResourceFile()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testAddObserver()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testRemoveObserver()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testResourceByFileName()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testResourceByName()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testResourceByMD5()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testUpdateResource()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::testAssignedTagsList()
{
    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::gradientServer();
}

void TestResourceServer::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}



QTEST_MAIN(TestResourceServer)
