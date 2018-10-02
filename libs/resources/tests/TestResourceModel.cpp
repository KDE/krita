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
#include "TestResourceModel.h"

#include <QTest>
#include <QStandardPaths>
#include <QDir>

#include <KisResourceCacheDb.h>
#include <KisResourceLoaderRegistry.h>
#include <KisResourceModel.h>

void TestResourceModel::initTestCase()
{
    QDir dbLocation(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (dbLocation.exists()) {
        QFile(dbLocation.path() + "/" + KisResourceCacheDb::resourceCacheDbFilename).remove();
        dbLocation.rmpath(dbLocation.path());
    }
    bool res = KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QVERIFY(res);
    QVERIFY(KisResourceCacheDb::isValid());

    m_sqlDb = QSqlDatabase::database();

}

void TestResourceModel::testRowCount()
{
    KisResourceModel rm("paintoppresets");
    qDebug() << rm.rowCount();
}

void TestResourceModel::cleanupTestCase()
{
    QDir dbLocation(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    bool res = QFile(dbLocation.path() + "/" + KisResourceCacheDb::resourceCacheDbFilename).remove();
    Q_ASSERT(res);
    res = dbLocation.rmpath(dbLocation.path());
    Q_ASSERT(res);
}

QTEST_MAIN(TestResourceModel)

