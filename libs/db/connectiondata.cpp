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

#include "connectiondata.h"
#include "drivermanager.h"

#include <QFileInfo>
#include <QDir>

#include <klocale.h>
#include <KDebug>

using namespace KexiDB;

namespace KexiDB
{
//! @internal
class ConnectionData::Private
{
public:
    Private() {
        dummy = false;
    }
    ~Private() {}
    bool dummy;
};
}

/*================================================================*/

ConnectionDataBase::ConnectionDataBase()
        : id(-1), port(0), useLocalSocketFile(true), savePassword(false)
{
}

/*================================================================*/

ConnectionData::ConnectionData()
        : QObject()
        , ConnectionDataBase()
        , formatVersion(0)
        , priv(new ConnectionData::Private())
{
}

ConnectionData::ConnectionData(const ConnectionData& cd)
        : QObject()
        , ConnectionDataBase()
        , priv(new ConnectionData::Private())
{
    static_cast<ConnectionData&>(*this) = static_cast<const ConnectionData&>(cd);//copy data members
}

ConnectionData::~ConnectionData()
{
    delete priv;
    priv = 0;
}

ConnectionData& ConnectionData::operator=(const ConnectionData & cd)
{
    if (this != &cd) {
        static_cast<ConnectionDataBase&>(*this) = static_cast<const ConnectionDataBase&>(cd);//copy data members
        *priv = *cd.priv;
    }
    return *this;
}

void ConnectionData::setFileName(const QString& fn)
{
    QFileInfo file(fn);
    if (!fn.isEmpty() && m_fileName != file.absoluteFilePath()) {
        m_fileName = QDir::convertSeparators(file.absoluteFilePath());
        m_dbPath = QDir::convertSeparators(file.absolutePath());
        m_dbFileName = file.fileName();
    }
}

QString ConnectionData::serverInfoString(bool addUser) const
{
    const QString& i18nFile = i18n("file");

    if (!m_dbFileName.isEmpty())
        return i18nFile + ": " + (m_dbPath.isEmpty() ? "" : m_dbPath
                                  + QDir::separator()) + m_dbFileName;

    DriverManager man;
    if (!driverName.isEmpty()) {
        Driver::Info info = man.driverInfo(driverName);
        if (!info.name.isEmpty() && info.fileBased)
            return QString("<") + i18nFile + ">";
    }

    return ((userName.isEmpty() || !addUser) ? QString("") : (userName + "@"))
           + (hostName.isEmpty() ? QString("localhost") : hostName)
           + (port != 0 ? (QString(":") + QString::number(port)) : QString());
}

