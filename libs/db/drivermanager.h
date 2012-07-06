/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
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

#ifndef KEXIDB_DRIVER_MNGR_H
#define KEXIDB_DRIVER_MNGR_H

//#include <klibloader.h>
#include <kservice.h>

#include "driver.h"

namespace KexiDB
{

class DriverManagerInternal;
class Connection;

//! Database driver management, e.g. finding and loading drivers.
class CALLIGRADB_EXPORT DriverManager : public QObject, public KexiDB::Object
{
public:
    typedef QHash<QString, KService::Ptr> ServicesHash;

    DriverManager();
    virtual ~DriverManager();

    /*! Tries to load db driver with named name \a name.
      The name is case insensitive.
      \return db driver, or 0 if error (then error message is also set) */
    Driver* driver(const QString& name);

    /*! returns list of available drivers names.
      That drivers can be loaded by first use of driver() method. */
    const QStringList driverNames();

    /*! returns information list of available drivers.
      That drivers can be loaded by first use of driver() method. */
    const KexiDB::Driver::InfoHash driversInfo();

    /*! \return information about driver's named with \a name.
      The name is case insensitive.
      You can check if driver information is not found calling
      Info::name.isEmpty() (then error message is also set). */
    KexiDB::Driver::Info driverInfo(const QString &name);

    /*! \return service information about driver's named with \a name.
      The name is case insensitive.
      In most cases you can use driverInfo() instead. */
    KService::Ptr serviceInfo(const QString &name);

    /*! \return a hash structure of the services. Not necessary for everyday use. */
    const ServicesHash& services();

    /*! Looks up a drivers list by MIME type of database file.
     Only file-based database drivers are checked.
     The lookup is case insensitive.
     \return driver name or null string if no driver found.
    */
    QString lookupByMime(const QString &mimeType);

    //! server error is set if there is error at KService level (useful for debugging)
    virtual QString serverErrorMsg();
    virtual int serverResult();
    virtual QString serverResultName();

    /*! HTML information about possible problems encountered.
     It's displayed in 'details' section, if an error encountered.
     Currently it contains a list of incompatible db drivers.
     Used in KexiStartupHandler::detectDriverForFile(). */
    QString possibleProblemsInfoMsg() const;

protected:
    virtual void drv_clearServerResult();

private:
    DriverManagerInternal *d_int;
};

} //namespace KexiDB

#endif
