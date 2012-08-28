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

#ifndef KEXIDB_CONNECTION_DATA_H
#define KEXIDB_CONNECTION_DATA_H

#include "calligradb_export.h"

#include <QObject>
#include <QString>
#include <QList>

namespace KexiDB
{

/*! ConnectionDataBase is a helper class for ConnectionData.  It
 is not intended to be instantiated explicitly.  Instead, use the
 ConnectionData class. */
/*! @internal
 Used by ConnectionData.
 It is easier to internally operate on non-QObject-derived object,
 e.g.: to copy data members in ConnectionData ctor. */
class ConnectionDataBase
{
public:
    /*!
    \brief The caption of the connection.

    Captions are optional for identyfying given connection
    by name eg. for users.
    */
    QString caption;

    /*!
    \brief The additional description for the connection
    */
    QString description;

    /*!
    \brief Used for identifying a single piece of data in a set

    Optional ID used for identifying a single piece data in a set.
    ConnectionData::ConstList for example) This is set automatically
    when needed. By default: -1.
     */
    int id;

    /*!
    \brief the name of the driver that should be used to create a connection

    Name (unique, not i18n'd) of driver that is used (or should be used) to
    create a connection. If you pass this ConnectionData object to
    KexiDB::Driver::createConnection() to create connection, the @a driverName member
    will be updated with a valid KexiDB driver name.
    In other situations the @a driverName member may be used to store information what
    driver should be used to perform connection, before we get an appropriate
    driver object from DriverManager.
    */
    QString driverName;

    /*!
    \brief Host name used for the remote connection.

    Can be empty if the connection is not remote. If it is empty "localhost" is used.
    */
    QString hostName;

    /*!
    \brief Server name of the server to be connected to

    @todo only used in Sybase driver, will be replaced by options after port to Predicate
    */
    QString serverName;

    /*!
    \brief Port used for the remote connection.

    The default is 0, what means we use don't change the database engine's default port.
    */
    unsigned short int port;

    /*!
    \brief True if local socket file should be used instead of TCP/IP port.

    Only meaningful for connections with localhost as server.
    True by default, so local communication can be optimized, and users can avoid problems
    with TCP/IP connections disabled by firewalls.

    If true, @a hostName and @a port will be ignored and @a localSocketFileName will be used.
    On MS Windows this option is often ignored and TCP/IP connection to the localhost is performed.
    */
    bool useLocalSocketFile;

    /*!
    \brief Name of local (named) socket file.

    For local connections only. If empty, it's driver will try to locate existing local socket
    file. Empty by default.
    */
    QString localSocketFileName;

    /*!
    \brief Password used for the connection.

    Can be empty string or null. If it is empty (equal to ""), empty password is passed
    to the driver. If it is empty, no password is passed to the driver.
    In this case, applications using KexiDB should ask for the password. */
    QString password;

    /*!
    \brief True if password should be saved to a file for the connection.

    False by default, in most cases can be set to true when nonempty
    password has been loaded from a file.
    For instance, this flag can be then shown for a user as a checkbox.
    */
    bool savePassword;

    /*!
    \brief Username used for the connection.

    Can be empty. */
    QString userName;

protected:
    ConnectionDataBase();

    /*!
    \brief The filename for file-based databases

    For file-based database engines like SQLite, \a fileName is used
    instead hostName and port
    */
    QString m_fileName;

    /*!
    \brief Absolute path to the database file

    Will be empty if database is not file-based
    */
    QString m_dbPath;

    /*!
    \brief  Filename of the database file

    Will be empty if database is not file-based
    */
    QString m_dbFileName;
    
    friend class ConnectionData;
};

//! Database specific connection data, e.g. host, port.
/*! Connection data, once configured, can be later stored for reuse.
*/
class CALLIGRADB_EXPORT ConnectionData : public QObject, public ConnectionDataBase
{
public:
    typedef QList<ConnectionData*> List;
    typedef QListIterator<ConnectionData*> ListIterator;

    ConnectionData();

    ConnectionData(const ConnectionData&);

    ~ConnectionData();

    ConnectionData& operator=(const ConnectionData& cd);

    /*!
    \brief Set the filename used by the connection

    For file-based database engines, like SQLite, you should use this
    function to set the file name of the database to use.
    \a fn can be either absolute or relative path to the file.
    */
    void setFileName(const QString& fn);

    /*!
    \brief Get the filename used by the connection

    For file-based database engines like SQLite, \a fileName is used
    instead hostName and port.
    @return An absolute path to the database file being used
    */
    QString fileName() const {
        return m_fileName;
    }

    /*!
    \brief The directory the database file is in

    \return file path (for file-based engines) but without a file name
    */
    QString dbPath() const {
        return m_dbPath;
    }

    /*!
    \brief The file name (without path) of the database file

    \return The file name (for file-based engines) but without a full path
    */
    QString dbFileName() const {
        return m_dbFileName;
    }

    /*!
     \brief  A user-friendly string for the server

     \return a user-friendly string like:
     - "myhost.org:12345" if a host and port is specified;
     - "localhost:12345" of only port is specified;
     - "user@myhost.org:12345" if also user is specified
     - "<file>" if file-based driver is assigned but no filename is assigned
     - "file: pathto/mydb.kexi" if file-based driver is assigned and
      filename is assigned

     User's name is added if \a addUser is true (the default).
    */
    QString serverInfoString(bool addUser = true) const;

    /*! @internal
     Format version used when saving the data to a shortcut file.
     This is set to 0 by default what means KexiDBShortcutFile_version should be used on saving.
     If KexiDBConnShortcutFile was used to create this KexiProjectData object,
     the version information is be retrieved from the file. */
    uint formatVersion;

protected:
    class Private;
    Private *priv;

    friend class Connection;
};

} //namespace KexiDB

#endif
