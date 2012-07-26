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

#ifndef KEXIDB_CONNECTION_H
#define KEXIDB_CONNECTION_H

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QVector>
#include <QVariant>
#include <QPointer>

#include "object.h"
#include "connectiondata.h"
#include "tableschema.h"
#include "queryschema.h"
#include "queryschemaparameter.h"
#include "transaction.h"
#include "driver.h"
#include "preparedstatement.h"
#include "RecordData.h"

#include "tristate.h"

namespace KexiDB
{

class Cursor;
class ConnectionPrivate;
class RowEditBuffer;
class DatabaseProperties;
class AlterTableHandler;

/*! @short Provides database connection, allowing queries and data modification.

 This class represents a database connection established within a data source.
 It supports data queries and modification by creating client-side database cursors.
 Database transactions are supported.
*/
class CALLIGRADB_EXPORT Connection : public QObject, public KexiDB::Object
{
    Q_OBJECT

public:

    /*! Opened connection is automatically disconnected and removed
     from driver's connections list.
     Note for driver developers: you should call destroy()
     from you Connection's subclass destructor. */
    virtual ~Connection();

    /*! \return parameters that were used to create this connection. */
    ConnectionData* data() const;

    /*! \return the driver used for this connection. */
    inline Driver* driver() const {
        return m_driver;
    }

    /*!
    \brief Connects to driver with given parameters.
    \return true if successful. */
    bool connect();

    /*! \return true, if connection is properly established. */
    bool isConnected() const;

    /*! \return true, both if connection is properly established
     and any database within this connection is properly used
     with useDatabase(). */
    bool isDatabaseUsed() const;

    /*! \return true for read only connection. Used especially for file-based drivers.
     Can be reimplemented in a driver to provide real read-only flag of the connection
     (SQlite3 dirver does this). */
    virtual bool isReadOnly() const;

    /*! Reimplemented from Object: also clears sql string.
     @sa recentSQLString() */
    virtual void clearError();

    /*! \brief Disconnects from driver with given parameters.

     The database (if used) is closed, and any active transactions
     (if supported) are rolled back, so commit these before disconnecting,
     if you'd like to save your changes. */
    bool disconnect();

    /*! \return list of database names for opened connection.
     If \a also_system_db is true, the system database names are also returned. */
    QStringList databaseNames(bool also_system_db = false);

    /*! \return true if database \a dbName exists.
     If \a ignoreErrors is true, error flag of connection
      won't be modified for any errors (it will quietly return),
      else (ignoreErrors == false) we can check why the database does
      not exist using error(), errorNum() and/or errorMsg(). */
    bool databaseExists(const QString &dbName, bool ignoreErrors = true);

    /*! \brief Creates new database with name \a dbName, using this connection.

     If database with \a dbName already exists, or other error occurred,
     false is returned.
     For file-based drivers, \a dbName should be equal to the database
     filename (the same as specified for ConnectionData).

     See doc/dev/kexidb_issues.txt document, chapter "Table schema, query schema, etc. storage"
     for database schema documentation (detailed description of kexi__* 'system' tables).

     \sa useDatabase() */
    bool createDatabase(const QString &dbName);

    /*!
    \brief Opens an existing database specified by \a dbName.

     If \a kexiCompatible is true (the default) initial checks will be performed
     to recognize database Kexi-specific format. Set \a kexiCompatible to false
     if you're using native database (one that have no Kexi System tables).
     For file-based drivers, \a dbName should be equal to filename
     (the same as specified for ConnectionData).
     \return true on success, false on failure.
     If user has cancelled this action and \a cancelled is not 0, *cancelled is set to true. */
    bool useDatabase(const QString &dbName, bool kexiCompatible = true, bool *cancelled = 0,
                     MessageHandler* msgHandler = 0);

    /*!
    \brief Closes currently used database for this connection.

     Any active transactions (if supported) are rolled back,
     so commit these before closing, if you'd like to save your changes. */
    bool closeDatabase();

    /*! \brief Get the name of the current database

    \return name of currently used database for this connection or empty string
      if there is no used database */
    QString currentDatabase() const;

    /*! \brief Drops database with name \a dbName.

     if dbName is not specified, currently used database name is used
     (it is closed before dropping).
    */
    bool dropDatabase(const QString &dbName = QString());

    /*! \return names of all the \a objecttype (see \a ObjectType in global.h)
    schemas stored in currently used database. KexiDB::AnyObjectType can be passed
    as \a objType to get names of objects of any type.
    If \a ok is not null then variable pointed by it will be set to the result.
    On error, the functions can return incomplete list. */
    QStringList objectNames(int objType = KexiDB::AnyObjectType, bool* ok = 0);

    /*! \return names of all table schemas stored in currently
     used database. If \a also_system_tables is true,
     internal KexiDB system table names (kexi__*) are also returned.
     \sa kexiDBSystemTableNames() */
    QStringList tableNames(bool also_system_tables = false);

    /*! \return list of internal KexiDB system table names
     (kexi__*). This does not mean that these tables can be found
     in currently opened database. Just static list of table
     names is returned.

     The list contents may depend on KexiDB library version;
     opened database can contain fewer 'system' tables than in current
     KexiDB implementation, if the current one is newer than the one used
     to build the database. */
    static const QStringList& kexiDBSystemTableNames();

    /*! \return server version information for this connection.
     If database is not connected (i.e. isConnected() is false) 0 is returned. */
    KexiDB::ServerVersionInfo* serverVersion() const;

    /*! \return version information for this connection.
     If database is not used (i.e. isDatabaseUsed() is false) 0 is returned.
     It can be compared to drivers' and KexiDB library version to maintain
     backward/upward compatiblility. */
    KexiDB::DatabaseVersionInfo* databaseVersion() const;

    /*! \return DatabaseProperties object allowing to read and write global database properties
     for this connection. */
    DatabaseProperties& databaseProperties();

    /*! \return ids of all table schema names stored in currently
     used database. These ids can be later used as argument for tableSchema().
     This is a shortcut for objectIds(TableObjectType).
     Internal KexiDB system tables (kexi__*) are not available here
     because these have no identifiers assigned (more formally: id=-1).

     Note: the fact that given id is on the returned list does not mean
     that tableSchema( id ) returns anything. The table definition can be broken,
     so you have to double check this. */
    QList<int> tableIds();

