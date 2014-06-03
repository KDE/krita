/* This file is part of the KDE project
   Copyright (C) 2003-2006 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

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

#ifndef _KEXI_ERROR_H_
#define _KEXI_ERROR_H_

#include <QString>
#include "global.h"
#include "calligradb_export.h"

/*! Fine-grained KexiDB error codes */

#define ERR_NONE 0
#define ERR_NO_NAME_SPECIFIED 9 //! used when name (e.g. for database) was not specified
#define ERR_DRIVERMANAGER 10
#define ERR_INVALID_IDENTIFIER 11 //! used when name (e.g. for database) was not specified
#define ERR_MISSING_DB_LOCATION 20
#define ERR_ALREADY_CONNECTED 30
#define ERR_NO_CONNECTION 40 //!< when opened connection was expected using KexiDB::Connection
#define ERR_CONNECTION_FAILED 41 //!< when connection has failed
#define ERR_CLOSE_FAILED 42 //!< when closing has failed
#define ERR_NO_DB_USED 43 //!< when used database was expected in KexiDB::Connection
#define ERR_OBJECT_EXISTS 50
#define ERR_OBJECT_THE_SAME 51
#define ERR_OBJECT_NOT_FOUND 60
#define ERR_ACCESS_RIGHTS 70
#define ERR_TRANSACTION_ACTIVE 80
#define ERR_NO_TRANSACTION_ACTIVE 81
#define ERR_NO_DB_PROPERTY 90 //! database property not found, see DatabaseProperties class
#define ERR_DB_SPECIFIC 100
#define ERR_CURSOR_NOT_OPEN 110
#define ERR_SINGLE_DB_NAME_MISMATCH 120
#define ERR_CURSOR_RECORD_FETCHING 130 //!< eg. for Cursor::drv_getNextRecord()
#define ERR_UNSUPPORTED_DRV_FEATURE 140 //!< given driver's feature is unsupported (eg. transactins)
#define ERR_ROLLBACK_OR_COMMIT_TRANSACTION 150 //!< error during transaction rollback or commit
#define ERR_SYSTEM_NAME_RESERVED 160 //!< system name is reserved and cannot be used 
//!< (e.g. for table, db, or field name)
#define ERR_CANNOT_CREATE_EMPTY_OBJECT 170 //!< empty object cannot be created
//!< (e.g. table without fields)
#define ERR_INVALID_DRIVER_IMPL 180 //! driver's implementation is invalid
#define ERR_INCOMPAT_DRIVER_VERSION 181 //!< driver's version is incompatible
#define ERR_INCOMPAT_DATABASE_VERSION 182 //!< db's version is incompatible with currently 
//!< used Kexi version
#define ERR_INVALID_DATABASE_CONTENTS 183 //!< db's contents are invalid 
//!< (e.g. no enough information to open db)

//! errors related to data updating on the server
#define ERR_UPDATE_NULL_PKEY_FIELD 190 //!< null pkey field on updating
#define ERR_UPDATE_SERVER_ERROR 191    //!< error @ the server side during data updating
#define ERR_UPDATE_NO_MASTER_TABLE 192 //!< data could not be edited because there
//!< is no master table defined
#define ERR_UPDATE_NO_MASTER_TABLES_PKEY 193 //!< data could not be edited 
//!< because it's master table has
//!< no primary key defined
#define ERR_UPDATE_NO_ENTIRE_MASTER_TABLES_PKEY 194 //!< data could not be edited 
//!< because it does not contain entire
//!< master table's primary key

//! errors related to data inserting on the server
#define ERR_INSERT_NULL_PKEY_FIELD 220 //!< null pkey field on updating
#define ERR_INSERT_SERVER_ERROR 221    //!< error @ the server side during data inserting
#define ERR_INSERT_NO_MASTER_TABLE 222 //!< data could not be inserted because there
//!< is no master table defined
#define ERR_INSERT_NO_MASTER_TABLES_PKEY 223 //!< data could not be inserted because master 
//!< table has no primary key defined
#define ERR_INSERT_NO_ENTIRE_MASTER_TABLES_PKEY 224 //!< data could not be inserted
//!< because it does not contain entire
//!< master table's primary key

//! errors related to data deleting on the server
#define ERR_DELETE_NULL_PKEY_FIELD 250 //!< null pkey field on updating
#define ERR_DELETE_SERVER_ERROR 251    //!< error @ the server side during data deleting
#define ERR_DELETE_NO_MASTER_TABLE 252 //!< data could not be deleted because there
//!< is no master table defined
#define ERR_DELETE_NO_MASTER_TABLES_PKEY 253 //!< data could not be deleted because master
//!< table has no primary key defined
#define ERR_DELETE_NO_ENTIRE_MASTER_TABLES_PKEY 254 //!< data could not be deleted
//!< because it does not contain entire
//!< master table's primary key

//! errors related to queries
#define ERR_SQL_EXECUTION_ERROR 260 //!< general server error for sql statement execution
//!< usually returned by Connection::executeSQL()
#define ERR_SQL_PARSE_ERROR 270 //!< Parse error coming from arser::parse()

#define ERR_OTHER 0xffff //!< use this if you have not (yet?) the name for given error 


namespace KexiDB
{

/*! This class contains a result information
 for various data manipulation operations, like cell/row updating/inserting. */
class CALLIGRADB_EXPORT ResultInfo
{
public:
    ResultInfo() {
        success = true;
        allowToDiscardChanges = false;
        column = -1;
    }
    /*! Sets information to default values. */
    void clear() {
        success = true;
        allowToDiscardChanges = false;
        column = -1;
        msg.clear();
        desc.clear();
    }
    bool success : 1; //!< result of the operation, true by default
    bool allowToDiscardChanges : 1; //!< True if changes can be discarded, false by default
    //!< If true, additional "Discard changes" messagebox
    //!< button can be displayed.
    QString msg, desc; //!< error message and detailed description, both empty by default
    int column; //!< faulty column, -1 (the default) means: there is no faulty column
};

}//namespace

#endif

