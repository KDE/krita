/* This file is part of the KDE project
   Copyright (C) 2003 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_SQLITECONN_P_H
#define KEXIDB_SQLITECONN_P_H

#include <db/connection_p.h>

#include <sqlite3.h>

namespace KexiDB
{

/*! Internal SQLite connection data. Also used inside SQLiteCursor. */
class SQLiteConnectionInternal : public ConnectionInternal
{
public:
    SQLiteConnectionInternal(Connection* connection);
    virtual ~SQLiteConnectionInternal();

    //! stores last result's message
    virtual void storeResult();

    bool extensionsLoadingEnabled() const;

    void setExtensionsLoadingEnabled(bool set);

    sqlite3 *data;
    bool data_owned; //!< true if data pointer should be freed on destruction
    QString errmsg; //<! server-specific message of last operation
    char *errmsg_p; //<! temporary: server-specific message of last operation
    int res; //<! result code of last operation on server

    QByteArray temp_st;
    const char *result_name;

private:
    bool m_extensionsLoadingEnabled;
};

}

#endif