    /*! \return ids of all database query schemas stored in currently
     used database. These ids can be later used as argument for querySchema().
     This is a shortcut for objectIds(QueryObjectType).

     Note: the fact that given id is on the returned list does not mean
     that querySchema( id ) returns anything. The query definition can be broken,
     so you have to double check this.

     @see tableIds()
     */
    QList<int> queryIds();

    /*! \return names of all schemas of object with \a objType type
     that are stored in currently used database.

     Note: the fact that given id is on the returned list does not mean
     that the definition of the object is valid,
     so you have to double check this.

     @see queryIds() */
    QList<int> objectIds(int objType);

    /*! \brief Creates new transaction handle and starts a new transaction.
     \return KexiDB::Transaction object if transaction has been started
     successfully, otherwise null transaction.
     For drivers that allow single transaction per connection
     (Driver::features() && SingleTransactions) this method can be called one time,
     and then this single transaction will be default ( setDefaultTransaction() will
     be called).
     For drivers that allow multiple transactions per connection, no default transaction is
     set automatically in beginTransaction() method, you could do this by hand.
     \sa setDefaultTransaction(), defaultTransaction().
    */
    Transaction beginTransaction();

    /*! \todo for nested transactions:
        Tansaction* beginTransaction(transaction *parent_transaction);
    */
    /*! Commits transaction \a trans.
     If there is not \a trans argument passed, and there is default transaction
     (obtained from defaultTransaction()) defined, this one will be committed.
     If default is not present, false is returned (when ignore_inactive is
     false, the default), or true is returned (when ignore_inactive is true).

     On successful commit, \a trans object will be destroyed.
     If this was default transaction, there is no default transaction for now.
    */
    bool commitTransaction(Transaction trans = Transaction(),
                           bool ignore_inactive = false);

    /*! Rollbacks transaction \a trans.
     If there is not \a trans argument passed, and there is default transaction
     (obtained from defaultTransaction()) defined, this one will be rolled back.
     If default is not present, false is returned (when ignore_inactive is
     false, the default), or true is returned (when ignore_inactive is true).

     or any error occurred, false is returned.

     On successful rollback, \a trans object will be destroyed.
     If this was default transaction, there is no default transaction for now.
    */
    bool rollbackTransaction(Transaction trans = Transaction(),
                             bool ignore_inactive = false);

    /*! \return handle for default transaction for this connection
     or null transaction if there is no such a transaction defined.
     If transactions are supported: Any operation on database (e.g. inserts)
     that is started without specifying transaction context, will be performed
     in the context of this transaction.

     Returned null transaction doesn't mean that there is no transactions
     started at all.
     Default transaction can be defined automatically for some drivers --
     see beginTransaction().
     \sa KexiDB::Driver::transactionsSupported()
    */
    Transaction& defaultTransaction() const;

    /*! Sets default transaction that will be used as context for operations
     on data in opened database for this connection. */
    void setDefaultTransaction(const Transaction& trans);

    /*! \return set of handles of currently active transactions.
     Note that in multithreading environment some of these
     transactions can be already inactive after calling this method.
     Use Transaction::active() to check that. Inactive transaction
     handle is useless and can be safely dropped.
    */
    const QList<Transaction>& transactions();

    /*! \return true if "auto commit" option is on.

     When auto commit is on (the default on for any new Connection object),
     every sql functional statement (statement that changes
     data in the database implicitly starts a new transaction.
     This transaction is automatically committed
     after successful statement execution or rolled back on error.

     For drivers that do not support transactions (see Driver::features())
     this method shouldn't be called because it does nothing ans always returns false.

     No internal KexiDB object should changes this option, although auto commit's
     behaviour depends on database engine's specifics. Engines that support only single
     transaction per connection (see Driver::SingleTransactions),
     use this single connection for autocommiting, so if there is already transaction
     started by the KexiDB user program (with beginTransaction()), this transaction
     is committed before any sql functional statement execution. In this situation
     default transaction is also affected (see defaultTransaction()).

     Only for drivers that support nested transactions (Driver::NestedTransactions),
     autocommiting works independently from previously started transaction,

     For other drivers set this option off if you need use transaction
     for grouping more statements together.

     NOTE: nested transactions are not yet implemented in KexiDB API.
    */
    bool autoCommit() const;

    /*! Changes auto commit option. This does not affect currently started transactions.
     This option can be changed even when connection is not established.
     \sa autoCommit() */
    bool setAutoCommit(bool on);

    /*! driver-specific string escaping */
//js: MOVED TO Driver  virtual QString escapeString(const QString& str) const = 0;
//  virtual QCString escapeString(const QCString& str) const = 0;

    /*! Prepares SELECT query described by raw \a statement.
     \return opened cursor created for results of this query
     or NULL if there was any error. Cursor can have optionally applied \a cursor_options
     (one of more selected from KexiDB::Cursor::Options).
     Preparation means that returned cursor is created but not opened.
     Open this when you would like to do it with Cursor::open().

     Note for driver developers: you should initialize cursor engine-specific
     resources and return Cursor subclass' object
     (passing \a statement and \a cursor_options to it's constructor).
    */
    virtual Cursor* prepareQuery(const QString& statement, uint cursor_options = 0) = 0;

    /*! \overload prepareQuery( const QString& statement = QString(), uint cursor_options = 0)
     Prepares query described by \a query schema. \a params are values of parameters that
     will be inserted into places marked with [] before execution of the query.

     Note for driver developers: you should initialize cursor engine-specific
     resources and return Cursor subclass' object
     (passing \a query and \a cursor_options to it's constructor).
     Kexi SQL and driver-specific escaping is performed on table names.
    */
    Cursor* prepareQuery(QuerySchema& query, const QList<QVariant>& params,
                         uint cursor_options = 0);

    /*! \overload prepareQuery( QuerySchema& query, const QList<QVariant>& params,
      uint cursor_options = 0 )
     Prepares query described by \a query schema without parameters.
    */
    virtual Cursor* prepareQuery(QuerySchema& query, uint cursor_options = 0) = 0;

    /*! \overload prepareQuery( const QString& statement = QString(), uint cursor_options = 0)
     Statement is build from data provided by \a table schema,
     it is like "select * from table_name".*/
    Cursor* prepareQuery(TableSchema& table, uint cursor_options = 0);

