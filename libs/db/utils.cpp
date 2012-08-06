/* This file is part of the KDE project
   Copyright (C) 2004-2012 Jarosław Staniek <staniek@kde.org>

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

#include "utils.h"
#include "cursor.h"
#include "drivermanager.h"
#include "lookupfieldschema.h"
#include "calligradb_global.h"
#include <KoIcon.h>

#include <QMap>
#include <QHash>
#include <QThread>
#include <QDomDocument>
#include <QBuffer>
#include <QPixmap>
#include <QMutex>
#include <QSet>
#include <QProgressBar>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <KMimeType>

#include <memory>

#include "utils_p.h"

using namespace KexiDB;

//! Cache
struct TypeCache {
    TypeCache() {
        for (uint t = 0; t <= Field::LastType; t++) {
            const Field::TypeGroup tg = Field::typeGroup(t);
            TypeGroupList list;
            QStringList name_list, str_list;
            if (tlist.contains(tg)) {
                list = tlist.value(tg);
                name_list = nlist.value(tg);
                str_list = slist.value(tg);
            }
            list += t;
            name_list += KexiDB::Field::typeName(t);
            str_list += KexiDB::Field::typeString(t);
            tlist[ tg ] = list;
            nlist[ tg ] = name_list;
            slist[ tg ] = str_list;
        }

        def_tlist[ Field::InvalidGroup ] = Field::InvalidType;
        def_tlist[ Field::TextGroup ] = Field::Text;
        def_tlist[ Field::IntegerGroup ] = Field::Integer;
        def_tlist[ Field::FloatGroup ] = Field::Double;
        def_tlist[ Field::BooleanGroup ] = Field::Boolean;
        def_tlist[ Field::DateTimeGroup ] = Field::Date;
        def_tlist[ Field::BLOBGroup ] = Field::BLOB;
    }

    QHash< Field::TypeGroup, TypeGroupList > tlist;
    QHash< Field::TypeGroup, QStringList > nlist;
    QHash< Field::TypeGroup, QStringList > slist;
    QHash< Field::TypeGroup, Field::Type > def_tlist;
};

K_GLOBAL_STATIC(TypeCache, KexiDB_typeCache)

const TypeGroupList KexiDB::typesForGroup(KexiDB::Field::TypeGroup typeGroup)
{
    return KexiDB_typeCache->tlist.value(typeGroup);
}

QStringList KexiDB::typeNamesForGroup(KexiDB::Field::TypeGroup typeGroup)
{
    return KexiDB_typeCache->nlist.value(typeGroup);
}

QStringList KexiDB::typeStringsForGroup(KexiDB::Field::TypeGroup typeGroup)
{
    return KexiDB_typeCache->slist.value(typeGroup);
}

KexiDB::Field::Type KexiDB::defaultTypeForGroup(KexiDB::Field::TypeGroup typeGroup)
{
    return (typeGroup <= Field::LastTypeGroup) ? KexiDB_typeCache->def_tlist.value(typeGroup) : Field::InvalidType;
}

void KexiDB::getHTMLErrorMesage(Object* obj, QString& msg, QString &details)
{
    Connection *conn = 0;
    if (!obj || !obj->error()) {
        if (dynamic_cast<Cursor*>(obj)) {
            conn = dynamic_cast<Cursor*>(obj)->connection();
            obj = conn;
        } else {
            return;
        }
    }
// if (dynamic_cast<Connection*>(obj)) {
    // conn = dynamic_cast<Connection*>(obj);
    //}
    if (!obj || !obj->error())
        return;
    //lower level message is added to the details, if there is alread message specified
    if (!obj->msgTitle().isEmpty())
        msg += "<p>" + obj->msgTitle();

    if (msg.isEmpty())
        msg = "<p>" + obj->errorMsg();
    else
        details += "<p>" + obj->errorMsg();

    if (!obj->serverErrorMsg().isEmpty())
        details += "<p><b>" + i18n("Message from server:") + "</b> " + obj->serverErrorMsg();
    if (!obj->recentSQLString().isEmpty())
        details += "<p><b>" + i18n("SQL statement:") + QString("</b> <tt>%1</tt>").arg(obj->recentSQLString());
    int serverResult;
    QString serverResultName;
    if (obj->serverResult() != 0) {
        serverResult = obj->serverResult();
        serverResultName = obj->serverResultName();
    } else {
        serverResult = obj->previousServerResult();
        serverResultName = obj->previousServerResultName();
    }
    if (   !details.isEmpty()
        && (   !obj->serverErrorMsg().isEmpty()
            || !obj->recentSQLString().isEmpty()
            || !serverResultName.isEmpty()
            || serverResult != 0)
           )
    {
        details += (QString("<p><b>") + i18n("Server result:") + "</b> " + QString::number(serverResult));
        if (!serverResultName.isEmpty()) {
            details += QString(" (%1)").arg(serverResultName);
        }
    }
    else {
        if (!serverResultName.isEmpty()) {
            details += (QString("<p><b>") + i18n("Server result:") + "</b> " + serverResultName);
        }
    }

    if (!details.isEmpty() && !details.startsWith("<qt>")) {
        if (details.startsWith("<p>"))
            details = QString::fromLatin1("<qt>") + details;
        else
            details = QString::fromLatin1("<p>") + details;
    }
}

void KexiDB::getHTMLErrorMesage(Object* obj, QString& msg)
{
    getHTMLErrorMesage(obj, msg, msg);
}

void KexiDB::getHTMLErrorMesage(Object* obj, ResultInfo *result)
{
    getHTMLErrorMesage(obj, result->msg, result->desc);
}

int KexiDB::idForObjectName(Connection &conn, const QString& objName, int objType)
{
    RecordData data;
    if (true != conn.querySingleRecord(
                QString::fromLatin1("select o_id from kexi__objects where lower(o_name)='%1' and o_type=%2")
                .arg(objName.toLower()).arg(objType), data))
        return 0;
    bool ok;
    int id = data[0].toInt(&ok);
    return ok ? id : 0;
}

//-----------------------------------------

TableOrQuerySchema::TableOrQuerySchema(Connection *conn, const QByteArray& name)
        : m_name(name)
{
    m_table = conn->tableSchema(QString(name));
    m_query = m_table ? 0 : conn->querySchema(QString(name));
    if (!m_table && !m_query)
        KexiDBWarn << "TableOrQuery(FieldList &tableOrQuery) : "
        " tableOrQuery is neither table nor query!";
}


TableOrQuerySchema::TableOrQuerySchema(Connection *conn, const QByteArray& name, bool table)
        : m_name(name)
        , m_table(table ? conn->tableSchema(QString(name)) : 0)
        , m_query(table ? 0 : conn->querySchema(QString(name)))
{
    if (table && !m_table)
        KexiDBWarn << "TableOrQuery(Connection *conn, const QByteArray& name, bool table) : "
        "no table specified!";
    if (!table && !m_query)
        KexiDBWarn << "TableOrQuery(Connection *conn, const QByteArray& name, bool table) : "
        "no query specified!";
}

TableOrQuerySchema::TableOrQuerySchema(FieldList &tableOrQuery)
        : m_table(dynamic_cast<TableSchema*>(&tableOrQuery))
        , m_query(dynamic_cast<QuerySchema*>(&tableOrQuery))
{
    if (!m_table && !m_query)
        KexiDBWarn << "TableOrQuery(FieldList &tableOrQuery) : "
        " tableOrQuery is nether table nor query!";
}

TableOrQuerySchema::TableOrQuerySchema(Connection *conn, int id)
{
    m_table = conn->tableSchema(id);
    m_query = m_table ? 0 : conn->querySchema(id);
    if (!m_table && !m_query)
        KexiDBWarn << "TableOrQuery(Connection *conn, int id) : no table or query found for id=="
        << id << "!";
}

TableOrQuerySchema::TableOrQuerySchema(TableSchema* table)
        : m_table(table)
        , m_query(0)
{
    if (!m_table)
        KexiDBWarn << "TableOrQuery(TableSchema* table) : no table specified!";
}

TableOrQuerySchema::TableOrQuerySchema(QuerySchema* query)
        : m_table(0)
        , m_query(query)
{
    if (!m_query)
        KexiDBWarn << "TableOrQuery(QuerySchema* query) : no query specified!";
}

uint TableOrQuerySchema::fieldCount() const
{
    if (m_table)
        return m_table->fieldCount();
    if (m_query)
        return m_query->fieldsExpanded().size();
    return 0;
}

const QueryColumnInfo::Vector TableOrQuerySchema::columns(bool unique)
{
    if (m_table)
        return m_table->query()->fieldsExpanded(unique ? QuerySchema::Unique : QuerySchema::Default);

    if (m_query)
        return m_query->fieldsExpanded(unique ? QuerySchema::Unique : QuerySchema::Default);

    KexiDBWarn << "TableOrQuery::fields() : no query or table specified!";
    return QueryColumnInfo::Vector();
}

QByteArray TableOrQuerySchema::name() const
{
    if (m_table)
        return m_table->name().toLatin1();
    if (m_query)
        return m_query->name().toLatin1();
    return m_name;
}

QString TableOrQuerySchema::captionOrName() const
{
    SchemaData *sdata = m_table ? static_cast<SchemaData *>(m_table) : static_cast<SchemaData *>(m_query);
    if (!sdata)
        return m_name;
    return sdata->caption().isEmpty() ? sdata->name() : sdata->caption();
}

Field* TableOrQuerySchema::field(const QString& name)
{
    if (m_table)
        return m_table->field(name);
    if (m_query)
        return m_query->field(name);

    return 0;
}

QueryColumnInfo* TableOrQuerySchema::columnInfo(const QString& name)
{
    if (m_table)
        return m_table->query()->columnInfo(name);

    if (m_query)
        return m_query->columnInfo(name);

    return 0;
}

QString TableOrQuerySchema::debugString()
{
    if (m_table)
        return m_table->debugString();
    else if (m_query)
        return m_query->debugString();
    return QString();
}

void TableOrQuerySchema::debug()
{
    if (m_table)
        return m_table->debug();
    else if (m_query)
        return m_query->debug();
}

Connection* TableOrQuerySchema::connection() const
{
    if (m_table)
        return m_table->connection();
    else if (m_query)
        return m_query->connection();
    return 0;
}


//------------------------------------------

ConnectionTestThread::ConnectionTestThread(ConnectionTestDialog* dlg, const KexiDB::ConnectionData& connData)
        : m_dlg(dlg), m_connData(connData)
{
    connect(this, SIGNAL(error(const QString&,const QString&)),
            dlg, SLOT(error(const QString&,const QString&)), Qt::QueuedConnection);

    // try to load driver now because it's not supported in different thread
    KexiDB::DriverManager manager;
    m_driver = manager.driver(m_connData.driverName);
    if (manager.error()) {
        emitError(&manager);
        m_driver = 0;
    }
}

void ConnectionTestThread::emitError(KexiDB::Object* object)
{
    QString msg;
    QString details;
    KexiDB::getHTMLErrorMesage(object, msg, details);
    emit error(msg, details);
}

void ConnectionTestThread::run()
{
    if (!m_driver) {
        return;
    }
    std::auto_ptr<KexiDB::Connection> conn(m_driver->createConnection(m_connData));
    if (!conn.get() || m_driver->error()) {
        //kDebug() << "err 1";
        emitError(m_driver);
        return;
    }
    if (!conn.get()->connect() || conn.get()->error()) {
        //kDebug() << "err 2";
        emitError(conn.get());
        return;
    }
    // SQL database backends like PostgreSQL require executing "USE database"
    // if we really want to know connection to the server succeeded.
    QString tmpDbName;
    if (!conn->useTemporaryDatabaseIfNeeded(tmpDbName)) {
        //kDebug() << "err 3";
        emitError(conn.get());
        return;
    }
    //kDebug() << "emitError(0)";
    emitError(0);
}

ConnectionTestDialog::ConnectionTestDialog(QWidget* parent,
        const KexiDB::ConnectionData& data,
        KexiDB::MessageHandler& msgHandler)
        : KProgressDialog(parent,
                          i18n("Test Connection"),
                          i18n("<qt>Testing connection to <b>%1</b> database server...</qt>",
                               data.serverInfoString(true))
                         )
        , m_thread(new ConnectionTestThread(this, data))
        , m_connData(data)
        , m_msgHandler(&msgHandler)
        , m_elapsedTime(0)
        , m_error(false)
        , m_stopWaiting(false)
{
    setModal(true);
    showCancelButton(true);
    progressBar()->setFormat(""); //hide %
    progressBar()->setRange(0, 0); //to show busy indicator
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    adjustSize();
    resize(250, height());
}

ConnectionTestDialog::~ConnectionTestDialog()
{
    m_wait.wakeAll();
    m_thread->terminate();
    delete m_thread;
}

int ConnectionTestDialog::exec()
{
    //kDebug() << "tid:" << QThread::currentThread() << "this_thread:" << thread();
    m_timer.start(20);
    m_thread->start();
    const int res = KProgressDialog::exec();
    m_thread->wait();
    m_timer.stop();
    return res;
}

void ConnectionTestDialog::slotTimeout()
{
    //kDebug() << "tid:" << QThread::currentThread() << "this_thread:" << thread();
    KexiDBDbg << m_error;
    bool notResponding = false;
    if (m_elapsedTime >= 1000*5) {//5 seconds
        m_stopWaiting = true;
        notResponding = true;
    }
    KexiDBDbg << m_elapsedTime << m_stopWaiting << notResponding;
    if (m_stopWaiting) {
        m_timer.disconnect(this);
        m_timer.stop();
        reject();
        KexiDBDbg << "after reject";
//  close();
        if (m_error) {
            KexiDBDbg << "show?";
            m_msgHandler->showErrorMessage(m_msg, m_details);
            //m_msgHandler->showErrorMessage(m_errorObj);
            KexiDBDbg << "shown";
            m_error = false;
        } else if (notResponding) {
            KMessageBox::sorry(0,
                               i18n("<qt>Test connection to <b>%1</b> database server failed. The server is not responding.</qt>", m_connData.serverInfoString(true)),
                               i18n("Test Connection"));
        } else {
            KMessageBox::information(0,
                                     i18n("<qt>Test connection to <b>%1</b> database server established successfully.</qt>",
                                          m_connData.serverInfoString(true)),
                                     i18n("Test Connection"));
        }
//  slotCancel();
//  reject();
        m_wait.wakeAll();
        return;
    }
    m_elapsedTime += 20;
    progressBar()->setValue(m_elapsedTime);
}

void ConnectionTestDialog::error(const QString& msg, const QString& details)
{
    //kDebug() << "tid:" << QThread::currentThread() << "this_thread:" << thread();
    kDebug() << msg << details;
    m_stopWaiting = true;
    if (!msg.isEmpty() || !details.isEmpty()) {
        m_error = true;
        m_msg = msg;
        m_details = details;
        kDebug() << "ERR!";
//        m_msgHandler->showErrorMessage(msg, details);
    }
}

void ConnectionTestDialog::reject()
{
// m_wait.wakeAll();
    m_thread->terminate();
    m_timer.disconnect(this);
    m_timer.stop();
    KProgressDialog::reject();
}

void KexiDB::connectionTestDialog(QWidget* parent, const KexiDB::ConnectionData& data,
                                  KexiDB::MessageHandler& msgHandler)
{
    ConnectionTestDialog dlg(parent, data, msgHandler);
    dlg.exec();
}

int KexiDB::rowCount(Connection &conn, const QString& sql)
{
    int count = -1; //will be changed only on success of querySingleNumber()
    QString selectSql(QString::fromLatin1("SELECT COUNT() FROM (") + sql + ") AS kexidb__subquery");
    conn.querySingleNumber(selectSql, count);
    return count;
}

int KexiDB::rowCount(const KexiDB::TableSchema& tableSchema)
{
//! @todo does not work with non-SQL data sources
    if (!tableSchema.connection()) {
        KexiDBWarn << "KexiDB::rowsCount(const KexiDB::TableSchema&): no tableSchema.connection() !";
        return -1;
    }
    int count = -1; //will be changed only on success of querySingleNumber()
    tableSchema.connection()->querySingleNumber(
        QString::fromLatin1("SELECT COUNT(*) FROM ")
        + tableSchema.connection()->driver()->escapeIdentifier(tableSchema.name()),
        count
    );
    return count;
}

int KexiDB::rowCount(KexiDB::QuerySchema& querySchema)
{
//! @todo does not work with non-SQL data sources
    if (!querySchema.connection()) {
        KexiDBWarn << "KexiDB::rowsCount(const KexiDB::QuerySchema&): no querySchema.connection() !";
        return -1;
    }
    int count = -1; //will be changed only on success of querySingleNumber()
    querySchema.connection()->querySingleNumber(
        QString::fromLatin1("SELECT COUNT(*) FROM (")
        + querySchema.connection()->selectStatement(querySchema) + ") AS kexidb__subquery",
        count
    );
    return count;
}

int KexiDB::rowCount(KexiDB::TableOrQuerySchema& tableOrQuery)
{
    if (tableOrQuery.table())
        return rowCount(*tableOrQuery.table());
    if (tableOrQuery.query())
        return rowCount(*tableOrQuery.query());
    return -1;
}

int KexiDB::fieldCount(KexiDB::TableOrQuerySchema& tableOrQuery)
{
    if (tableOrQuery.table())
        return tableOrQuery.table()->fieldCount();
    if (tableOrQuery.query())
        return tableOrQuery.query()->fieldsExpanded().count();
    return -1;
}

QMap<QString, QString> KexiDB::toMap(const ConnectionData& data)
{
    QMap<QString, QString> m;
    m["caption"] = data.caption;
    m["description"] = data.description;
    m["driverName"] = data.driverName;
    m["hostName"] = data.hostName;
    m["port"] = QString::number(data.port);
    m["useLocalSocketFile"] = QString::number((int)data.useLocalSocketFile);
    m["localSocketFileName"] = data.localSocketFileName;
    m["password"] = data.password;
    m["savePassword"] = QString::number((int)data.savePassword);
    m["userName"] = data.userName;
    m["fileName"] = data.fileName();
    return m;
}

void KexiDB::fromMap(const QMap<QString, QString>& map, ConnectionData& data)
{
    data.caption = map["caption"];
    data.description = map["description"];
    data.driverName = map["driverName"];
    data.hostName = map["hostName"];
    data.port = map["port"].toInt();
    data.useLocalSocketFile = map["useLocalSocketFile"].toInt() == 1;
    data.localSocketFileName = map["localSocketFileName"];
    data.password = map["password"];
    data.savePassword = map["savePassword"].toInt() == 1;
    data.userName = map["userName"];
    data.setFileName(map["fileName"]);
}

bool KexiDB::splitToTableAndFieldParts(const QString& string,
                                       QString& tableName, QString& fieldName,
                                       SplitToTableAndFieldPartsOptions option)
{
    const int id = string.indexOf('.');
    if (option & SetFieldNameIfNoTableName && id == -1) {
        tableName.clear();
        fieldName = string;
        return !fieldName.isEmpty();
    }
    if (id <= 0 || id == int(string.length() - 1))
        return false;
    tableName = string.left(id);
    fieldName = string.mid(id + 1);
    return !tableName.isEmpty() && !fieldName.isEmpty();
}

bool KexiDB::supportsVisibleDecimalPlacesProperty(Field::Type type)
{
//! @todo add check for decimal type as well
    return Field::isFPNumericType(type);
}

QString KexiDB::formatNumberForVisibleDecimalPlaces(double value, int decimalPlaces)
{
//! @todo round?
    if (decimalPlaces < 0) {
        QString s(QString::number(value, 'f', 10 /*reasonable precision*/));
        uint i = s.length() - 1;
        while (i > 0 && s[i] == '0')
            i--;
        if (s[i] == '.') //remove '.'
            i--;
        s = s.left(i + 1).replace('.', KGlobal::locale()->decimalSymbol());
        return s;
    }
    if (decimalPlaces == 0)
        return QString::number((int)value);
    return KGlobal::locale()->formatNumber(value, decimalPlaces);
}

