/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#include "driver.h"
#include "driver_p.h"
#include "drivermanager.h"
#include "drivermanager_p.h"
#include "error.h"
#include "drivermanager.h"
#include "connection.h"
#include "connectiondata.h"
#include "admin.h"
#include "utils.h"

#include <klocale.h>
#include <kdebug.h>

#include <assert.h>

using namespace KexiDB;

/*! @internal Used in Driver::defaultSQLTypeName(int)
 when we do not have Driver instance yet, or when we cannot get one */
static const char* KexiDB_defaultSQLTypeNames[] = {
    "InvalidType",
    "Byte",
    "ShortInteger",
    "Integer",
    "BigInteger",
    "Boolean",
    "Date",
    "DateTime",
    "Time",
    "Float",
    "Double",
    "Text",
    "LongText",
    "BLOB"
};

//---------------------------------------------

DriverBehaviour::DriverBehaviour()
        : UNSIGNED_TYPE_KEYWORD("UNSIGNED")
        , AUTO_INCREMENT_FIELD_OPTION("AUTO_INCREMENT")
        , AUTO_INCREMENT_PK_FIELD_OPTION("AUTO_INCREMENT PRIMARY KEY")
        , SPECIAL_AUTO_INCREMENT_DEF(false)
        , AUTO_INCREMENT_REQUIRES_PK(false)
        , ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE(false)
        , QUOTATION_MARKS_FOR_IDENTIFIER('"')
        , USING_DATABASE_REQUIRED_TO_CONNECT(true)
        , _1ST_ROW_READ_AHEAD_REQUIRED_TO_KNOW_IF_THE_RESULT_IS_EMPTY(false)
        , SELECT_1_SUBQUERY_SUPPORTED(false)
        , TEXT_TYPE_MAX_LENGTH(0)
{
}

//---------------------------------------------

Driver::Info::Info()
        : fileBased(false)
        , allowImportingTo(true)
{
}

//---------------------------------------------

Driver::Driver(QObject *parent, const QVariantList &args)
        : QObject(parent)
        , Object()
        , beh(new DriverBehaviour())
        , d(new DriverPrivate())
{
    Q_UNUSED(args)
    d->typeNames.resize(Field::LastType + 1);
}

Driver::~Driver()
{
    DriverManagerInternal::self()->aboutDelete(this);
// KexiDBDbg << "Driver::~Driver()";
    // make a copy because d->connections will be touched by ~Connection
    QSet<Connection*> connections(d->connections);
    qDeleteAll(connections);
    d->connections.clear();
    delete beh;
    delete d;
// KexiDBDbg << "Driver::~Driver() ok";
}

bool Driver::isValid()
{
    clearError();
/* moved to DriverManagerInternal::driver():
    if (KexiDB::version().major != version().major
            || KexiDB::version().minor != version().minor) {
        setError(ERR_INCOMPAT_DRIVER_VERSION,
                 i18n(
                     "Incompatible database driver's \"%1\" version: found version %2, expected version %3.",
                     objectName(),
                     QString("%1.%2").arg(version().major).arg(version().minor),
                     QString("%1.%2").arg(KexiDB::version().major).arg(KexiDB::version().minor)));
        return false;
    }*/

    QString inv_impl = i18n("Invalid database driver's \"%1\" implementation:\n", name());
    KLocalizedString not_init = ki18n("Value of \"%1\" is not initialized for the driver.");
    if (beh->ROW_ID_FIELD_NAME.isEmpty()) {
        setError(ERR_INVALID_DRIVER_IMPL,
                 inv_impl + not_init.subs("DriverBehaviour::ROW_ID_FIELD_NAME").toString());
        return false;
    }

    return true;
}

const QSet<Connection*> Driver::connections() const
{
    return d->connections;
}

QString Driver::fileDBDriverMimeType() const
{
    return d->fileDBDriverMimeType;
}

const KService* Driver::service() const
{
    return d->service;
}

bool Driver::isFileDriver() const
{
    return d->isFileDriver;
}

int Driver::features() const
{
    return d->features;
}

bool Driver::transactionsSupported() const
{
    return d->features & (SingleTransactions | MultipleTransactions);
}

AdminTools& Driver::adminTools() const
{
    if (!d->adminTools)
        d->adminTools = drv_createAdminTools();
    return *d->adminTools;
}