    /*! Executes SELECT query described by \a statement.
     \return opened cursor created for results of this query
     or NULL if there was any error on the cursor creation or opening.
     Cursor can have optionally applied \a cursor_options
     (one of more selected from KexiDB::Cursor::Options).
     Identifiers in \a statement that are the same as keywords in Kexi
     SQL or the backend's SQL need to have been escaped.
     */
    Cursor* executeQuery(const QString& statement, uint cursor_options = 0);

    /*! \overload executeQuery( const QString& statement, uint cursor_options = 0 )
     \a params are values of parameters that
     will be inserted into places marked with [] before execution of the query.

     Statement is build from data provided by \a query schema.
     Kexi SQL and driver-specific escaping is performed on table names. */
    Cursor* executeQuery(QuerySchema& query, const QList<QVariant>& params,
                         uint cursor_options = 0);

    /*! \overload executeQuery( QuerySchema& query, const QList<QVariant>& params,
      uint cursor_options = 0 ) */
    Cursor* executeQuery(QuerySchema& query, uint cursor_options = 0);

    /*! \overload executeQuery( const QString& statement, uint cursor_options = 0 )
     Executes query described by \a query schema without parameters.
     Statement is build from data provided by \a table schema,
     it is like "select * from table_name".*/
    Cursor* executeQuery(TableSchema& table, uint cursor_options = 0);

    /*! Deletes cursor \a cursor previously created by functions like executeQuery()
     for this connection.
     There is an attempt to close the cursor with Cursor::close() if it was opened.
     Anyway, at last cursor is deleted.
     \return true if cursor is properly closed before deletion. */
    bool deleteCursor(Cursor *cursor);

    /*! \return schema of a table pointed by \a tableId, retrieved from currently
     used database. The schema is cached inside connection,
     so retrieval is performed only once, on demand. */
    TableSchema* tableSchema(int tableId);

    /*! \return schema of a table pointed by \a tableName, retrieved from currently
     used database. KexiDB system table schema can be also retrieved.
     \sa tableSchema( int tableId ) */
    TableSchema* tableSchema(const QString& tableName);

    /*! \return schema of a query pointed by \a queryId, retrieved from currently
     used database. The schema is cached inside connection,
     so retrieval is performed only once, on demand. */
    QuerySchema* querySchema(int queryId);

    /*! \return schema of a query pointed by \a queryName, retrieved from currently
     used database.  \sa querySchema( int queryId ) */
    QuerySchema* querySchema(const QString& queryName);

    /*! Sets \a queryName query obsolete by moving it out of the query sets, so it will not be
     accessible by querySchema( const QString& queryName ). The existing query object is not
     destroyed, to avoid problems when it's referenced. In this case,
     a new query schema will be retrieved directly from the backend.

     For now it's used in KexiQueryDesignerGuiEditor::storeLayout().
     This solves the problem when user has changed a query schema but already form still uses
     previously instantiated query schema.
     \return true if there is such query. Otherwise the method does nothing. */
    bool setQuerySchemaObsolete(const QString& queryName);

//js: MOVED TO Driver  QString valueToSQL( const Field::Type ftype, const QVariant& v ) const;
//  QString valueToSQL( const Field *field, const QVariant& v ) const;

    /*! Executes \a sql query and stores first record's data inside \a data.
     This is convenient method when we need only first record from query result,
     or when we know that query result has only one record.
     If \a addLimitTo1 is true (the default), adds a LIMIT clause to the query,
     so \a sql should not include one already.
     \return true if query was successfully executed and first record has been found,
     false on data retrieving failure, and cancelled if there's no single record available. */
    tristate querySingleRecord(const QString& sql, RecordData &data, bool addLimitTo1 = true);

    /*! Like tristate querySingleRecord(const QString& sql, RecordData &data)
     but uses QuerySchema object.
     If \a addLimitTo1 is true (the default), adds a LIMIT clause to the query. */
    tristate querySingleRecord(QuerySchema& query, RecordData &data, bool addLimitTo1 = true);

    /*! Executes \a sql query and stores first record's field's (number \a column) string value
     inside \a value. For efficiency it's recommended that a query defined by \a sql
     should have just one field (SELECT one_field FROM ....).
     If \a addLimitTo1 is true (the default), adds a LIMIT clause to the query,
     so \a sql should not include one already.
     \return true if query was successfully executed and first record has been found,
     false on data retrieving failure, and cancelled if there's no single record available.
     \sa queryStringList() */
    tristate querySingleString(const QString& sql, QString &value, uint column = 0,
                               bool addLimitTo1 = true);

    /*! Convenience function: executes \a sql query and stores first
     record's field's (number \a column) value inside \a number. \sa querySingleString().
     Note: "LIMIT 1" is appended to \a sql statement if \a addLimitTo1 is true (the default).
     \return true if query was successfully executed and first record has been found,
     false on data retrieving failure, and cancelled if there's no single record available. */
    tristate querySingleNumber(const QString& sql, int &number, uint column = 0,
                               bool addLimitTo1 = true);

    /*! Executes \a sql query and stores Nth field's string value of every record
     inside \a list, where N is equal to \a column. The list is initially cleared.
     For efficiency it's recommended that a query defined by \a sql
     should have just one field (SELECT one_field FROM ....).
     \return true if all values were fetched successfuly,
     false on data retrieving failure. Returning empty list can be still a valid result.
     On errors, the list is not cleared, it may contain a few retrieved values. */
    bool queryStringList(const QString& sql, QStringList& list, uint column = 0);

    /*! \return true if there is at least one record returned in \a sql query.
     Does not fetch any records. \a success will be set to false
     on query execution errors (true otherwise), so you can see a difference between
     "no results" and "query execution error" states.
     Note: real executed query is: "SELECT 1 FROM (\a sql) LIMIT 1"
     if \a addLimitTo1 is true (the default). */
    bool resultExists(const QString& sql, bool &success, bool addLimitTo1 = true);

    /*! \return true if there is at least one record in \a table. */
    bool isEmpty(TableSchema& table, bool &success);

//! @todo perhaps use quint64 here?
    /*! \return number of records in \a sql query.
     Does not fetch any records. -1 is returned on query execution errors (>0 otherwise).
     Note: real executed query is: "SELECT COUNT() FROM (\a sql) LIMIT 1"
     (using querySingleNumber()) */
    int resultCount(const QString& sql);

