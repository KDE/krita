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

#include "sqliteconnection.h"
#include "sqliteconnection_p.h"
#include "sqlitecursor.h"
#include "sqlitepreparedstatement.h"

#include <sqlite3.h>

#include <db/driver.h>
#include <db/cursor.h>
#include <db/error.h>

#include <QFile>
#include <QDir>
#include <QRegExp>

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

//remove debug
#undef KexiDBDrvDbg
#define KexiDBDrvDbg if (0) kDebug()

#if defined(Q_OS_WIN)
#define SHARED_LIB_EXTENSION ".dll"
#elif defined(Q_OS_MAC)
#define SHARED_LIB_EXTENSION ".dylib"
#else
#define SHARED_LIB_EXTENSION ".so"
#endif

using namespace KexiDB;

SQLiteConnectionInternal::SQLiteConnectionInternal(Connection *connection)
        : ConnectionInternal(connection)
        , data(0)
        , data_owned(true)
        , errmsg_p(0)
        , res(SQLITE_OK)
        , result_name(0)
        , m_extensionsLoadingEnabled(false)
{
}

SQLiteConnectionInternal::~SQLiteConnectionInternal()
{
    if (data_owned && data) {
        sqlite3_close(data);
        data = 0;
    }
}

void SQLiteConnectionInternal::storeResult()
{
    if (errmsg_p) {
        errmsg = errmsg_p;
        sqlite3_free(errmsg_p);
        errmsg_p = 0;
    }
    errmsg = (data && res != SQLITE_OK) ? sqlite3_errmsg(data) : 0;
}

bool SQLiteConnectionInternal::extensionsLoadingEnabled() const
{
    return m_extensionsLoadingEnabled;
}

void SQLiteConnectionInternal::setExtensionsLoadingEnabled(bool set)
{
    if (set == m_extensionsLoadingEnabled)
        return;
    sqlite3_enable_load_extension(data, set);
    m_extensionsLoadingEnabled = set;
}

/*! Used by driver */
SQLiteConnection::SQLiteConnection(Driver *driver, ConnectionData &conn_data)
        : Connection(driver, conn_data)
        , d(new SQLiteConnectionInternal(this))
{
}

SQLiteConnection::~SQLiteConnection()
{
    KexiDBDrvDbg << "SQLiteConnection::~SQLiteConnection()";
    //disconnect if was connected
// disconnect();
    destroy();
    delete d;
    KexiDBDrvDbg << "SQLiteConnection::~SQLiteConnection() ok";
}

bool SQLiteConnection::drv_connect(KexiDB::ServerVersionInfo& version)
{
    KexiDBDrvDbg << "SQLiteConnection::connect()";
    version.string = QString(SQLITE_VERSION); //defined in sqlite3.h
    QRegExp re("(\\d+)\\.(\\d+)\\.(\\d+)");
    if (re.exactMatch(version.string)) {
        version.major = re.cap(1).toUInt();
        version.minor = re.cap(2).toUInt();
        version.release = re.cap(3).toUInt();
    }
    return true;
}

bool SQLiteConnection::drv_disconnect()
{
    KexiDBDrvDbg << "SQLiteConnection::disconnect()";
    return true;
}

bool SQLiteConnection::drv_getDatabasesList(QStringList &list)
{
    //this is one-db-per-file database
    list.append(data()->fileName());   //more consistent than dbFileName() ?
    return true;
}

bool SQLiteConnection::drv_containsTable(const QString &tableName)
{
    bool success=false;
    return resultExists(QString("select name from sqlite_master where type='table' and name LIKE %1")
                        .arg(driver()->escapeString(tableName)), success) && success;
}

bool SQLiteConnection::drv_getTablesList(QStringList &list)
{
    KexiDB::Cursor *cursor;
    m_sql = "select lower(name) from sqlite_master where type='table'";
    if (!(cursor = executeQuery(m_sql))) {
        KexiDBWarn << "Connection::drv_getTablesList(): !executeQuery()";
        return false;
    }
    list.clear();
    cursor->moveFirst();
    while (!cursor->eof() && !cursor->error()) {
        list += cursor->value(0).toString();
        cursor->moveNext();
    }
    if (cursor->error()) {
        deleteCursor(cursor);
        return false;
    }
    return deleteCursor(cursor);
}

bool SQLiteConnection::drv_createDatabase(const QString &dbName)
{
    Q_UNUSED(dbName);
    return drv_useDatabaseInternal(0, 0, true/*create if missing*/);
}

bool SQLiteConnection::drv_useDatabase(const QString &dbName, bool *cancelled,
                                       MessageHandler* msgHandler)
{
    Q_UNUSED(dbName);
    return drv_useDatabaseInternal(cancelled, msgHandler, false/*do not create if missing*/);
}

