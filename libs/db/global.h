/* This file is part of the KDE project
   Copyright (C) 2003-2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_GLOBAL_H
#define KEXIDB_GLOBAL_H

#include "calligradb_export.h"
#include <QString>

//global public definitions

/*! KexiDB implementation version.
 It is altered after every API change:
 - major number is increased after KexiDB storage format change,
 - minor is increased after adding binary-incompatible change.
 In external code: do not use this to get library version information:
 use KexiDB::versionMajor() and KexiDB::versionMinor() instead to get real version.
*/
#define KEXIDB_VERSION_MAJOR 1
#define KEXIDB_VERSION_MINOR 9

/*! KexiDB implementation version. @see KEXIDB_VERSION_MAJOR, KEXIDB_VERSION_MINOR */
#define KEXIDB_VERSION KexiDB::DatabaseVersionInfo(KEXIDB_VERSION_MAJOR, KEXIDB_VERSION_MINOR)

/*! \namespace KexiDB
\brief High-level database connectivity library with database backend drivers

\section Framework
DriverManager

Database access
 - Connection
 - ConnectionData

Database structure
 - Schema
  - tableschema
  - queryschema
  - indexschema

Stored in the database.


Data representation
 - Record
 - Field


\section Drivers

Drivers are loaded using DriverManager::driver(const QString& name).  The names
of drivers are given in their drivers .desktop file in the
X-Kexi-DriverName field.

KexiDB supports two kinds of databases: file-based and network-based databases.
The type of a driver is available from several places. The X-Kexi-DriverType
field in the driver's .desktop file, is read by the DriverManager and
available by calling DriverManager::driverInfo(const QString &name) and using
the Driver::Info#fileBased member from the result. Given a reference to a
Driver, its type can also be found directly using Driver::isFileDriver() const.

Each database backend driver consists of three main classes: a driver,
a connection and a cursor class, e.g SQLiteDriver, SQLiteConnection,
SQLiteCursor.

The driver classes subclass the Driver class.  They set Driver#m_typeNames,
which maps KexiDB's Field::Type on to the types supported by the database.  They also
provide functions for escaping strings and checking table names.  These may be
used, for example, on a database backend that uses the database name as a
filename.  In this case, it should be ensured that all the characters in the
database name are valid characters in a filename.

The connection classes subclass the Connection class, and include most of the
calls to the native database API.

The cursor classes subclass Cursor, and implement cursor functionality specific
to the database backend.

*/
namespace KexiDB
{

#define KexiDBDbg  kDebug(44000)   //! Debug area for core KexiDB code
#define KexiDBDrvDbg kDebug(44001) //! Debug area for KexiDB's drivers implementation code
#define KexiDBWarn  kWarning(44000)
#define KexiDBDrvWarn kWarning(44001)
#define KexiDBFatal kFatal(44000)

/*! @short Contains database version information about a Kexi-compatible database.
 The version is stored as internal database properties. */
class CALLIGRADB_EXPORT DatabaseVersionInfo
{
public:
    DatabaseVersionInfo();
    DatabaseVersionInfo(uint majorVersion, uint minorVersion);

    bool matches(uint _major, uint _minor) const { return _major == major && _minor == minor; }

    //! Major version number, e.g. 1 for 1.8
    uint major;

    //! Minor version number, e.g. 8 for 1.8
    uint minor;
};

//! \return KexiDB version info
CALLIGRADB_EXPORT DatabaseVersionInfo version();

/*! @short Contains version information about a database backend. */
class CALLIGRADB_EXPORT ServerVersionInfo
{
public:
    ServerVersionInfo();

    //! Clears the information - integers will be set to 0 and string to null
    void clear();

    //! Major version number, e.g. 1 for 1.2.3
    uint major;

    //! Minor version number, e.g. 2 for 1.2.3
    uint minor;

    //! Release version number, e.g. 3 for 1.2.3
    uint release;

    //! Version string, as returned by the server
    QString string;
};

/*! Object types set like table or query. */
enum ObjectType {
    UnknownObjectType = -1, //!< helper
    AnyObjectType = 0,      //!< helper
    TableObjectType = 1,
    QueryObjectType = 2,
    LastObjectType = 2, //ALWAYS UPDATE THIS

    KexiDBSystemTableObjectType = 128,//!< helper, not used in storage
    //!< (allows to select kexidb system tables
    //!< may be or'd with TableObjectType)
    IndexObjectType = 256 //!< special
};

}

#ifndef futureI18n
# define futureI18n QString
# define futureI18n2(a,b) QString(b)
#endif

#ifndef FUTURE_I18N_NOOP
# define FUTURE_I18N_NOOP(x) (x)
#endif

#endif