KexiDB::Field::Type KexiDB::intToFieldType(int type)
{
    if (type < (int)KexiDB::Field::InvalidType || type > (int)KexiDB::Field::LastType) {
        KexiDBWarn << "KexiDB::intToFieldType(): invalid type " << type;
        return KexiDB::Field::InvalidType;
    }
    return (KexiDB::Field::Type)type;
}

static bool setIntToFieldType(Field& field, const QVariant& value)
{
    bool ok;
    const int intType = value.toInt(&ok);
    if (!ok || KexiDB::Field::InvalidType == intToFieldType(intType)) {//for sanity
        KexiDBWarn << "KexiDB::setFieldProperties(): invalid type";
        return false;
    }
    field.setType((KexiDB::Field::Type)intType);
    return true;
}

//! @internal for KexiDB::isBuiltinTableFieldProperty()
struct KexiDB_BuiltinFieldProperties {
    KexiDB_BuiltinFieldProperties() {
#define ADD(name) set.insert(name)
        ADD("type");
        ADD("primaryKey");
        ADD("indexed");
        ADD("autoIncrement");
        ADD("unique");
        ADD("notNull");
        ADD("allowEmpty");
        ADD("unsigned");
        ADD("name");
        ADD("caption");
        ADD("description");
        ADD("maxLength");
        ADD("maxLengthIsDefault");
        ADD("precision");
        ADD("defaultValue");
        ADD("width");
        ADD("visibleDecimalPlaces");
//! @todo always update this when new builtins appear!
#undef ADD
    }
    QSet<QByteArray> set;
};

