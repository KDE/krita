/* This file is part of the KDE project
   Copyright (C) 2012 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TestConnection.h"

#include <db/drivermanager.h>
#include <db/driver.h>
#include <db/connection.h>

#include <kdebug.h>

#include <QFile>
#include <QTest>

KexiDB::Connection *conn = 0;

#include "tables_test.h"

void TestConnection::initTestCase()
{
}

void TestConnection::testCreateDb()
{
    QString drv_name = "sqlite3";
    QString db_name(QFile::decodeName(FILES_OUTPUT_DIR "test.kexi"));

    KexiDB::DriverManager manager;
    QStringList names = manager.driverNames();
    kDebug() << "DRIVERS: ";
    for (QStringList::ConstIterator it = names.constBegin(); it != names.constEnd() ; ++it)
        kDebug() << *it;
    QVERIFY2(!manager.error(), "Error in driver manager");
    manager.debugError();
    QVERIFY2(!names.isEmpty(), "No db drivers found");

    //get driver
    const KexiDB::Driver::Info drv_info = manager.driverInfo(drv_name);
    KexiDB::Driver *driver = manager.driver(drv_name);
    QVERIFY2(!manager.error() && driver, "Error in driver manager after DriverManager::driver() call");
    QVERIFY2(!drv_info.name.isEmpty(), "Driver info not empty");
    QCOMPARE(drv_info.name, drv_name);
    QCOMPARE(drv_info.fileBased, true);

    //open connection
    KexiDB::ConnectionData cdata;
    cdata.setFileName(db_name);
    conn = driver->createConnection(cdata);
    kDebug() << driver->errorMsg();
    QVERIFY2(!driver->error() && conn, "Failed to create connection");

    {
        QScopedPointer<KexiDB::Connection> connGuard(conn);

        QVERIFY2(conn->connect(), "Failed to connect");
        if (conn->databaseExists(db_name)) {
            QVERIFY2(conn->dropDatabase(db_name), "Failed to drop database");
        }
        QVERIFY2(conn->createDatabase(db_name), "Failed to create db");
        QVERIFY2(conn->useDatabase(db_name), "Failed to use db");

        QVERIFY2(tablesTest() == 0, "Failed to create test data");

        QVERIFY2(conn->disconnect(), "Failed to disconnect database");
    }
    conn = 0;
}

void TestConnection::cleanupTestCase()
{
}

QTEST_MAIN(TestConnection)
