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

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisResourceLocator.h>
#include <KritaVersionWrapper.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif

void TestResourceLocator::initTestCase()
{
    m_srcLocation = QString(FILES_DATA_DIR);
    QVERIFY2(QDir(m_srcLocation).exists(), m_srcLocation.toUtf8());
    m_dstLocation = QString(FILES_DEST_DIR);
    cleanDstLocation();
    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    cfg.writeEntry(KisResourceLocator::resourceLocationKey, m_dstLocation);
}

void TestResourceLocator::testLocatorInitalization()
{
    KisResourceLocator::LocatorError r = m_locator.initialize(m_srcLocation);
    if (!m_locator.errorMessages().isEmpty()) qDebug() << m_locator.errorMessages();
    QVERIFY(r == KisResourceLocator::LocatorError::Ok);
    QVERIFY(QDir(m_dstLocation).exists());
    Q_FOREACH(const QString &folder, KisResourceLocator::resourceTypeFolders) {
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

void TestResourceLocator::testLocatorSynchronization()
{
    QVERIFY(m_locator.synchronizeDb());
}

void TestResourceLocator::cleanupTestCase()
{
    cleanDstLocation();
}

bool TestResourceLocator::cleanDstLocation()
{
    if (QDir(m_dstLocation).exists()) {
        {
            QDirIterator iter(m_dstLocation, QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
            while (iter.hasNext()) {
                iter.next();
                QFile f(iter.filePath());
                f.remove();
                //qDebug() << (r ? "Removed" : "Failed to remove") << iter.filePath();
            }
        }
        {
            QDirIterator iter(m_dstLocation, QStringList() << "*", QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (iter.hasNext()) {
                iter.next();
                QDir(iter.filePath()).rmpath(iter.filePath());
                //qDebug() << (r ? "Removed" : "Failed to remove") << iter.filePath();
            }
        }

        return QDir().rmpath(m_dstLocation);
    }
    return true;
}

QTEST_MAIN(TestResourceLocator)