//! for KexiDB::isBuiltinTableFieldProperty()
K_GLOBAL_STATIC(KexiDB_BuiltinFieldProperties, KexiDB_builtinFieldProperties)


bool KexiDB::isBuiltinTableFieldProperty(const QByteArray& propertyName)
{
    return KexiDB_builtinFieldProperties->set.contains(propertyName);
}

bool KexiDB::setFieldProperties(Field& field, const QHash<QByteArray, QVariant>& values)
{
    QHash<QByteArray, QVariant>::ConstIterator it;
    if ((it = values.find("type")) != values.constEnd()) {
        if (!setIntToFieldType(field, *it))
            return false;
    }

#define SET_BOOLEAN_FLAG(flag, value) { \
        constraints |= KexiDB::Field::flag; \
        if (!value) \
            constraints ^= KexiDB::Field::flag; \
    }

    uint constraints = field.constraints();
    bool ok = true;
    if ((it = values.find("primaryKey")) != values.constEnd())
        SET_BOOLEAN_FLAG(PrimaryKey, (*it).toBool());
    if ((it = values.find("indexed")) != values.constEnd())
        SET_BOOLEAN_FLAG(Indexed, (*it).toBool());
    if ((it = values.find("autoIncrement")) != values.constEnd()
            && KexiDB::Field::isAutoIncrementAllowed(field.type()))
        SET_BOOLEAN_FLAG(AutoInc, (*it).toBool());
    if ((it = values.find("unique")) != values.constEnd())
        SET_BOOLEAN_FLAG(Unique, (*it).toBool());
    if ((it = values.find("notNull")) != values.constEnd())
        SET_BOOLEAN_FLAG(NotNull, (*it).toBool());
    if ((it = values.find("allowEmpty")) != values.constEnd())
        SET_BOOLEAN_FLAG(NotEmpty, !(*it).toBool());
    field.setConstraints(constraints);

    uint options = 0;
    if ((it = values.find("unsigned")) != values.constEnd()) {
        options |= KexiDB::Field::Unsigned;
        if (!(*it).toBool())
            options ^= KexiDB::Field::Unsigned;
    }
    field.setOptions(options);

    if ((it = values.find("name")) != values.constEnd())
        field.setName((*it).toString());
    if ((it = values.find("caption")) != values.constEnd())
        field.setCaption((*it).toString());
    if ((it = values.find("description")) != values.constEnd())
        field.setDescription((*it).toString());
    if ((it = values.find("maxLength")) != values.constEnd())
        field.setMaxLength((*it).isNull() ? 0/*default*/ : (*it).toUInt(&ok));
    if (!ok)
        return false;
    if ((it = values.find("maxLengthIsDefault")) != values.constEnd()
            && (*it).toBool())
    {
        field.setMaxLengthStrategy(Field::DefaultMaxLength);
    }
    if ((it = values.find("precision")) != values.constEnd())
        field.setPrecision((*it).isNull() ? 0/*default*/ : (*it).toUInt(&ok));
    if (!ok)
        return false;
    if ((it = values.find("defaultValue")) != values.constEnd())
        field.setDefaultValue(*it);
    if ((it = values.find("width")) != values.constEnd())
        field.setWidth((*it).isNull() ? 0/*default*/ : (*it).toUInt(&ok));
    if (!ok)
        return false;
    if ((it = values.find("visibleDecimalPlaces")) != values.constEnd()
            && KexiDB::supportsVisibleDecimalPlacesProperty(field.type()))
        field.setVisibleDecimalPlaces((*it).isNull() ? -1/*default*/ : (*it).toInt(&ok));
    if (!ok)
        return false;

    return true;