    //PROTOTYPE:
#define A , const QVariant&
#define H_INS_REC(args) bool insertRecord(TableSchema &tableSchema args)
#define H_INS_REC_ALL \
    H_INS_REC(A); \
    H_INS_REC(A A); \
    H_INS_REC(A A A); \
    H_INS_REC(A A A A); \
    H_INS_REC(A A A A A); \
    H_INS_REC(A A A A A A); \
    H_INS_REC(A A A A A A A); \
    H_INS_REC(A A A A A A A A)
    H_INS_REC_ALL;

#undef H_INS_REC
#define H_INS_REC(args) bool insertRecord(FieldList& fields args)

    H_INS_REC_ALL;
#undef H_INS_REC_ALL
#undef H_INS_REC
#undef A

    bool insertRecord(TableSchema &tableSchema, const QList<QVariant>& values);

    bool insertRecord(FieldList& fields, const QList<QVariant>& values);

    /*! Creates table defined by \a tableSchema.
     Schema information is also added into kexi system tables, for later reuse.
     \return true on success - \a tableSchema object is then
     inserted to Connection structures - it is owned by Connection object now,
     so you shouldn't destroy the tableSchema object by hand
     (or declare it as local-scope variable).

     If \a replaceExisting is false (the default) and table with the same name
     (as tableSchema->name()) exists, false is returned.
     If \a replaceExisting is true, a table schema with the same name (if exists)
     is overwritten, then a new table schema gets the same identifier
     as existing table schema's identifier.

     Note that on error:
     - \a tableSchema is not inserted into Connection's structures,
       so you are still owner of this object
     - existing table schema object is not destroyed (i.e. it is still available
       e.g. using Connection::tableSchema(const QString& ), even if the table
       was physically dropped.
    */
    bool createTable(TableSchema* tableSchema, bool replaceExisting = false);

    /*! Drops a table defined by \a tableSchema (both table object as well as physically).
     If true is returned, schema information \a tableSchema is destoyed
     (because it's owned), so don't keep this anymore!
     No error is raised if the table does not exist physically
     - its schema is removed even in this case.
    */
//! @todo (js): update any structure (e.g. query) that depend on this table!
    tristate dropTable(TableSchema* tableSchema);

    /*! It is a convenience function, does exactly the same as
     bool dropTable( KexiDB::TableSchema* tableSchema ) */
    tristate dropTable(const QString& table);

    /*! Alters \a tableSchema using \a newTableSchema in memory and on the db backend.
     \return true on success, cancelled if altering was cancelled. */
//! @todo (js): implement real altering
//! @todo (js): update any structure (e.g. query) that depend on this table!
    tristate alterTable(TableSchema& tableSchema, TableSchema& newTableSchema);

    /*! Alters name of table described by \a tableSchema to \a newName.
     If \a replace is true, destination table is completely dropped and replaced
     by \a tableSchema, if present. In this case, identifier of
     \a tableSchema becomes equal to the dropped table's id, what can be useful
     if \a tableSchema was created with a temporary name and ID (used in AlterTableHandler).

     If \a replace is false (the default) and destination table is present
     -- false is returned and ERR_OBJECT_EXISTS error is set.
     The schema of \a tableSchema is updated on success.
     \return true on success. */
    bool alterTableName(TableSchema& tableSchema, const QString& newName, bool replace = false);

    /*! Drops a query defined by \a querySchema.
     If true is returned, schema information \a querySchema is destoyed
     (because it's owned), so don't keep this anymore!
    */
    bool dropQuery(QuerySchema* querySchema);

    /*! It is a convenience function, does exactly the same as
     bool dropQuery( KexiDB::QuerySchema* querySchema ) */
    bool dropQuery(const QString& query);

    /*! Removes information about object with \a objId
     from internal "kexi__object" and "kexi__objectdata" tables.
     \return true on success. */
    bool removeObject(uint objId);

    /*! \return first field from \a fieldlist that has system name,
     null if there are no such field.
     For checking, Driver::isSystemFieldName() is used, so this check can
     be driver-dependent. */
    Field* findSystemFieldName(const FieldList& fieldlist);

    /*! \return name of any (e.g. first found) database for this connection.
     This method does not close or open this connection. The method can be used
     (it is also internally used, e.g. for database dropping) when we need
     a database name before we can connect and execute any SQL statement
     (e.g. DROP DATABASE).

     The method can return nul lstring, but in this situation no automatic (implicit)
     connections could be made, what is useful by e.g. dropDatabase().

     Note for driver developers: return here a name of database which you are sure
     is existing.
     Default implementation returns:
     - value that previously had been set using setAvailableDatabaseName() for
       this connection, if it is not empty
     - else (2nd priority): value of DriverBehaviour::ALWAYS_AVAILABLE_DATABASE_NAME
     if it is not empty.

     See decription of DriverBehaviour::ALWAYS_AVAILABLE_DATABASE_NAME member.
     You may want to reimplement this method only when you need to depend on
     this connection specifics
     (e.g. you need to check something remotely).
    */
    virtual QString anyAvailableDatabaseName();

    /*! Sets \a dbName as name of a database that can be accessible.
     This is option that e.g. application that make use of KexiDB library can set
     to tune connection's behaviour when it needs to temporary connect to any database
     in the server to do some work.
     You can pass empty dbName - then anyAvailableDatabaseName() will try return
     DriverBehaviour::ALWAYS_AVAILABLE_DATABASE_NAME (the default) value
     instead of the one previously set with setAvailableDatabaseName().

     \sa anyAvailableDatabaseName()
    */
    void setAvailableDatabaseName(const QString& dbName);

    /*! Because some engines need to have opened any database before
     executing administrative sql statements like "create database" or "drop database",
     this method is used to use appropriate, existing database for this connection.
     For file-based db drivers this always return true and does not set tmpdbName
     to any value. For other db drivers: this sets tmpdbName to db name computed
     using anyAvailableDatabaseName(), and if the name computed is empty, false
     is returned; if it is not empty, useDatabase() is called.
     False is returned also when useDatabase() fails.
     You can call this method from your application's level if you really want to perform
     tasks that require any used database. In such a case don't forget
     to closeDatabase() if returned tmpdbName is not empty.

     Note: This method has nothing to do with creating or using temporary databases
     in such meaning that these database are not persistent
    */
    bool useTemporaryDatabaseIfNeeded(QString &tmpdbName);

