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

#include "simplecommandlineapp.h"

#include <QFileInfo>
#include <QTextStream>

#include <KCmdLineArgs>
#include <KDebug>
#include <KLocale>
#include <KComponentData>

#include "connectiondata.h"
#include "drivermanager.h"
#include "utils.h"

using namespace KexiDB;

//-----------------------------------------

//! @internal used for SimpleCommandLineApp
class SimpleCommandLineApp::Private
{
public:
    Private()
            : conn(0) {}
    ~Private() {
        if (conn) {
            conn->disconnect();
            delete(Connection*)conn;
        }
    }

    KexiDB::DriverManager manager;
    KComponentData componentData;
    ConnectionData connData;
    QPointer<Connection> conn;
};

//-----------------------------------------

SimpleCommandLineApp::SimpleCommandLineApp(
    int argc, char** argv, const KCmdLineOptions &options,
    const char *programName, const char *version,
    const char *shortDescription, KAboutData::LicenseKey licenseType,
    const char *copyrightStatement, const char *text,
    const char *homePageAddress, const char *bugsEmailAddress)
        : Object()
        , d(new Private())
{
    QFileInfo fi(argv[0]);
    QByteArray appName(fi.baseName().toLatin1());
    KCmdLineArgs::init(argc, argv,
                       new KAboutData(appName, 0, ki18n(programName),
                                      version, ki18n(shortDescription), licenseType, ki18n(copyrightStatement), ki18n(text),
                                      homePageAddress, bugsEmailAddress));

    d->componentData = KComponentData(appName);

    KCmdLineOptions allOptions;

    // add predefined options
    allOptions.add("drv", KLocalizedString(), KexiDB::defaultFileBasedDriverName().toUtf8());
    allOptions.add("driver <name>", ki18n("Database driver name"));
    allOptions.add("u");
    allOptions.add("user <name>", ki18n("Database user name"));
    allOptions.add("p");
    allOptions.add("password", ki18n("Prompt for password"));
    allOptions.add("h");
    allOptions.add("host <name>", ki18n("Host (server) name"));
    allOptions.add("port <number>", ki18n("Server's port number"));
    allOptions.add("s");
    allOptions.add("local-socket <filename>", ki18n("Server's local socket filename"));

    // add user options
    allOptions.add(options);

    KCmdLineArgs::addCmdLineOptions(allOptions);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    d->connData.driverName = args->getOption("driver");
    d->connData.userName = args->getOption("user");
    d->connData.hostName = args->getOption("host");
    d->connData.localSocketFileName = args->getOption("local-socket");
    d->connData.port = args->getOption("port").toInt();
    d->connData.useLocalSocketFile = args->isSet("local-socket");

    if (args->isSet("password")) {
        QString userAtHost = d->connData.userName;
        if (!d->connData.userName.isEmpty())
            userAtHost += '@';
        userAtHost += (d->connData.hostName.isEmpty() ? "localhost" : d->connData.hostName);
        QTextStream cout(stdout, QIODevice::WriteOnly);
        cout << i18n("Enter password for %1: ", userAtHost);
//! @todo make use of pty/tty here! (and care about portability)
        QTextStream cin(stdin, QIODevice::ReadOnly);
        cin >> d->connData.password;
        KexiDBDbg << d->connData.password;
    }
}

SimpleCommandLineApp::~SimpleCommandLineApp()
{
    closeDatabase();
    delete d;
}

bool SimpleCommandLineApp::openDatabase(const QString& databaseName)
{
    if (!d->conn) {
        if (d->manager.error()) {
            setError(&d->manager);
            return false;
        }

        //get the driver
        KexiDB::Driver *driver = d->manager.driver(d->connData.driverName);
        if (!driver || d->manager.error()) {
            setError(&d->manager);
            return false;
        }

        if (driver->isFileDriver())
            d->connData.setFileName(databaseName);

        d->conn = driver->createConnection(d->connData);
        if (!d->conn || driver->error()) {
            setError(driver);
            return false;
        }
    }
    if (d->conn->isConnected()) {
        // db already opened
        if (d->conn->isDatabaseUsed() && d->conn->currentDatabase() == databaseName) //the same: do nothing
            return true;
        if (!closeDatabase()) // differs: close the first
            return false;
    }
    if (!d->conn->connect()) {
        setError(d->conn);
        delete d->conn;
        d->conn = 0;
        return false;
    }

    if (!d->conn->useDatabase(databaseName)) {
        setError(d->conn);
        delete d->conn;
        d->conn = 0;
        return false;
    }
    return true;
}

bool SimpleCommandLineApp::closeDatabase()
{
    if (!d->conn)
        return true;
    if (!d->conn->disconnect()) {
        setError(d->conn);
        return false;
    }
    return true;
}

const KComponentData &SimpleCommandLineApp::componentData() const
{
    return d->componentData;
}

KexiDB::ConnectionData* SimpleCommandLineApp::connectionData() const
{
    return &d->connData;
}

KexiDB::Connection* SimpleCommandLineApp::connection() const
{
    return d->conn;
}