#undef SET_BOOLEAN_FLAG
}

//! @internal for isExtendedTableProperty()
struct KexiDB_ExtendedProperties {
    KexiDB_ExtendedProperties() {
#define ADD(name) set.insert( name )
        ADD("visibledecimalplaces");
        ADD("rowsource");
        ADD("rowsourcetype");
        ADD("rowsourcevalues");
        ADD("boundcolumn");
        ADD("visiblecolumn");
        ADD("columnwidths");
        ADD("showcolumnheaders");
        ADD("listrows");
        ADD("limittolist");
        ADD("displaywidget");
#undef ADD
    }
    QSet<QByteArray> set;
};

//! for isExtendedTableProperty()
K_GLOBAL_STATIC(KexiDB_ExtendedProperties, KexiDB_extendedProperties)

bool KexiDB::isExtendedTableFieldProperty(const QByteArray& propertyName)
{
    return KexiDB_extendedProperties->set.contains(QByteArray(propertyName).toLower());
}

bool KexiDB::setFieldProperty(Field& field, const QByteArray& propertyName, const QVariant& value)
{
#define SET_BOOLEAN_FLAG(flag, value) { \
        constraints |= KexiDB::Field::flag; \
        if (!value) \
            constraints ^= KexiDB::Field::flag; \
        field.setConstraints( constraints ); \
        return true; \
    }
