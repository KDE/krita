/* This file is part of the KDE project
    Copyright (C) 2003-2007 Jarosław Staniek <staniek@kde.org>
    Copyright (C) 2004 Martin Ellis <martin.ellis@kdemail.net>

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

#include <KDebug>
#include <KLocale>
#include "driver_p.h"

using namespace KexiDB;

DriverPrivate::DriverPrivate()
        : isFileDriver(false)
        , isDBOpenedAfterCreate(false)
        , features(Driver::NoFeatures)
{
    adminTools = 0;

    properties["client_library_version"] = "";
    propertyCaptions["client_library_version"] =
        i18n("Client library version");

    properties["default_server_encoding"] = "";
    propertyCaptions["default_server_encoding"] =
        i18n("Default character encoding on server");
}

void DriverPrivate::initInternalProperties()
{
    properties["is_file_database"] = QVariant(isFileDriver);
    propertyCaptions["is_file_database"] = i18n("File-based database driver");
    if (isFileDriver) {
        properties["file_database_mimetype"] = fileDBDriverMimeType;
        propertyCaptions["file_database_mimetype"] = i18n("File-based database's MIME type");
    }

#if 0
    QString str;
    if (features & Driver::SingleTransactions)
        str = i18n("Single transactions");
    else if (features & Driver::MultipleTransactions)
        str = i18n("Multiple transactions");
    else if (features & Driver::NestedTransactions)
        str = i18n("Nested transactions");
    else if (features & Driver::IgnoreTransactions)
        str = i18n("Ignored");
    else
        str = i18n("None");
#endif
// properties["transaction_support"] = features & Driver::TransactionsMask;
// propertyCaptions["transaction_support"] = i18n("Transaction support");
    properties["transaction_single"] = QVariant(features & Driver::SingleTransactions);
    propertyCaptions["transaction_single"] = i18n("Single transactions support");
    properties["transaction_multiple"] = QVariant(features & Driver::MultipleTransactions);
    propertyCaptions["transaction_multiple"] = i18n("Multiple transactions support");
    properties["transaction_nested"] = QVariant(features & Driver::NestedTransactions);
    propertyCaptions["transaction_nested"] = i18n("Nested transactions support");

    properties["kexidb_driver_version"] =
        QString("%1.%2").arg(version().major).arg(version().minor);
    propertyCaptions["kexidb_driver_version"] =
        i18n("KexiDB driver version");
}

DriverPrivate::~DriverPrivate()
{
    delete adminTools;
}

//--------------------------

AdminTools::Private::Private()
{
}

AdminTools::Private::~Private()
{
}
