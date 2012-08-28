/* This file is part of the KDE project
   Copyright (C) 2003 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_DRIVER_MNGR_P_H
#define KEXIDB_DRIVER_MNGR_P_H

#include "object.h"

#include <QObject>

namespace KexiDB
{

/*! Internal class of driver manager.
*/
class CALLIGRADB_EXPORT DriverManagerInternal : public QObject, public KexiDB::Object
{
    Q_OBJECT
public:
    ~DriverManagerInternal();

    /*! Tries to load db driver \a name.
      \return db driver, or 0 if error (then error message is also set) */
    KexiDB::Driver* driver(const QString& name);

    KexiDB::Driver::Info driverInfo(const QString &name);

    static DriverManagerInternal *self();

    /*! increments the refcount for the manager */
    void incRefCount();

    /*! decrements the refcount for the manager
      if the refcount reaches a value less than 1 the manager is freed */
    void decRefCount();

    /*! Called from Driver dtor (because sometimes KLibrary (used by Driver)
     is destroyed before DriverManagerInternal) */
    void aboutDelete(Driver* drv);

protected slots:
    /*! Used to destroy all drivers on QApplication quit, so even if there are
     DriverManager's static instances that are destroyed on program
     "static destruction", drivers are not kept after QApplication death.
    */
    void slotAppQuits();

protected:
    /*! Used by self() */
    DriverManagerInternal();

    bool lookupDrivers();

    static KexiDB::DriverManagerInternal* s_self;

    DriverManager::ServicesHash m_services; //! services map
    DriverManager::ServicesHash m_services_lcase; //! as above but service names in lowercase
    DriverManager::ServicesHash m_services_by_mimetype;
    Driver::InfoHash m_driversInfo; //! used to store drivers information
    QHash<QString, KexiDB::Driver*> m_drivers;
    ulong m_refCount;

    QString m_serverErrMsg;
    int m_serverResultNum;
    QString m_serverResultName;
    //! result names for KParts::ComponentFactory::ComponentLoadingError
    //QHash<int, QString> m_componentLoadingErrors;

    QStringList possibleProblems;

    bool lookupDriversNeeded : 1;

    friend class DriverManager;
};
}

#endif