#define GET_INT(method) { \
        const uint ival = value.toUInt(&ok); \
        if (!ok) \
            return false; \
        field.method( ival ); \
        return true; \
    }

    if (propertyName.isEmpty())
        return false;

    bool ok;
    if (KexiDB::isExtendedTableFieldProperty(propertyName)) {
        //a little speedup: identify extended property in O(1)
        if ("visibleDecimalPlaces" == propertyName
                && KexiDB::supportsVisibleDecimalPlacesProperty(field.type())) {
            GET_INT(setVisibleDecimalPlaces);
        } else {
            if (!field.table()) {
                KexiDBWarn << QString(
                    "KexiDB::setFieldProperty() Cannot set \"%1\" property - no table assigned for field!")
                .arg(QString(propertyName));
            } else {
                LookupFieldSchema *lookup = field.table()->lookupFieldSchema(field);
                const bool hasLookup = lookup != 0;
                if (!hasLookup)
                    lookup = new LookupFieldSchema();
                if (LookupFieldSchema::setProperty(*lookup, propertyName, value)) {
                    if (!hasLookup && lookup)
                        field.table()->setLookupFieldSchema(field.name(), lookup);
                    return true;
                }
                delete lookup;
            }
        }
    } else {//non-extended
        if ("type" == propertyName)
            return setIntToFieldType(field, value);

        uint constraints = field.constraints();
        if ("primaryKey" == propertyName)
            SET_BOOLEAN_FLAG(PrimaryKey, value.toBool());
        if ("indexed" == propertyName)
            SET_BOOLEAN_FLAG(Indexed, value.toBool());
        if ("autoIncrement" == propertyName
                && KexiDB::Field::isAutoIncrementAllowed(field.type()))
            SET_BOOLEAN_FLAG(AutoInc, value.toBool());
        if ("unique" == propertyName)
            SET_BOOLEAN_FLAG(Unique, value.toBool());
        if ("notNull" == propertyName)
            SET_BOOLEAN_FLAG(NotNull, value.toBool());
        if ("allowEmpty" == propertyName)
            SET_BOOLEAN_FLAG(NotEmpty, !value.toBool());

        uint options = 0;
        if ("unsigned" == propertyName) {
            options |= KexiDB::Field::Unsigned;
            if (!value.toBool())
                options ^= KexiDB::Field::Unsigned;
            field.setOptions(options);
            return true;
        }

        if ("name" == propertyName) {
            if (value.toString().isEmpty())
                return false;
            field.setName(value.toString());
            return true;
        }
        if ("caption" == propertyName) {
            field.setCaption(value.toString());
            return true;
        }
        if ("description" == propertyName) {
            field.setDescription(value.toString());
            return true;
        }
        if ("maxLength" == propertyName)
            GET_INT(setMaxLength);
        if ("maxLengthIsDefault" == propertyName) {
            field.setMaxLengthStrategy(Field::DefaultMaxLength);
        }
        if ("precision" == propertyName)
            GET_INT(setPrecision);
        if ("defaultValue" == propertyName) {
            field.setDefaultValue(value);
            return true;
        }
        if ("width" == propertyName)
            GET_INT(setWidth);

        // last chance that never fails: custom field property
        field.setCustomProperty(propertyName, value);
    }

    KexiDBWarn << "KexiDB::setFieldProperty() property \"" << propertyName << "\" not found!";
    return false;
