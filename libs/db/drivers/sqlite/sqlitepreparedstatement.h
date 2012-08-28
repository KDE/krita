/* This file is part of the KDE project
   Copyright (C) 2005-2012 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_SQLITEPREPAREDSTATEMENT_H
#define KEXIDB_SQLITEPREPAREDSTATEMENT_H

#include <db/preparedstatement.h>
#include "sqliteconnection_p.h"

namespace KexiDB
{

/*! Implementation of prepared statements for SQLite driver. */
class SQLitePreparedStatement : public PreparedStatement, SQLiteConnectionInternal
{
public:
    SQLitePreparedStatement(StatementType type, ConnectionInternal& conn,
                            FieldList& fields);

    virtual ~SQLitePreparedStatement();

    virtual bool execute();

    sqlite3_stmt *prepared_st_handle;
    bool m_resetRequired : 1;
};

}

#endif