    /*! \return autoincrement field's \a aiFieldName value
     of last inserted record. This refers \a tableName table.

     Simply, method internally fetches last inserted record and returns selected
     field's value. Requirements: field must be of integer type, there must be a
     record inserted in current database session (whatever this means).
     On error (quint64)-1 is returned.
     Last inserted record is identified by magical record identifier, usually called
     ROWID (PostgreSQL has it as well as SQLite;
     see DriverBehaviour::ROW_ID_FIELD_RETURNS_LAST_AUTOINCREMENTED_VALUE).
     ROWID's value will be assigned back to \a ROWID if this pointer is not null.
    */
    quint64 lastInsertedAutoIncValue(const QString& aiFieldName, const QString& tableName,
                                     quint64* ROWID = 0);

    /*! \overload int lastInsertedAutoIncValue(const QString&, const QString&, quint64*)
    */
    quint64 lastInsertedAutoIncValue(const QString& aiFieldName,
                                     const TableSchema& table, quint64* ROWID = 0);

    /*! Executes query \a statement, but without returning resulting
     rows (used mostly for functional queries).
     Only use this method if you really need. */
    bool executeSQL(const QString& statement);

    //! @short options used in selectStatement()
    class CALLIGRADB_EXPORT SelectStatementOptions
    {
    public:
        SelectStatementOptions();
        ~SelectStatementOptions();

        //! A mode for escaping identifier, Driver::EscapeDriver|Driver::EscapeAsNecessary by default
        int identifierEscaping;

        //! True if ROWID should be also retrieved. False by default.
        bool alsoRetrieveROWID : 1;

        /*! True if relations (LEFT OUTER JOIN) for visible lookup columns should be added.
         True by default. This is set to false when user-visible statement is generated
         e.g. for the Query Designer. */
        bool addVisibleLookupColumns : 1;
    };

    /*! \return "SELECT ..." statement's string needed for executing query
     defined by \a querySchema, \a params and \a options. */
    QString selectStatement(QuerySchema& querySchema,
                            const QList<QVariant>& params,
                            const SelectStatementOptions& options = SelectStatementOptions()) const;

    /*! \overload QString selectStatement( QuerySchema& querySchema,
      QList<QVariant> params = QList<QVariant>(),
      const SelectStatementOptions& options = SelectStatementOptions() ) const;
     \return "SELECT ..." statement's string needed for executing query
     defined by \a querySchema. */
    inline QString selectStatement(QuerySchema& querySchema,
                                   const SelectStatementOptions& options = SelectStatementOptions()) const {
        return selectStatement(querySchema, QList<QVariant>(), options);
    }

    /*! Stores object's schema data (id, name, caption, help text)
     described by \a sdata on the backend.
     If \a newObject is true, new entry is created,
     and (when sdata.id() was <=0), new, unique object identifier
     is obtained and assigned to \a sdata (see SchemaData::id()).

     If \a newObject is false, it's expected that entry on the
     backend already exists, so it's updated (changes to identifier are not allowed).
     \return true on success. */
    bool storeObjectSchemaData(SchemaData &sdata, bool newObject);

    /*! Added for convenience.
     \sa setupObjectSchemaData( const KexiDB::RecordData &data, SchemaData &sdata ).
     \return true on success, false on failure and cancelled when such object couldn't */
    tristate loadObjectSchemaData(int objectID, SchemaData &sdata);

    /*! Finds object schema data for object of type \a objectType and name \a objectName.
     If the object is found, resulted schema is stored in \a sdata and true is returned,
     otherwise false is returned. */
    tristate loadObjectSchemaData(int objectType, const QString& objectName, SchemaData &sdata);

    /*! Loads (potentially large) data block (e.g. xml form's representation), referenced by objectID
     and puts it to \a dataString. The can be block indexed with optional \a dataID.
     \return true on success, false on failure and cancelled when there is no such data block
     \sa storeDataBlock(). */
    tristate loadDataBlock(int objectID, QString &dataString, const QString& dataID);

    /*! Stores (potentially large) data block \a dataString (e.g. xml form's representation),
     referenced by objectID. Block will be stored in "kexi__objectdata" table and
     an optional \a dataID identifier.
     If there is already such record in the table, it's simply overwritten.
     \return true on success
     \sa loadDataBlock(). */
    bool storeDataBlock(int objectID, const QString &dataString,
                        const QString& dataID = QString());

    /*! Removes (potentially large) string data (e.g. xml form's representation),
     referenced by objectID, and pointed by optional \a dataID.
     \return true on success. Does not fail if the block does not exist.
     Note that if \a dataID is not specified, all data blocks for this dialog will be removed.
     \sa loadDataBlock() storeDataBlock(). */
    bool removeDataBlock(int objectID, const QString& dataID = QString());

    class CALLIGRADB_EXPORT TableSchemaChangeListenerInterface
    {
    public:
        TableSchemaChangeListenerInterface() {}
        virtual ~TableSchemaChangeListenerInterface() {}

        /*! Closes listening object so it will be deleted and thus no longer use
         a conflicting table schema. */
        virtual tristate closeListener() = 0;

        /*! i18n'd string that can be displayed for user to inform about
         e.g. conflicting listeners. */
        QString listenerInfoString;
    };
//TMP// TODO: will be more generic
    /** Register \a listener for receiving (listening) information about changes
     in TableSchema object. Changes could be: altering and removing. */
    void registerForTableSchemaChanges(TableSchemaChangeListenerInterface& listener,
                                       TableSchema& schema);

    void unregisterForTableSchemaChanges(TableSchemaChangeListenerInterface& listener,
                                         TableSchema &schema);

    void unregisterForTablesSchemaChanges(TableSchemaChangeListenerInterface& listener);

    QSet<Connection::TableSchemaChangeListenerInterface*>*
    tableSchemaChangeListeners(TableSchema& tableSchema) const;

    tristate closeAllTableSchemaChangeListeners(TableSchema& tableSchema);

//! @todo move this somewhere to low level class (MIGRATION?)
    /*! LOW LEVEL METHOD. For reimplementation: returns true if table
     with name \a tableName exists in the database.
     \return false if it does not exist or error occurred.
     The lookup is case insensitive. */
    virtual bool drv_containsTable(const QString &tableName) = 0;