AdminTools* Driver::drv_createAdminTools() const
{
    return new AdminTools(); //empty impl.
}

QString Driver::sqlTypeName(int id_t, int /*p*/) const
{
    if (id_t > Field::InvalidType && id_t <= Field::LastType)
        return d->typeNames[(id_t>0 && id_t<=Field::LastType) ? id_t : Field::InvalidType /*sanity*/];

    return d->typeNames[Field::InvalidType];
}

Connection *Driver::createConnection(ConnectionData &conn_data, int options)
{
    clearError();
    if (!isValid())
        return 0;
    if (d->isFileDriver) {
        if (conn_data.fileName().isEmpty()) {
            setError(ERR_MISSING_DB_LOCATION,
                     i18n("File name expected for file-based database driver."));
            return 0;
        }
    }
// Connection *conn = new Connection( this, conn_data );
    Connection *conn = drv_createConnection(conn_data);

    conn->setReadOnly(options & ReadOnlyConnection);

    conn_data.driverName = name();
    d->connections.insert(conn);
    return conn;
}

Connection* Driver::removeConnection(Connection *conn)
{
    clearError();
    if (d->connections.remove(conn))
        return conn;
    return 0;
}

QString Driver::defaultSQLTypeName(int id_t)
{
    if (id_t < 0 || id_t > (Field::LastType + 1))
        return QString::fromLatin1("Null");
    return QString::fromLatin1(KexiDB_defaultSQLTypeNames[id_t]);
}

bool Driver::isSystemObjectName(const QString& n) const
{
    return Driver::isKexiDBSystemObjectName(n);
}

bool Driver::isKexiDBSystemObjectName(const QString& n)
{
    if (!n.startsWith(QLatin1String("kexi__"), Qt::CaseInsensitive))
        return false;
    return Connection::kexiDBSystemTableNames().contains(n, Qt::CaseInsensitive);
}

bool Driver::isSystemFieldName(const QString& n) const
{
    if (!beh->ROW_ID_FIELD_NAME.isEmpty()
        && 0 == n.compare(beh->ROW_ID_FIELD_NAME, Qt::CaseInsensitive))
    {
        return true;
    }
    return drv_isSystemFieldName(n);
}

static QString valueToSQLInternal(const KexiDB::Driver *driver, uint ftype, const QVariant& v)
{
    if (v.isNull())
        return "NULL";
    switch (ftype) {
    case Field::Text:
    case Field::LongText: {
        QString s = v.toString();
        return driver ? driver->escapeString(s) : KexiDB::escapeString(s); //QString("'")+s.replace( '"', "\\\"" ) + "'";
    }
    case Field::Byte:
    case Field::ShortInteger:
    case Field::Integer:
    case Field::BigInteger:
        return v.toString();
    case Field::Float:
    case Field::Double: {
        if (v.type() == QVariant::String) {
            //workaround for values stored as string that should be casted to floating-point
            QString s(v.toString());
            return s.replace(',', ".");
        }
        return v.toString();
    }
//TODO: here special encoding method needed
    case Field::Boolean:
        return QString::number(v.toInt() ? 1 : 0); //0 or 1
    case Field::Time:
        return QString("\'") + v.toTime().toString(Qt::ISODate) + "\'";
    case Field::Date:
        return QString("\'") + v.toDate().toString(Qt::ISODate) + "\'";
    case Field::DateTime:
        return driver ? driver->dateTimeToSQL(v.toDateTime())
                      : KexiDB::dateTimeToSQL(v.toDateTime());
    case Field::BLOB: {
        if (v.toByteArray().isEmpty())
            return QString::fromLatin1("NULL");
        if (v.type() == QVariant::String) {
            return driver ? driver->escapeBLOB(v.toString().toUtf8())
                          : KexiDB::escapeBLOB(v.toString().toUtf8(), KexiDB::BLOBEscape0xHex);
        }
        return driver ? driver->escapeBLOB(v.toByteArray())
                      : KexiDB::escapeBLOB(v.toByteArray(), KexiDB::BLOBEscape0xHex);
    }
    case Field::InvalidType:
        return "!INVALIDTYPE!";
    default:
        KexiDBDbg << "Driver::valueToSQL(): UNKNOWN!";
        return QString();
    }
    return QString();
}