#undef SET_BOOLEAN_FLAG
#undef GET_INT
}

int KexiDB::loadIntPropertyValueFromDom(const QDomNode& node, bool* ok)
{
    QByteArray valueType = node.nodeName().toLatin1();
    if (valueType.isEmpty() || valueType != "number") {
        if (ok)
            *ok = false;
        return 0;
    }
    const QString text(QDomNode(node).toElement().text());
    int val = text.toInt(ok);
    return val;
}

QString KexiDB::loadStringPropertyValueFromDom(const QDomNode& node, bool* ok)
{
    QByteArray valueType = node.nodeName().toLatin1();
    if (valueType != "string") {
        if (ok)
            *ok = false;
        return QString();
    }
    if (ok)
        *ok = true;
    return QDomNode(node).toElement().text();
}

QVariant KexiDB::loadPropertyValueFromDom(const QDomNode& node, bool* ok)
{
    QByteArray valueType = node.nodeName().toLatin1();
    if (valueType.isEmpty()) {
        if (ok)
            *ok = false;
        return QVariant();
    }
    if (ok)
        *ok = true;
    const QString text(QDomNode(node).toElement().text());
    bool _ok;
    if (valueType == "string") {
        return text;
    }
    else if (valueType == "cstring") {
        return text.toLatin1();
    }
    else if (valueType == "number") { // integer or double
        if (text.indexOf('.') != -1) {
            double val = text.toDouble(&_ok);
            if (_ok)
                return val;
        }
        else {
            const int val = text.toInt(&_ok);
            if (_ok)
                return val;
            const qint64 valLong = text.toLongLong(&_ok);
            if (_ok)
                return valLong;
        }
    }
    else if (valueType == "bool") {
        return QVariant(text.toLower() == "true" || text == "1");
    }
    else {
//! @todo add more QVariant types
        KexiDBWarn << "loadPropertyValueFromDom(): unknown type '" << valueType << "'";
    }
    if (ok)
        *ok = false;
    return QVariant();
}

QDomElement KexiDB::saveNumberElementToDom(QDomDocument& doc, QDomElement& parentEl,
        const QString& elementName, int value)
{
    QDomElement el(doc.createElement(elementName));
    parentEl.appendChild(el);
    QDomElement numberEl(doc.createElement("number"));
    el.appendChild(numberEl);
    numberEl.appendChild(doc.createTextNode(QString::number(value)));
    return el;
}

QDomElement KexiDB::saveBooleanElementToDom(QDomDocument& doc, QDomElement& parentEl,
        const QString& elementName, bool value)
{
    QDomElement el(doc.createElement(elementName));
    parentEl.appendChild(el);
    QDomElement numberEl(doc.createElement("bool"));
    el.appendChild(numberEl);
    numberEl.appendChild(doc.createTextNode(
                             value ? QString::fromLatin1("true") : QString::fromLatin1("false")));
    return el;
}

//! @internal Used in KexiDB::emptyValueForType()
struct KexiDB_EmptyValueForTypeCache {
    KexiDB_EmptyValueForTypeCache()
            : values(int(Field::LastType) + 1) {
#define ADD(t, value) values.insert(t, value);
        ADD(Field::Byte, 0);
        ADD(Field::ShortInteger, 0);
        ADD(Field::Integer, 0);
        ADD(Field::BigInteger, 0);
        ADD(Field::Boolean, false);
        ADD(Field::Float, 0.0);
        ADD(Field::Double, 0.0);
//! @todo ok? we have no better defaults
        ADD(Field::Text, QString(" "));
        ADD(Field::LongText, QString(" "));
        ADD(Field::BLOB, QByteArray());
#undef ADD
    }
    QVector<QVariant> values;
};

//! Used in KexiDB::emptyValueForType()
K_GLOBAL_STATIC(KexiDB_EmptyValueForTypeCache, KexiDB_emptyValueForTypeCache)