    /*! Creates table using \a tableSchema information.
     \return true on success. Default implementation
     builds a statement using createTableStatement() and calls drv_executeSQL()
     Note for driver developers: reimplement this only if you want do to
     this in other way.

     Moved to public for KexiMigrate.
     @todo fix this after refactoring
     */
    virtual bool drv_createTable(const TableSchema& tableSchema);

    /*! Alters table's described \a tableSchema name to \a newName.
     This is the default implementation, using "ALTER TABLE <oldname> RENAME TO <newname>",
     what's supported by SQLite >= 3.2, PostgreSQL, MySQL.
     Backends lacking ALTER TABLE can reimplement this with by an inefficient
     data copying to a new table. In any case, renaming is performed at the backend.
     It's good idea to keep the operation within a transaction.
     \return true on success.

     Moved to public for KexiProject.
     @todo fix this after refactoring
    */
    virtual bool drv_alterTableName(TableSchema& tableSchema, const QString& newName);

    /*! Physically drops table named with \a name.
     Default impelmentation executes "DROP TABLE.." command,
     so you rarely want to change this.

      Moved to public for KexiMigrate
      @todo fix this after refatoring
    */
    virtual bool drv_dropTable(const QString& name);

    /*! Prepare a SQL statement and return a \a PreparedStatement instance. */
    virtual PreparedStatement::Ptr prepareStatement(PreparedStatement::StatementType type,
            FieldList& fields) = 0;

    bool isInternalTableSchema(const QString& tableName);

    /*! Setups schema data for object that owns sdata (e.g. table, query)
      using \a cursor opened on 'kexi__objects' table, pointing to a record
      corresponding to given object.

      Moved to public for KexiMigrate
      @todo fix this after refatoring
    */
    bool setupObjectSchemaData(const RecordData &data, SchemaData &sdata);

    /*! \return a new field table schema for a table retrieved from \a data.
     Used internally by tableSchema().

      Moved to public for KexiMigrate
      @todo fix this after refatoring
    */
    KexiDB::Field* setupField(const RecordData &data);

    /*! @internal. Inserts internal table to Connection's structures, so it can be found by name.
     This method is used for example in KexiProject to insert information about "kexi__blobs"
     table schema. Use createTable() to physically create table. After createTable()
     calling insertInternalTable() is not required.
     Also used internally by Connection::newKexiDBSystemTableSchema(const QString&) */
    void insertInternalTable(TableSchema& tableSchema);

protected:
    /*! Used by Driver */
    Connection(Driver *driver, ConnectionData &conn_data);

    /*! Method to be called form Connection's subclass destructor.
     \sa ~Connection() */
    void destroy();

    /*! @internal drops table \a tableSchema physically, but destroys
     \a tableSchema object only if \a alsoRemoveSchema is true.
     Used (alsoRemoveSchema==false) on table altering:
     if recreating table can failed we're giving up and keeping
     the original table schema (even if it is no longer points to any real data). */
    tristate dropTable(KexiDB::TableSchema* tableSchema, bool alsoRemoveSchema);

    /*! For reimplementation: connects to database. \a version should be set to real
     server's version.
      \return true on success. */
    virtual bool drv_connect(KexiDB::ServerVersionInfo& version) = 0;

    /*! For reimplementation: disconnects database
      \return true on success. */
    virtual bool drv_disconnect() = 0;

    /*! Executes query \a statement, but without returning resulting
     rows (used mostly for functional queries).
     Only use this method if you really need. */
    virtual bool drv_executeSQL(const QString& statement) = 0;

    /*! For reimplementation: loads list of databases' names available for this connection
     and adds these names to \a list. If your server is not able to offer such a list,
     consider reimplementing drv_databaseExists() instead.
     The method should return true only if there was no error on getting database names
     list from the server.
     Default implementation puts empty list into \a list and returns true. */
    virtual bool drv_getDatabasesList(QStringList &list);

//! @todo move this somewhere to low level class (MIGRATION?)
    /*! LOW LEVEL METHOD. For reimplementation: loads low-level list of table names
     available for this connection. The names are in lower case.
     The method should return true only if there was no error on getting database names
     list from the server. */
    virtual bool drv_getTablesList(QStringList &list) = 0;

    /*! For optional reimplementation: asks server if database \a dbName exists.
     This method is used internally in databaseExists(). The default  implementation
     calls databaseNames and checks if that list contains \a dbName. If you need to
     ask the server specifically if a database exists, eg. if you can't retrieve a list
     of all available database names, please reimplement this method and do all
     needed checks.

     See databaseExists() description for details about ignoreErrors argument.
     You should use it properly in your implementation.

     Note: This method should also work if there is already database used (with useDatabase());
     in this situation no changes should be made in current database selection. */
    virtual bool drv_databaseExists(const QString &dbName, bool ignoreErrors = true);

    /*! For reimplementation: creates new database using connection */
    virtual bool drv_createDatabase(const QString &dbName = QString()) = 0;

    /*! For reimplementation: opens existing database using connection
     \return true on success, false on failure and cancelled if user has cancelled this action. */
    virtual bool drv_useDatabase(const QString &dbName = QString(), bool *cancelled = 0,
                                 MessageHandler* msgHandler = 0) = 0;

    /*! For reimplementation: closes previously opened database
      using connection. */
    virtual bool drv_closeDatabase() = 0;

    /*! \return true if internal driver's structure is still in opened/connected
     state and database is used.
     Note for driver developers: Put here every test that you can do using your
     internal engine's database API,
     eg (a bit schematic):  my_connection_struct->isConnected()==true.
     Do not check things like Connection::isDatabaseUsed() here or other things
     that "KexiDB already knows" at its level.
     If you cannot test anything, just leave default implementation (that returns true).

     Result of this method is used as an additional chance to check for isDatabaseUsed().
     Do not call this method from your driver's code, it should be used at KexiDB
     level only.
    */
    virtual bool drv_isDatabaseUsed() const {
        return true;
    }

    /*! For reimplementation: drops database from the server
      using connection. After drop, database shouldn't be accessible
      anymore. */
    virtual bool drv_dropDatabase(const QString &dbName = QString()) = 0;

    /*! \return "CREATE TABLE ..." statement string needed for \a tableSchema
     creation in the database.

     Note: The statement string can be specific for this connection's driver database,
     and thus not reusable in general.
    */
    QString createTableStatement(const TableSchema& tableSchema) const;