QString Driver::valueToSQL(uint ftype, const QVariant& v) const
{
    return valueToSQLInternal(this, ftype, v);
}

QVariant Driver::propertyValue(const QByteArray& propName) const
{
    return d->properties.value(propName.toLower());
}

QString Driver::propertyCaption(const QByteArray& propName) const
{
    return d->propertyCaptions.value(propName.toLower());
}

QList<QByteArray> Driver::propertyNames() const
{
    QList<QByteArray> names(d->properties.keys());
    qSort(names);
    return names;
}

QString Driver::escapeIdentifier(const QString& str, int options) const
{
    const QByteArray cstr(str.toLatin1());
    return QString(escapeIdentifier(cstr, options));
}

QByteArray Driver::escapeIdentifier(const QByteArray& str, int options) const
{
    if (options & EscapeKexi) {
        return KexiDB::escapeIdentifier(str, options);
    }

    bool needOuterQuotes = false;

// Need to use quotes if ...
// ... we have been told to, or ...
    if (options & EscapeAlways)
        needOuterQuotes = true;

// ... or if the driver does not have a list of keywords,
    else if (d->driverSpecificSQLKeywords.isEmpty())
        needOuterQuotes = true;

// ... or if it's a keyword in Kexi's SQL dialect,
    else if (KexiDB::isKexiSQLKeyword(str))
        needOuterQuotes = true;

// ... or if it's a keyword in the backends SQL dialect,
    else if ((options & EscapeDriver) && d->driverSpecificSQLKeywords.contains(str.toUpper()))
        needOuterQuotes = true;

// ... or if the identifier has a space in it...
    else if (str.contains(' '))
        needOuterQuotes = true;

    if (needOuterQuotes) {
        const char quote = beh->QUOTATION_MARKS_FOR_IDENTIFIER.toLatin1();
        return quote + drv_escapeIdentifier(str) + quote;
    } else {
        return drv_escapeIdentifier(str);
    }
}

void Driver::initDriverSpecificKeywords(const char** keywords)
{
    d->driverSpecificSQLKeywords.setStrings(keywords);
}

bool Driver::isDriverSpecificKeyword(const QByteArray& word) const
{
    return d->driverSpecificSQLKeywords.contains(word);
}

//---------------

K_GLOBAL_STATIC_WITH_ARGS(KexiDB::StaticSetOfStrings, KexiDB_kexiSQLKeywords, (DriverPrivate::kexiSQLKeywords))

CALLIGRADB_EXPORT bool KexiDB::isKexiSQLKeyword(const QByteArray& word)
{
    return KexiDB_kexiSQLKeywords->contains(word);
}

CALLIGRADB_EXPORT QString KexiDB::escapeString(const QString& str)
{
    return QLatin1String("'") + QString(str).replace('\'', "''") + QLatin1String("'");
}

CALLIGRADB_EXPORT QString KexiDB::escapeIdentifier(const QString& str, int options)
{
    const QByteArray cstr(str.toLatin1());
    return QString(escapeIdentifier(cstr, options));
}

CALLIGRADB_EXPORT QByteArray KexiDB::escapeIdentifier(const QByteArray& str, int options)
{
    if ((options & KexiDB::Driver::EscapeDriver)) {
        kWarning() << "Driver escaping not supported by KexiDB::escapeIdentifier(), will fallback to Kexi escaping.";
    }

    bool needOuterQuotes = false;
// Need to use quotes if ...
// ... we have been told to, or ...
    if (options & KexiDB::Driver::EscapeAlways)
        needOuterQuotes = true;

// ... or if it's a keyword in Kexi's SQL dialect,
    else if (KexiDB::isKexiSQLKeyword(str))
        needOuterQuotes = true;

// ... or if the identifier has a space in it...
    else if (str.contains(' '))
        needOuterQuotes = true;

    if (needOuterQuotes) {
        const char quote = '"';
        return quote + QByteArray(str).replace(quote, "\"\"") + quote;
    }
    return str;
}

CALLIGRADB_EXPORT QString KexiDB::valueToSQL(uint ftype, const QVariant& v)
{
    return valueToSQLInternal(0, ftype, v);
}

#include "driver.moc"