QVariant KexiDB::emptyValueForType(KexiDB::Field::Type type)
{
    const QVariant val(KexiDB_emptyValueForTypeCache->values.at(
                           (type <= Field::LastType) ? type : Field::InvalidType));
    if (!val.isNull())
        return val;
    else { //special cases
        if (type == Field::Date)
            return QDate::currentDate();
        if (type == Field::DateTime)
            return QDateTime::currentDateTime();
        if (type == Field::Time)
            return QTime::currentTime();
    }
    KexiDBWarn << "KexiDB::emptyValueForType() no value for type "
    << Field::typeName(type);
    return QVariant();
}

//! @internal Used in KexiDB::notEmptyValueForType()
struct KexiDB_NotEmptyValueForTypeCache {
    KexiDB_NotEmptyValueForTypeCache()
            : values(int(Field::LastType) + 1) {
#define ADD(t, value) values.insert(t, value);
        // copy most of the values
        for (int i = int(Field::InvalidType) + 1; i <= Field::LastType; i++) {
            if (i == Field::Date || i == Field::DateTime || i == Field::Time)
                continue; //'current' value will be returned
            if (i == Field::Text || i == Field::LongText) {
                ADD(i, QVariant(QString("")));
                continue;
            }
            if (i == Field::BLOB) {
//! @todo blobs will contain other mime types too
                QByteArray ba;
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                QPixmap pm(koSmallIcon("document-new"));
                pm.save(&buffer, "PNG"/*! @todo default? */);
                ADD(i, ba);
                continue;
            }
            ADD(i, KexiDB::emptyValueForType((Field::Type)i));
        }
#undef ADD
    }
    QVector<QVariant> values;
};
//! Used in KexiDB::notEmptyValueForType()
K_GLOBAL_STATIC(KexiDB_NotEmptyValueForTypeCache, KexiDB_notEmptyValueForTypeCache)

QVariant KexiDB::notEmptyValueForType(KexiDB::Field::Type type)
{
    const QVariant val(KexiDB_notEmptyValueForTypeCache->values.at(
                           (type <= Field::LastType) ? type : Field::InvalidType));
    if (!val.isNull())
        return val;
    else { //special cases
        if (type == Field::Date)
            return QDate::currentDate();
        if (type == Field::DateTime)
            return QDateTime::currentDateTime();
        if (type == Field::Time)
            return QTime::currentTime();
    }
    KexiDBWarn << "KexiDB::notEmptyValueForType() no value for type "
    << Field::typeName(type);
    return QVariant();
}

QString KexiDB::escapeBLOB(const QByteArray& array, BLOBEscapingType type)
{
    const int size = array.size();
    if (size == 0)
        return QString();
    int escaped_length = size * 2;
    if (type == BLOBEscape0xHex || type == BLOBEscapeOctal)
        escaped_length += 2/*0x or X'*/;
    else if (type == BLOBEscapeXHex)
        escaped_length += 3; //X' + '
    QString str;
    str.reserve(escaped_length);
    if (str.capacity() < escaped_length) {
        KexiDBWarn << "KexiDB::Driver::escapeBLOB(): no enough memory (cannot allocate " <<
        escaped_length << " chars)";
        return QString();
    }
    if (type == BLOBEscapeXHex)
        str = QString::fromLatin1("X'");
    else if (type == BLOBEscape0xHex)
        str = QString::fromLatin1("0x");
    else if (type == BLOBEscapeOctal)
        str = QString::fromLatin1("'");

    int new_length = str.length(); //after X' or 0x, etc.
    if (type == BLOBEscapeOctal) {
        // only escape nonprintable characters as in Table 8-7:
        // http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
        // i.e. escape for bytes: < 32, >= 127, 39 ('), 92(\).
        for (int i = 0; i < size; i++) {
            const unsigned char val = array[i];
            if (val < 32 || val >= 127 || val == 39 || val == 92) {
                str[new_length++] = '\\';
                str[new_length++] = '\\';
                str[new_length++] = '0' + val / 64;
                str[new_length++] = '0' + (val % 64) / 8;
                str[new_length++] = '0' + val % 8;
            } else {
                str[new_length++] = val;
            }
        }
    } else {
        for (int i = 0; i < size; i++) {
            const unsigned char val = array[i];
            str[new_length++] = (val / 16) < 10 ? ('0' + (val / 16)) : ('A' + (val / 16) - 10);
            str[new_length++] = (val % 16) < 10 ? ('0' + (val % 16)) : ('A' + (val % 16) - 10);
        }
    }
    if (type == BLOBEscapeXHex || type == BLOBEscapeOctal)
        str[new_length++] = '\'';
    return str;
}

QByteArray KexiDB::pgsqlByteaToByteArray(const char* data, int length)
{
    QByteArray array;
    int output = 0;
    for (int pass = 0; pass < 2; pass++) {//2 passes to avoid allocating buffer twice:
        //  0: count #of chars; 1: copy data
        const char* s = data;
        const char* end = s + length;
        if (pass == 1) {
            KexiDBDbg << "processBinaryData(): real size == " << output;
            array.resize(output);
            output = 0;
        }
        for (int input = 0; s < end; ++output) {
            //  KexiDBDbg<<(int)s[0]<<" "<<(int)s[1]<<" "<<(int)s[2]<<" "<<(int)s[3]<<" "<<(int)s[4];
            if (s[0] == '\\' && (s + 1) < end) {
                //special cases as in http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
                if (s[1] == '\'') {// \'
                    if (pass == 1)
                        array[output] = '\'';
                    s += 2;
                } else if (s[1] == '\\') { // 2 backslashes
                    if (pass == 1)
                        array[output] = '\\';
                    s += 2;
                } else if ((input + 3) < length) {// \\xyz where xyz are 3 octal digits
                    if (pass == 1)
                        array[output] = char((int(s[1] - '0') * 8 + int(s[2] - '0')) * 8 + int(s[3] - '0'));
                    s += 4;
                } else {
                    KexiDBDrvWarn << "processBinaryData(): no octal value after backslash";
                    s++;
                }
            } else {
                if (pass == 1)
                    array[output] = s[0];
                s++;
            }
            //  KexiDBDbg<<output<<": "<<(int)array[output];
        }
    }
    return array;
}