    /*! \return "SELECT ..." statement's string needed for executing query
     defined by "select * from table_name" where <i>table_name</i> is \a tableSchema's name.
     This method's variant can be useful when there is no appropriate QuerySchema defined.

     Note: The statement string can be specific for this connection's driver database,
     and thus not reusable in general.
    */
    QString selectStatement(TableSchema& tableSchema,
                            const SelectStatementOptions& options = SelectStatementOptions()) const;

    /*!
     Creates table named by \a tableSchemaName. Schema object must be on
     schema tables' list before calling this method (otherwise false if returned).
     Just uses drv_createTable( const KexiDB::TableSchema& tableSchema ).
     Used internally, e.g. in createDatabase().
     \return true on success
    */
    virtual bool drv_createTable(const QString& tableSchemaName);

//  /*! Executes query \a statement and returns resulting rows
//   (used mostly for SELECT query). */
//  virtual bool drv_executeQuery( const QString& statement ) = 0;

    /*! \return unique identifier of last inserted row.
     Typically this is just primary key value.
     This identifier could be reused when we want to reference
     just inserted row.
     Note for driver developers: contact staniek (at) kde.org
     if your engine do not offers this information. */
    virtual quint64 drv_lastInsertRowID() = 0;

    /*! Note for driver developers: begins new transaction
     and returns handle to it. Default implementation just
     executes "BEGIN" sql statement and returns just empty data (TransactionData object).

     Drivers that do not support transactions (see Driver::features())
     do never call this method.
     Reimplement this method if you need to do something more
     (e.g. if you driver will support multiple transactions per connection).
     Make subclass of TransactionData (declared in transaction.h)
     and return object of this subclass.
     You should return NULL if any error occurred.
     Do not check anything in connection (isConnected(), etc.) - all is already done.
    */
    virtual TransactionData* drv_beginTransaction();

    /*! Note for driver developers: begins new transaction
     and returns handle to it. Default implementation just
     executes "COMMIT" sql statement and returns true on success.

     \sa drv_beginTransaction()
    */
    virtual bool drv_commitTransaction(TransactionData* trans);

    /*! Note for driver developers: begins new transaction
     and returns handle to it. Default implementation just
     executes "ROLLBACK" sql statement and returns true on success.

     \sa drv_beginTransaction()
    */
    virtual bool drv_rollbackTransaction(TransactionData* trans);


    /*! Preprocessing (if any) required by drivers before execution of an
        Insert statement.
        Reimplement this method in your driver if there are any special processing steps to be
        executed before an Insert statement.
      \sa drv_afterInsert()
    */
    virtual bool drv_beforeInsert(const QString& table, FieldList& fields) {
        Q_UNUSED(table);
        Q_UNUSED(fields);
        return true;
    }

    /*! Postprocessing (if any) required by drivers before execution of an
        Insert statement.
        Reimplement this method in your driver if there are any special processing steps to be
        executed after an Insert statement.
      \sa drv_beforeInsert()
    */
    virtual bool drv_afterInsert(const QString& table, FieldList& fields) {
        Q_UNUSED(table);
        Q_UNUSED(fields);
        return true;
    }

    /*! Preprocessing required by drivers before execution of an
        Update statement.
        Reimplement this method in your driver if there are any special processing steps to be
        executed before an Update statement.
    \sa drv_afterUpdate()
    */
    virtual bool drv_beforeUpdate(const QString& table, FieldList& fields) {
        Q_UNUSED(table);
        Q_UNUSED(fields);
        return true;
    }

    /*! Postprocessing required by drivers before execution of an
        Insert statement.
        Reimplement this method in your driver if there are any special processing steps to be
        executed after an Update statement.
      \sa drv_beforeUpdate()
    */
    virtual bool drv_afterUpdate(const QString& table, FieldList& fields) {
        Q_UNUSED(table);
        Q_UNUSED(fields);
        return true;
    }


    /*! Changes autocommiting option for established connection.
      \return true on success.

      Note for driver developers: reimplement this only if your engine
      allows to set special auto commit option (like "SET AUTOCOMMIT=.." in MySQL).
      If not, auto commit behaviour will be simulated if at least single
      transactions per connection are supported by the engine.
      Do not set any internal flags for autocommiting -- it is already done inside
      setAutoCommit().

      Default implementation does nothing with connection, just returns true.

      \sa drv_beginTransaction(), autoCommit(), setAutoCommit()
     */
    virtual bool drv_setAutoCommit(bool on);

    /*! Internal, for handling autocommited transactions:
     begins transaction if one is supported.
     \return true if new transaction started
     successfully or no transactions are supported at all by the driver
     or if autocommit option is turned off.
     A handle to a newly created transaction (or null on error) is passed
     to \a tg parameter.

     Special case when used database driver has only single transaction support
     (Driver::SingleTransactions):
     and there is already transaction started, it is committed before
     starting a new one, but only if this transaction has been started inside Connection object.
     (i.e. by beginAutoCommitTransaction()). Otherwise, a new transaction will not be started,
     but true will be returned immediately.
    */
    bool beginAutoCommitTransaction(TransactionGuard& tg);

    /*! Internal, for handling autocommited transactions:
     Commits transaction prevoiusly started with beginAutoCommitTransaction().
     \return true on success or when no transactions are supported
     at all by the driver.

     Special case when used database driver has only single transaction support
     (Driver::SingleTransactions): if \a trans has been started outside Connection object
     (i.e. not by beginAutoCommitTransaction()), the transaction will not be committed.
    */
    bool commitAutoCommitTransaction(const Transaction& trans);

    /*! Internal, for handling autocommited transactions:
     Rollbacks transaction prevoiusly started with beginAutoCommitTransaction().
     \return true on success or when no transactions are supported
     at all by the driver.

     Special case when used database driver has only single transaction support
     (Driver::SingleTransactions): \a trans will not be rolled back
     if it has been started outside this Connection object.
    */
    bool rollbackAutoCommitTransaction(const Transaction& trans);

    /*! Creates cursor data and initializes cursor
      using \a statement for later data retrieval. */
//  virtual CursorData* drv_createCursor( const QString& statement ) = 0;
    /*! Closes and deletes cursor data. */
//  virtual bool drv_deleteCursor( CursorData *data ) = 0;

    /*! Helper: checks if connection is established;
      if not: error message is set up and false returned */
    bool checkConnected();

