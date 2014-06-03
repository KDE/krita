/* This file is part of the KDE project
   Copyright (C) 2005 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_DBPROPERTIES_H
#define KEXIDB_DBPROPERTIES_H

#include "connection.h"

namespace KexiDB
{

//! @todo implement KConfigBase interface here?

//! A set of storable database properties.
/*! This is a convenience class that allows to store global dabatase properties without a need
 for creating and maintain custom table.
 DatabaseProperties object is accessible only using KexiDB::Connection::databaseProperties() method.
 */
class CALLIGRADB_EXPORT DatabaseProperties : public KexiDB::Object
{
public:
    /*! Sets \a value for property \a name. Optional caption can be also set.
     If there's no such property defined, it will be added. Existing value will be overwritten.
     Note that to execute this method, database must be opened in read-write mode.
     \return true on successful data. Connection */
    bool setValue(const QString& name, const QVariant& value);

    /*! Sets \a caption for for property \a name.
     Usually it shouldn't be translated: trnaslation can be performed before displaying. */
    bool setCaption(const QString& name, const QString& caption);

    //! \return property value for \a propeName available for this driver.
    //! If there's no such property defined for driver, Null QVariant value is returned.
    QVariant value(const QString& name);

    //! \return translated property caption for \a name.
    //! If there's no such property defined for driver, empty string value is returned.
    QString caption(const QString& name);

    //! \return a list of available property names.
    QStringList names();

protected:
    DatabaseProperties(Connection *conn);
    ~DatabaseProperties();

    QPointer<Connection> m_conn;
    friend class Connection;
};
}

#endif
