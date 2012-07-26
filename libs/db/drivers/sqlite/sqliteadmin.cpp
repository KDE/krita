/* This file is part of the KDE project
   Copyright (C) 2006 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QDir>

#include "sqliteadmin.h"
#include <db/drivermanager.h>
#include <db/driver_p.h>

#include "sqlitevacuum.h"

SQLiteAdminTools::SQLiteAdminTools()
        : KexiDB::AdminTools()
{
}

SQLiteAdminTools::~SQLiteAdminTools()
{
}

bool SQLiteAdminTools::vacuum(const KexiDB::ConnectionData& data, const QString& databaseName)
{
    clearError();
    KexiDB::DriverManager manager;
    KexiDB::Driver *drv = manager.driver(data.driverName);
    QString title(i18n("Could not compact database \"%1\".", QDir::convertSeparators(databaseName)));
    if (!drv) {
        setError(&manager, title);
        return false;
    }
    SQLiteVacuum vacuum(data.dbPath() + QDir::separator() + databaseName);
    tristate result = vacuum.run();
    if (!result) {
        setError(title);
        return false;
    } else //success or cancelled
        return true;
}

