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

#include "TestFolderStorage.h"
#include <QTest>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KritaVersionWrapper.h>

#include <KisFolderStorage.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisResourceLoaderRegistry.h>
#include <KisResourceCacheDb.h>

#include <KisResourceLocator.h>
#include <KisResourceLoaderRegistry.h>

#include "DummyResource.h"
#include "ResourceTestHelper.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif


void TestFolderStorage::initTestCase()
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

    KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    m_locator->initialize(m_srcLocation);

    if (!m_locator->errorMessages().isEmpty()) qDebug() << m_locator->errorMessages();
}

void TestFolderStorage ::testStorage()
{

    KisFolderStorage folderStorage(m_dstLocation);

    KisResourceLoaderRegistry::instance()->add(ResourceType::Brushes, new KisResourceLoader<DummyResource>("dummy", ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush"));
    QSharedPointer<KisResourceStorage::ResourceIterator> iter = folderStorage.resources(ResourceType::Brushes);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        qDebug() << iter->url() << iter->type() << iter->lastModified();
        count++;
    }
    QVERIFY(count == 1);
}

void TestFolderStorage::testTagIterator()
{
    KisFolderStorage folderStorage(m_dstLocation);
    QSharedPointer<KisResourceStorage::TagIterator> iter = folderStorage.tags(ResourceType::PaintOpPresets);
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        qDebug() << iter->url() << iter->name() << iter->tag();
        count++;
    }
    QVERIFY(count == 1);
}

void TestFolderStorage::testAddResource()
{
    KoResourceSP resource(new DummyResource("anewresource.kpp"));
    resource->setValid(true);
    resource->setVersion(0);

    KisFolderStorage folderStorage(QString(FILES_DEST_DIR));
    bool r = folderStorage.addResource("paintoppresets", resource);
    QVERIFY(r);
    QString fileName = resource->filename();

    resource.dynamicCast<DummyResource>()->setSomething("It's changed");
    r = folderStorage.addResource("paintoppresets", resource);
    QVERIFY(r);
    QVERIFY(resource->filename() != fileName);
    QVERIFY(resource->version() == 1);

    QDir d(m_dstLocation + "/" + "paintoppresets");
    QVERIFY(d.entryList().contains("anewresource_0001.kpp"));
}

void TestFolderStorage::cleanupTestCase()
{
    ResourceTestHelper::rmTestDb();
    ResourceTestHelper::cleanDstLocation(m_dstLocation);
}

QTEST_MAIN(TestFolderStorage)