bool SQLiteConnection::drv_useDatabaseInternal(bool *cancelled,
                                               MessageHandler* msgHandler, bool createIfMissing)
{
//! @todo add option (command line or in kexirc?)
//! @todo   int exclusiveFlag = Connection::isReadOnly() ? SQLITE_OPEN_READONLY : SQLITE_OPEN_WRITE_LOCKED; // <-- shared read + (if !r/o): exclusive write
    int openFlags = 0;
    if (isReadOnly()) {
        openFlags |= SQLITE_OPEN_READONLY;
    }
    else {
        openFlags |= SQLITE_OPEN_READWRITE;
        if (createIfMissing) {
            openFlags |= SQLITE_OPEN_CREATE;
        }
    }

//! @todo add option
//    int allowReadonly = 1;
//    const bool wasReadOnly = Connection::isReadOnly();

    d->res = sqlite3_open_v2(
                 //QFile::encodeName( data()->fileName() ),
                 data()->fileName().toUtf8().constData(), /* unicode expected since SQLite 3.1 */
                 &d->data,
                 openFlags, /*exclusiveFlag,
                 allowReadonly *//* If 1 and locking fails, try opening in read-only mode */
                 0
             );
    d->storeResult();

    if (d->res == SQLITE_OK) {
        // Set the secure-delete on, so SQLite overwrites deleted content with zeros.
        // The default setting is determined by the SQLITE_SECURE_DELETE compile-time option but we overwrite it here.
        // Works with 3.6.23. Earlier versions just ignore this pragma.
        // See http://www.sqlite.org/pragma.html#pragma_secure_delete
//! @todo add connection flags to the driver and global setting to control the "secure delete" pragma
        if (!drv_executeSQL("PRAGMA secure_delete = on")) {
            drv_closeDatabaseSilently();
            return false;
        }
        // Load ICU extension for unicode collations
        QString icuExtensionFilename(
            KStandardDirs::locate("module", QLatin1String("kexidb_sqlite3_icu" SHARED_LIB_EXTENSION)));
        if (!loadExtension(icuExtensionFilename)) {
            drv_closeDatabaseSilently();
            return false;
        }
        // load ROOT collation for use as default collation
        if (!drv_executeSQL("SELECT icu_load_collation('', '')")) {
            drv_closeDatabaseSilently();
            return false;
        }
    }

//! @todo check exclusive status
    Q_UNUSED(cancelled);
    Q_UNUSED(msgHandler);
#if 0
    if (d->res == SQLITE_OK && cancelled && !wasReadOnly && allowReadonly && isReadOnly()) {
        //opened as read only, ask
        if (KMessageBox::Continue !=
                askQuestion(
                    i18n("Do you want to open file \"%1\" as read-only?",
                         QDir::convertSeparators(data()->fileName()))
                    + "\n\n"
                    + i18n("The file is probably already open on this or another computer.") + " "
                    + i18n("Could not gain exclusive access for writing the file."),
                    KMessageBox::WarningContinueCancel, KMessageBox::Continue,
                    KGuiItem(i18n("Open As Read-Only"), "document-open"), KStandardGuiItem::cancel(),
                    "askBeforeOpeningFileReadOnly", KMessageBox::Notify, msgHandler)) {
            clearError();
            if (!drv_closeDatabase())
                return false;
            *cancelled = true;
            return false;
        }
    }
//! @todo
/*
    if (d->res == SQLITE_CANTOPEN_WITH_LOCKED_READWRITE) {
        setError(ERR_ACCESS_RIGHTS,
                 i18n("The file is probably already open on this or another computer.") + "\n\n"
                 + i18n("Could not gain exclusive access for reading and writing the file.") + " "
                 + i18n("Check the file's permissions and whether it is already opened and locked by another application."));
    } else if (d->res == SQLITE_CANTOPEN_WITH_LOCKED_WRITE) {
        setError(ERR_ACCESS_RIGHTS,
                 i18n("The file is probably already open on this or another computer.") + "\n\n"
                 + i18n("Could not gain exclusive access for writing the file.") + " "
                 + i18n("Check the file's permissions and whether it is already opened and locked by another application."));
    }
    */
#endif
    return d->res == SQLITE_OK;
}

bool SQLiteConnection::drv_closeDatabase()
{
    if (!d->data)
        return false;

    const int res = sqlite3_close(d->data);
    if (SQLITE_OK == res) {
        d->data = 0;
        return true;
    }
    if (SQLITE_BUSY == res) {
#if 0 //this is ANNOYING, needs fixing (by closing cursors or waiting)
        setError(ERR_CLOSE_FAILED, i18n("Could not close busy database."));
#else
        return true;
#endif
    }
    return false;
}

void SQLiteConnection::drv_closeDatabaseSilently()
{
    const QString errmsg(d->errmsg); // save
    const int res = d->res; // save
    drv_closeDatabase();
    d->errmsg = errmsg;
    d->res = res;
}