QString KexiDB::variantToString(const QVariant& v)
{
    if (v.type() == QVariant::ByteArray)
        return KexiDB::escapeBLOB(v.toByteArray(), KexiDB::BLOBEscapeHex);
    return v.toString();
}

QVariant KexiDB::stringToVariant(const QString& s, QVariant::Type type, bool &ok)
{
    if (s.isNull()) {
        ok = true;
        return QVariant();
    }
    if (QVariant::Invalid == type) {
        ok = false;
        return QVariant();
    }
    if (type == QVariant::ByteArray) {//special case: hex string
        const uint len = s.length();
        QByteArray ba;
        ba.resize(len / 2 + len % 2);
        for (uint i = 0; i < (len - 1); i += 2) {
            int c = s.mid(i, 2).toInt(&ok, 16);
            if (!ok) {
                KexiDBWarn << "KexiDB::stringToVariant(): Error in digit " << i;
                return QVariant();
            }
            ba[i/2] = (char)c;
        }
        ok = true;
        return ba;
    }
    QVariant result(s);
    if (!result.convert(type)) {
        ok = false;
        return QVariant();
    }
    ok = true;
    return result;
}

bool KexiDB::isDefaultValueAllowed(KexiDB::Field* field)
{
    return field && !field->isUniqueKey();
}

void KexiDB::getLimitsForType(Field::Type type, int &minValue, int &maxValue)
{
    switch (type) {
    case Field::Byte:
//! @todo always ok?
        minValue = 0;
        maxValue = 255;
        break;
    case Field::ShortInteger:
        minValue = -32768;
        maxValue = 32767;
        break;
    case Field::Integer:
    case Field::BigInteger: //cannot return anything larger
    default:
        minValue = (int) - 0x07FFFFFFF;
        maxValue = (int)(0x080000000 - 1);
    }
}

Field::Type KexiDB::maximumForIntegerTypes(Field::Type t1, Field::Type t2)
{
    if (!Field::isIntegerType(t1) || !Field::isIntegerType(t2))
        return Field::InvalidType;
    if (t1 == t2)
        return t2;
    if (t1 == Field::ShortInteger && t2 != Field::Integer && t2 != Field::BigInteger)
        return t1;
    if (t1 == Field::Integer && t2 != Field::BigInteger)
        return t1;
    if (t1 == Field::BigInteger)
        return t1;
    return KexiDB::maximumForIntegerTypes(t2, t1); //swap
}

QString KexiDB::simplifiedTypeName(const Field& field)
{
    if (field.isNumericType())
        return i18n("Number"); //simplify
    else if (field.type() == Field::BLOB)
//! @todo support names of other BLOB subtypes
        return i18n("Image"); //simplify

    return field.typeGroupName();
}

QString KexiDB::defaultFileBasedDriverMimeType()
{
    return QString::fromLatin1("application/x-kexiproject-sqlite3");
}

QString KexiDB::defaultFileBasedDriverIconName()
{
    KMimeType::Ptr mimeType(KMimeType::mimeType(
                                KexiDB::defaultFileBasedDriverMimeType()));
    if (mimeType.isNull()) {
        KexiDBWarn << QString("'%1' mimetype not installed!")
        .arg(KexiDB::defaultFileBasedDriverMimeType());
        return QString();
    }
    return mimeType->iconName();
}

QString KexiDB::defaultFileBasedDriverName()
{
    DriverManager dm;
    return dm.lookupByMime(KexiDB::defaultFileBasedDriverMimeType()).toLower();
}

//--------------------------------------------------------------------------------

//! @internal
class StaticSetOfStrings::Private
{
public:
    Private() : array(0), set(0) {}
    ~Private() {
        delete set;
    }
    const char** array;
    QSet<QByteArray> *set;
};

StaticSetOfStrings::StaticSetOfStrings()
        : d(new Private)
{
}

StaticSetOfStrings::StaticSetOfStrings(const char* array[])
        : d(new Private)
{
    setStrings(array);
}

StaticSetOfStrings::~StaticSetOfStrings()
{
    delete d;
}

void StaticSetOfStrings::setStrings(const char* array[])
{
    delete d->set;
    d->set = 0;
    d->array = array;
}

bool StaticSetOfStrings::isEmpty() const
{
    return d->array == 0;
}

bool StaticSetOfStrings::contains(const QByteArray& string) const
{
    if (!d->set) {
        d->set = new QSet<QByteArray>();
        for (const char ** p = d->array;*p;p++) {
            d->set->insert(QByteArray::fromRawData(*p, qstrlen(*p)));
        }
    }
    return d->set->contains(string);
}

//--------------------------------------------------------------------------------

bool KexiDB::isIdentifier(const QString& s)
{
    uint i;
    const uint sLength = s.length();
    for (i = 0; i < sLength; i++) {
        QChar c = s.at(i).toLower();
        if (!(c == '_' || (c >= 'a' && c <= 'z') || (i > 0 && c >= '0' && c <= '9')))
            break;
    }
    return i > 0 && i == sLength;
}

QString KexiDB::string2FileName(const QString &s)
{
    QString fn = s.simplified();
    fn.replace(' ', "_"); fn.replace('$', "_");
    fn.replace('\\', "-"); fn.replace('/', "-");
    fn.replace(':', "-"); fn.replace('*', "-");
    return fn;
}

#include "utils_p.moc"
