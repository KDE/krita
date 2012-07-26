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

#ifndef KEXIDB_ADMIN_H
#define KEXIDB_ADMIN_H

#include "object.h"

namespace KexiDB
{
class Connection;
class ConnectionData;

//! @short An interface containing a set of tools for database administration
/*! Can be implemented in database drivers. @see Driver::adminTools
*/
class CALLIGRADB_EXPORT AdminTools : public Object
{
public:
    AdminTools();
    virtual ~AdminTools();

    /*! Performs vacuum (compacting) for connection \a data.
     Can be implemented for your driver.
     Note: in most cases the database should not be opened.

     Currently it is implemented for SQLite drivers.

     \return true on success, false on failure
     (then you can get error status from the AdminTools object). */
    virtual bool vacuum(const ConnectionData& data, const QString& databaseName);
    //virtual bool vacuum(Connection& conn);

protected:
    class Private;
    Private * const d;
};
}

#endif