    /*! Helper: checks both if connection is established and database any is used;
      if not: error message is set up and false returned */
    bool checkIsDatabaseUsed();

    /*! \return a full table schema for a table retrieved using 'kexi__*' system tables.
     Used internally by tableSchema() methods. */
    TableSchema* setupTableSchema(const RecordData &data);

    /*! \return a full query schema for a query using 'kexi__*' system tables.
     Used internally by querySchema() methods. */
    QuerySchema* setupQuerySchema(const RecordData &data);

    /*! Update a row. */
    bool updateRow(QuerySchema &query, RecordData& data, RowEditBuffer& buf, bool useROWID = false);
    /*! Insert a new row. */
    bool insertRow(QuerySchema &query, RecordData& data, RowEditBuffer& buf, bool getROWID = false);
    /*! Delete an existing row. */
    bool deleteRow(QuerySchema &query, RecordData& data, bool useROWID = false);
    /*! Delete all existing rows. */
    bool deleteAllRows(QuerySchema &query);

    /*! Allocates all needed table KexiDB system objects for kexi__* KexiDB liblary's
     system tables schema.
     These objects are used internally in this connection
     and are added to list of tables (by name,
     not by id because these have no ids).
    */
    bool setupKexiDBSystemSchema();

    /*! used internally by setupKexiDBSystemSchema():
     Allocates single table KexiDB system object named \a tsname
     and adds this to list of such objects (for later removal on closeDatabase()).
    */
    TableSchema* newKexiDBSystemTableSchema(const QString& tsname);

    //! Identifier escaping function in the associated Driver.
    /*! Calls the identifier escaping function in the associated Driver to
     escape table and column names.  This should be used when explicitly
     constructing SQL strings (e.g. "FROM " + escapeIdentifier(tablename)).
     It should not be used for other functions (e.g. don't do
     useDatabase(escapeIdentifier(database))), because the identifier will
     be escaped when the called function generates, for example, "USE " +
     escapeIdentifier(database).

     For efficiency, kexi__* system tables and columns therein are not escaped
     - we assume these are valid identifiers for all drivers.
    */
    inline QString escapeIdentifier(const QString& id,
                                    int escaping = Driver::EscapeDriver | Driver::EscapeAsNecessary) const {
        return m_driver->escapeIdentifier(id, escaping);
    }

    /*! Called by TableSchema -- signals destruction to Connection object
     To avoid having deleted table object on its list. */
    void removeMe(TableSchema *ts);

    /*! @internal
     \return true if the cursor \a cursor contains column \a column,
     else, sets appropriate error with a message and returns false. */
    bool checkIfColumnExists(Cursor *cursor, uint column);

    /*! @internal used by querySingleRecord() methods.
     Note: "LIMIT 1" is appended to \a sql statement if \a addLimitTo1 is true (the default). */
    tristate querySingleRecordInternal(RecordData &data, const QString* sql,
                                       QuerySchema* query, bool addLimitTo1 = true);

    /*! @internal used by Driver::createConnection().
     Only works if connection is not yet established. */
    void setReadOnly(bool set);

    /*! Loads extended schema information for table \a tableSchema,
     if present (see ExtendedTableSchemaInformation in Kexi Wiki).
     \return true on success */
    bool loadExtendedTableSchemaData(TableSchema& tableSchema);

    /*! Stores extended schema information for table \a tableSchema,
     (see ExtendedTableSchemaInformation in Kexi Wiki).
     The action is performed within the current transaction,
     so it's up to you to commit.
     Used, e.g. by createTable(), within its transaction.
     \return true on success */
    bool storeExtendedTableSchemaData(TableSchema& tableSchema);

    /*! @internal
     Stores main field's schema information for field \a field.
     Used in table altering code when information in kexi__fields has to be updated.
     \return true on success and false on failure. */
    bool storeMainFieldSchema(Field *field);

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /*! This is a part of alter table interface implementing lower-level operations
     used to perform table schema altering. Used by AlterTableHandler.

     Changes value of field property.
     \return true on success, false on failure, cancelled if the action has been cancelled.

     Note for driver developers: implement this if the driver has to supprot the altering. */
    virtual tristate drv_changeFieldProperty(TableSchema &table, Field& field,
            const QString& propertyName, const QVariant& value) {
        Q_UNUSED(table); Q_UNUSED(field); Q_UNUSED(propertyName); Q_UNUSED(value);
        return cancelled;
    }

    //! Used by Cursor class
    void addCursor(KexiDB::Cursor& cursor);

    //! Used by Cursor class
    void takeCursor(KexiDB::Cursor& cursor);

private:
    ConnectionPrivate* d; //!< @internal d-pointer class.
    Driver* const m_driver; //!< The driver this \a Connection instance uses.
    bool m_destructor_started : 1; //!< helper: true if destructor is started.
    bool m_insideCloseDatabase : 1; //!< helper: true while closeDatabase() is executed

    friend class KexiDB::Driver;
    friend class KexiDB::Cursor;
    friend class KexiDB::TableSchema; //!< for removeMe()
    friend class KexiDB::DatabaseProperties; //!< for setError()
    friend class ConnectionPrivate;
    friend class KexiDB::AlterTableHandler;
};

/*! \return "SELECT ..." statement's string needed for executing query
    defined by \a querySchema, \a params and \a options. 
    \a driver can be provided to generate driver-dependent statement. 
    If \a driver is 0, KexiSQL statement is generated. */
CALLIGRADB_EXPORT QString selectStatement(const KexiDB::Driver *driver,
                                       KexiDB::QuerySchema& querySchema,
                                       const QList<QVariant>& params,
                                       const KexiDB::Connection::SelectStatementOptions& options = KexiDB::Connection::SelectStatementOptions());

/*! \overload QString selectStatement(const KexiDB::Driver *driver, KexiDB::QuerySchema& querySchema, const QList<QVariant>& params, const KexiDB::Connection::SelectStatementOptions& options); */
CALLIGRADB_EXPORT inline QString selectStatement(const KexiDB::Driver *driver,
                                              QuerySchema& querySchema,
                                              const KexiDB::Connection::SelectStatementOptions& options = KexiDB::Connection::SelectStatementOptions())
{
    return KexiDB::selectStatement(driver, querySchema, QList<QVariant>(), options);
}

} //namespace KexiDB

#endif
