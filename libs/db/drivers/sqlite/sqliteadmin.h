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

#ifndef KEXIDB_SQLITEADMIN_H
#define KEXIDB_SQLITEADMIN_H

#include <db/admin.h>

//! @short An interface containing a set of tools for SQLite database administration.
class SQLiteAdminTools : public KexiDB::AdminTools
{
public:
    SQLiteAdminTools();
    virtual ~SQLiteAdminTools();

    /*! Performs vacuum (compacting) for connection \a conn. */
    virtual bool vacuum(const KexiDB::ConnectionData& data, const QString& databaseName);
};

#endif