bool SQLiteConnection::drv_dropDatabase(const QString &dbName)
{
    Q_UNUSED(dbName); // Each database is one single SQLite file.
    const QString filename = data()->fileName();
    if (QFile(filename).exists() && !QDir().remove(filename)) {
        setError(ERR_ACCESS_RIGHTS, i18n("Could not remove file \"%1\".",
                                         QDir::convertSeparators(filename)) + " "
                 + i18n("Check the file's permissions and whether it is already opened and locked by another application."));
        return false;
    }
    return true;
}

//CursorData* SQLiteConnection::drv_createCursor( const QString& statement )
Cursor* SQLiteConnection::prepareQuery(const QString& statement, uint cursor_options)
{
    return new SQLiteCursor(this, statement, cursor_options);
}

Cursor* SQLiteConnection::prepareQuery(QuerySchema& query, uint cursor_options)
{
    return new SQLiteCursor(this, query, cursor_options);
}

bool SQLiteConnection::drv_executeSQL(const QString& statement)
{
// KexiDBDrvDbg << "SQLiteConnection::drv_executeSQL(" << statement << ")";
// QCString st(statement.length()*2);
// st = escapeString( statement.local8Bit() ); //?
#ifdef SQLITE_UTF8
    d->temp_st = statement.toUtf8();
#else
    d->temp_st = statement.toLocal8Bit(); //latin1 only
#endif

#ifdef KEXI_DEBUG_GUI
    KexiDB::addKexiDBDebug(QString("ExecuteSQL (SQLite): ") + statement);
#endif

    d->res = sqlite3_exec(
                 d->data,
                 (const char*)d->temp_st,
                 0/*callback*/,
                 0,
                 &d->errmsg_p);
    d->storeResult();
#ifdef KEXI_DEBUG_GUI
    KexiDB::addKexiDBDebug(d->res == SQLITE_OK ? "  Success" : "  Failure");
#endif
    return d->res == SQLITE_OK;
}

quint64 SQLiteConnection::drv_lastInsertRowID()
{
    return (quint64)sqlite3_last_insert_rowid(d->data);
}

int SQLiteConnection::serverResult()
{
    return d->res == 0 ? Connection::serverResult() : d->res;
}

static const char* serverResultNames[] = {
    "SQLITE_OK", // 0
    "SQLITE_ERROR",
    "SQLITE_INTERNAL",
    "SQLITE_PERM",
    "SQLITE_ABORT",
    "SQLITE_BUSY",
    "SQLITE_LOCKED",
    "SQLITE_NOMEM",
    "SQLITE_READONLY",
    "SQLITE_INTERRUPT",
    "SQLITE_IOERR",
    "SQLITE_CORRUPT",
    "SQLITE_NOTFOUND",
    "SQLITE_FULL",
    "SQLITE_CANTOPEN",
    "SQLITE_PROTOCOL",
    "SQLITE_EMPTY",
    "SQLITE_SCHEMA",
    "SQLITE_TOOBIG",
    "SQLITE_CONSTRAINT",
    "SQLITE_MISMATCH",
    "SQLITE_MISUSE",
    "SQLITE_NOLFS",
    "SQLITE_AUTH",
    "SQLITE_FORMAT",
    "SQLITE_RANGE",
    "SQLITE_NOTADB", // 26
};

QString SQLiteConnection::serverResultName()
{
    if (d->res >= 0 && d->res <= SQLITE_NOTADB)
        return QString::fromLatin1(serverResultNames[d->res]);
    else if (d->res == SQLITE_ROW)
        return QLatin1String("SQLITE_ROW");
    else if (d->res == SQLITE_DONE)
        return QLatin1String("SQLITE_DONE");
    return QString();
}

void SQLiteConnection::drv_clearServerResult()
{
    if (!d)
        return;
    d->res = SQLITE_OK;
}

QString SQLiteConnection::serverErrorMsg()
{
    return d->errmsg.isEmpty() ? Connection::serverErrorMsg() : d->errmsg;
}

PreparedStatement::Ptr SQLiteConnection::prepareStatement(PreparedStatement::StatementType type,
        FieldList& fields)
{
    return KSharedPtr<PreparedStatement>(new SQLitePreparedStatement(type, *d, fields));
}

bool SQLiteConnection::isReadOnly() const
{
    //! @todo
#if 0
    return (d->data ? sqlite3_is_readonly(d->data) : false)
           || Connection::isReadOnly();
#else
    return Connection::isReadOnly();
#endif
}

bool SQLiteConnection::loadExtension(const QString& path)
{
    bool tempEnable = false;
    if (!d->extensionsLoadingEnabled()) {
        tempEnable = true;
        d->setExtensionsLoadingEnabled(true);
    }
    d->res = sqlite3_load_extension(d->data, path.toUtf8().constData(), 0, &d->errmsg_p);
    d->storeResult();
    bool ok = SQLITE_OK == d->res;
    if (tempEnable) {
        d->setExtensionsLoadingEnabled(false);
    }
    if (!ok) {
        kWarning() << "Could not load SQLite extension" << path << ":" << d->errmsg_p;
    }
    return ok;
}

#include "sqliteconnection.moc"
